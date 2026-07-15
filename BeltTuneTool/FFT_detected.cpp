#include "headers/FFT_detected.hpp"

#include <cmath>
#include <cstring>

template<typename T>
FFT_detected<T>::FFT_detected(TFT_MultiCore& tft, int sampleRate, int samples, int pin_WS, int pin_SD, int pin_SCK)
	: m_tft(tft),
	  _pin_WS(pin_WS),
	  _pin_SD(pin_SD),
	  _pin_SCK(pin_SCK),
	  _sampleRate(sampleRate),
	  _samples(samples)
{
	// Spectrogramme affiche comme une image glissante en RAM.
	_sgBuffer = new uint16_t[SG_W * SG_H];

	for(int i = 0; i < SG_W * SG_H; i++)
		_sgBuffer[i] = TFT_BLACK;

	_maxfreq = (T)sampleRate / 2;
	_minfreq = 0;

	_minBinfreq = 0;
	_maxBinfreq = (uint)((_maxfreq * samples) / sampleRate);

	_vReal = new T[_samples]();
	_vImag = new T[_samples]();
	_backgroundNoise = nullptr;
	_vRealNoise = nullptr;

	_ringBuffer = new T[_samples]();

	for(int i = 0; i < _samples; i++)
		_ringBuffer[i] = 0;

	meanNoise = 0;

	// Buffer brut lu par DMA depuis l'interface I2S.
	_i2sReadBuffer = new uint32_t[BLOCK_SIZE];

	const i2s_config_t i2s_config =
	{
		.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
		.sample_rate = sampleRate,
		.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
		.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
		.communication_format = I2S_COMM_FORMAT_I2S,
		.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
		.dma_buf_count = 8,
		.dma_buf_len = 256,
		.use_apll = false,
		.tx_desc_auto_clear = false,
		.fixed_mclk = 0
	};

	const i2s_pin_config_t pin_config =
	{
		.bck_io_num = _pin_SCK,
		.ws_io_num = _pin_WS,
		.data_out_num = I2S_PIN_NO_CHANGE,
		.data_in_num = _pin_SD
	};

	i2s_driver_install(I2S_NUM_0, &i2s_config, NULL, NULL);
	i2s_set_pin(I2S_NUM_0, &pin_config);

	// L'objet FFT travaille directement sur _vReal/_vImag.
	_FFT = std::shared_ptr<ArduinoFFT<T>>(new ArduinoFFT<T>(_vReal, _vImag, _samples, _sampleRate));
}

template<typename T>void FFT_detected<T>::mappingFreq(T minFreq, T maxFreq)
{
	_minfreq = minFreq;
	_maxfreq = maxFreq;

	if(_minfreq <= 0)
	{
		_minfreq = 0;
	}
	else
	{
		_minBinfreq = (uint)((_minfreq * _samples) / _sampleRate);
	}

	if(_maxfreq >= (T)_sampleRate / 2)
	{
		_maxfreq = (T)_sampleRate / 2;
	}
	else
	{
		_maxBinfreq = (uint)((_maxfreq * _samples) / _sampleRate);
	}

	this->drawFreqScale(_minfreq, _maxfreq);
}

template<typename T> bool FFT_detected<T>::readSamples()
{
	esp_err_t err = i2s_read(
		I2S_NUM_0,
		_i2sReadBuffer,
		BLOCK_SIZE * sizeof(uint32_t),
		&_i2sReadBufferSize,
		portMAX_DELAY);

	if(err != ESP_OK)
		return false;

	if(_i2sReadBufferSize != BLOCK_SIZE * sizeof(uint32_t))
		return false;

	// Chaque bloc lu est injecte dans un tampon circulaire pour reconstruire
	// en continu une fenetre FFT complete de _samples echantillons.
	for(int i = 0; i < BLOCK_SIZE; i++)
	{
		int32_t sample = (int32_t)_i2sReadBuffer[i];
		sample >>= 8;

		_ringBuffer[_writeIndex] = sample - meanNoise;
		_writeIndex++;

		if(_writeIndex >= _samples)
		{
			_writeIndex = 0;
			_bufferFull = true;
		}
	}

	return _bufferFull;
}

template<typename T> void FFT_detected<T>::buildFFTBuffer()
{
	// Reconstitue la fenetre temporelle la plus recente dans l'ordre chronologique.
	for(int i = 0; i < _samples; i++)
	{
		uint16_t index = (_writeIndex + i) % _samples;

		_vReal[i] = _ringBuffer[index];
		_vImag[i] = 0;
	}
}

template<typename T> void FFT_detected<T>::NoiseAquisition(int n , int bias)
{
	// Bloque temporairement l'affichage pour guider l'utilisateur pendant
	// l'acquisition du bruit de fond.
	this->m_tft.lock();

	const int BAR_X = 0;
	const int BAR_Y = 0;
	const int BAR_W = 240;
	const int BAR_H = 8;

	m_tft.tft->fillRect(0, 0, 240, 120, TFT_BLACK);
	m_tft.tft->setTextDatum(MC_DATUM);
	m_tft.tft->setTextColor(TFT_WHITE, TFT_BLACK);
	m_tft.tft->drawString("Placez le capteur", 120, 25, 2);
	m_tft.tft->drawString("a quelques mm", 120, 45, 2);
	m_tft.tft->drawString("de la courroie", 120, 65, 2);
	m_tft.tft->setTextColor(TFT_YELLOW, TFT_BLACK);
	m_tft.tft->drawString("Ne faites aucun bruit", 120, 85, 2);

	for(int i = 9; i > 0; i--)
	{
		m_tft.tft->fillRect(90, 100, 60, 20, TFT_BLACK);
		m_tft.tft->setTextColor(TFT_GREEN, TFT_BLACK);
		m_tft.tft->drawCentreString(String(i), 120, 95, 4);

		this->m_tft.unlock();
		vTaskDelay(pdMS_TO_TICKS(1000));
		this->m_tft.lock();
	}

	m_tft.tft->setTextDatum(TL_DATUM);
	m_tft.tft->fillRect(0, 0, 240, 120, TFT_BLACK);
	m_tft.tft->fillRect(BAR_X, BAR_Y, BAR_W, BAR_H, TFT_DARKGREY);
	m_tft.tft->drawRect(BAR_X, BAR_Y, BAR_W, BAR_H, TFT_WHITE);
	m_tft.tft->setTextColor(TFT_WHITE, TFT_BLACK);
	m_tft.tft->drawString("Acquisition bruit...", 5, BAR_H + 2, 1);

	if(_backgroundNoise == nullptr)
	{
		_backgroundNoise = new T[_samples / 2 + 1]();
	}

	for(int i = 0; i <= _samples / 2; i++)
	{
		_backgroundNoise[i] = 0;
	}

	int nt = 0;

	for(int i = 0; i < n; i++)
	{
		// Attend qu'une fenetre FFT complete soit disponible.
		while(!this->readSamples())
		{
			this->m_tft.unlock();
			taskYIELD();
			this->m_tft.lock();
		}

		this->buildFFTBuffer();
		this->computeFFT();

		// Accumule un bruit moyen en dB sur chaque bin frequentiel.
		for(int j = 0; j <= _samples / 2; j++)
		{
			double db = 20.0 * log10(std::max(_vReal[j], (T)1e-12));
			_backgroundNoise[j] += db;
		}

		nt++;

		int progress = (i + 1) * (BAR_W - 2) / n;

		m_tft.tft->fillRect(BAR_X + 1, BAR_Y + 1, progress, BAR_H - 2, TFT_GREEN);
		m_tft.tft->fillRect(180, BAR_H + 2, 50, 8, TFT_BLACK);
		m_tft.tft->drawString(String((i + 1) * 100 / n) + "%", 180, BAR_H + 2, 1);
	}

	this->m_tft.unlock();

	if(nt == 0)
	{
		return;
	}

	for(int i = 0; i <= _samples / 2; i++)
	{
		// Le biais final evite de considerer le bruit moyen comme un vrai pic utile.
		_backgroundNoise[i] /= nt;
		_backgroundNoise[i] += bias;
	}
}

template<typename T> void FFT_detected<T>::computeFFT()
{
	// Fenetrage puis passage temporel -> frequentiel -> module.
	_FFT->windowing(FFTWindow::Hamming, FFTDirection::Forward);
	_FFT->compute(FFTDirection::Forward);
	_FFT->complexToMagnitude();
}

template<typename T> void FFT_detected<T>::subNoiseBackground()
{
	if(_backgroundNoise == nullptr)
	{
		return;
	}

	// Convertit le module en dB puis soustrait le spectre de bruit moyen appris.
	for(int i = 0; i <= _samples / 2; i++)
	{
		double s = 20.0 * log10(std::max(_vReal[i], (T)1e-12));
		_vReal[i] = s - _backgroundNoise[i];
	}

	// Applique une compensation empirique du micro INMP441 sur les basses frequences.
	for(int i = 1; i <= _samples / 2; i++)
	{
		double freq = (double)i * _sampleRate / _samples;
		double correction = correctionINMP441(freq);
		_vReal[i] += correction;
	}
}

template<typename T> HpsData FFT_detected<T>::Hps(T thresholdDb, uint8_t nHarmonics)
{
	T bestScore = -1e30;
	T bestFreq = -1;

	// Recherche le meilleur candidat fondamental via un score HPS simplifie:
	// pic principal interpole + contribution attenuee des harmoniques.
	for(uint i = _minBinfreq + 1; i < _maxBinfreq - 1; i++)
	{
		if(_vReal[i] < thresholdDb)
			continue;

		if(_vReal[i] <= _vReal[i - 1] || _vReal[i] < _vReal[i + 1])
			continue;

		// Interpolation parabolique locale pour affiner la position du pic.
		T alpha = _vReal[i - 1];
		T beta = _vReal[i];
		T gamma = _vReal[i + 1];
		T denom = alpha - 2.0 * beta + gamma;
		T delta = 0;

		if(std::abs(denom) > 1e-12)
		{
			delta = 0.5 * (alpha - gamma) / denom;
			delta = constrain(delta, (T)-0.5, (T)0.5);
		}

		T refinedBin = (T)i + delta;
		T refinedAmplitude = beta - (alpha - gamma) * delta / 4.0;
		T score = refinedAmplitude;

		// Additionne les harmoniques plausibles pour privilegier une fondamentale coherente.
		for(uint8_t h = 2; h <= nHarmonics; h++)
		{
			T harmonicBin = refinedBin * h;
			uint k = round(harmonicBin);

			if(k <= 1 || k >= (_samples / 2) - 1)
				break;

			T a = _vReal[k - 1];
			T b = _vReal[k];
			T c = _vReal[k + 1];

			if(b < 0.5 * thresholdDb)
				continue;

			T denomH = a - 2.0 * b + c;
			T deltaH = 0;

			if(std::abs(denomH) > 1e-12)
			{
				deltaH = 0.5 * (a - c) / denomH;
				deltaH = constrain(deltaH, (T)-0.5, (T)0.5);
			}

			T harmonicAmplitude = b - (a - c) * deltaH / 4.0;
			score += harmonicAmplitude / h;
		}

		if(score > bestScore)
		{
			bestScore = score;
			bestFreq = refinedBin * _sampleRate / _samples;
		}
	}

	data.score = bestScore;

	// Conserve un petit historique 
	for(int i = 2; i > 0; i--)
	{
		data.lastMesures[i] = data.lastMesures[i - 1];
	}

	data.lastMesures[0] = bestFreq;

	double mean = (data.lastMesures[0] + data.lastMesures[1] + data.lastMesures[2]) / 3.0;
	double variance = (pow(data.lastMesures[0] - mean, 2)
					+ pow(data.lastMesures[1] - mean, 2)
					+ pow(data.lastMesures[2] - mean, 2)) / 3.0;
	data.stdDev = sqrt(variance);

	return data;
}

template<typename T> void FFT_detected<T>::updateSG()
{
	// Fait glisser l'image du spectrogramme d'une ligne vers le bas,
	// puis dessine la nouvelle ligne en tete.
	this->m_tft.lock();

	memmove(&_sgBuffer[SG_W], &_sgBuffer[0], sizeof(uint16_t) * (SG_W * (SG_H - 1)));

	for(int x = 0; x < SG_W; x++)
	{
		double freq = _minfreq + (_maxfreq - _minfreq) * x / (SG_W - 1);
		uint bin = round(freq * _samples / _sampleRate);
		double db = _vReal[bin];

		_sgBuffer[x] = dbToColor(db);
	}

	m_tft.tft->pushImage(0, 0, SG_W, SG_H, _sgBuffer);

	this->m_tft.unlock();
}

template<typename T> void FFT_detected<T>::updateSpectrum(double dbLimite, double lastFondamental, bool lastBest, bool _selected)
{
	const int SG_W = 240;
	const int SG_H = 120;
	const double DB_MIN = 0.0;

	for(int x = 0; x < SG_W; x++)
	{
		double freq = _minfreq + (_maxfreq - _minfreq) * x / (SG_W - 1);
		uint bin = round(freq * _samples / _sampleRate);

		if(bin > _maxBinfreq)
			continue;
	}

	this->m_tft.lock();

	const double DB_MAX = 1.5 * dbLimite;

	// Le spectre vert represente la mesure courante; la courbe rouge memorise
	// une reference precedente marquee comme meilleure.
	m_tft.tft->fillRect(0, 0, SG_W, SG_H, TFT_BLACK);

	if(_referenceValid)
	{
		int prevXR = -1;
		int prevYR = -1;

		for(int x = 0; x < SG_W; x++)
		{
			double dbRef = constrain(_referenceSpectrum[x], DB_MIN, DB_MAX);
			int yRef = (int)((DB_MAX - dbRef) * (SG_H - 1) / (DB_MAX - DB_MIN));

			if(prevXR >= 0)
			{
				m_tft.tft->drawLine(prevXR, prevYR, x, yRef, TFT_RED);
			}

			prevXR = x;
			prevYR = yRef;
		}
	}

	int prevX = -1;
	int prevY = -1;

	for(int x = 0; x < SG_W; x++)
	{
		double freq = _minfreq + (_maxfreq - _minfreq) * x / (SG_W - 1);
		uint bin = round(freq * _samples / _sampleRate);

		if(bin > _maxBinfreq)
			continue;

		double db = _vReal[bin];

		if(lastBest)
		{
			_referenceSpectrum[x] = db;
		}

		db = constrain(db, DB_MIN, DB_MAX);

		int y = (int)((DB_MAX - db) * (SG_H - 1) / (DB_MAX - DB_MIN));

		if(prevX >= 0)
		{
			m_tft.tft->drawLine(prevX, prevY, x, y, TFT_GREEN);
		}

		prevX = x;
		prevY = y;
	}

	if(lastBest)
	{
		// Mémorise la courbe courante comme reference visuelle persistante.
		_referenceValid = true;
	}

	if(dbLimite >= DB_MIN && dbLimite <= DB_MAX)
	{
		int yLimite = (int)((DB_MAX - dbLimite) * (SG_H - 1) / (DB_MAX - DB_MIN));

		m_tft.tft->drawLine(0, yLimite, SG_W - 1, yLimite, TFT_BLUE);
		m_tft.tft->setTextColor(!_selected ? TFT_BLUE : TFT_GREEN, TFT_BLACK);
		m_tft.tft->drawString(String(dbLimite, 0) + " dB", 2, yLimite - 8, 1);
	}

	if(lastFondamental >= _minfreq && lastFondamental <= _maxfreq)
	{
		int xFond = (int)((lastFondamental - _minfreq) * (SG_W - 1) / (_maxfreq - _minfreq));

		m_tft.tft->drawLine(xFond, 0, xFond, SG_H - 1, TFT_RED);
		m_tft.tft->setTextColor(TFT_RED, TFT_BLACK);
		m_tft.tft->drawCentreString(String(lastFondamental, 1) + " Hz", xFond + 30, 2, 1);
	}

	this->m_tft.unlock();
}

template<typename T> FFT_detected<T>::~FFT_detected()
{
	delete[] _vReal;
	delete[] _vImag;
	delete[] _i2sReadBuffer;
	delete[] _sgBuffer;
	delete[] _ringBuffer;

	if(_backgroundNoise != nullptr)
	{
		delete[] _backgroundNoise;
	}

	if(_vRealNoise != nullptr)
	{
		delete[] _vRealNoise;
	}
}

template<typename T> double FFT_detected<T>::correctionINMP441(double f)
{
	// Courbe de compensation tres simple, interpolee lineairement entre quelques points.
	struct Point
	{
		double freq;
		double gain;
	};

	static const Point p[] =
	{
		{30, 7},
		{40, 4.5},
		{50, 3},
		{60, 2.5},
		{80, 2},
		{100, 1},
		{200, 0}
	};

	if(f <= p[0].freq)
		return p[0].gain;

	for(int i = 0; i < 6; i++)
	{
		if(f <= p[i + 1].freq)
		{
			double t = (f - p[i].freq) / (p[i + 1].freq - p[i].freq);
			return p[i].gain + t * (p[i + 1].gain - p[i].gain);
		}
	}

	return 0.0;
}

template<typename T> uint16_t FFT_detected<T>::dbToColor(double db)
{
	// Palette pseudo-thermique compacte pour le spectrogramme.
	db = constrain(db, 0.0, 40.0);

	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;

	if(db < 10)
	{
		return TFT_BLACK;
	}
	else if(db < 20)
	{
		double t = (db - 10.0) / 10.0;
		b = 50 + 205 * t;
	}
	else if(db < 30)
	{
		double t = (db - 20.0) / 10.0;
		r = 255 * t;
		g = 255 * t;
		b = 255;
	}
	else
	{
		double t = (db - 30.0) / 10.0;
		r = 255;
		g = 255;
		b = 255 * t;
	}

	return m_tft.tft->color565(r, g, b);
}

template<typename T> void FFT_detected<T>::drawFreqScale(double minFreq, double maxFreq)
{
	const int SG_W = 240;
	const int SG_H = 120;

	this->m_tft.lock();

	m_tft.tft->fillRect(0, SG_H, SG_W, 25, TFT_BLACK);
	m_tft.tft->setTextColor(TFT_WHITE, TFT_BLACK);
	m_tft.tft->setTextFont(1);
	m_tft.tft->drawLine(0, SG_H, SG_W - 1, SG_H, TFT_DARKGREY);

	// Choisit un pas de graduation lisible plutot qu'un pas mathematiquement exact.
	double span = maxFreq - minFreq;
	double step = span / 6.0;

	if(step <= 5)
		step = 5;
	else if(step <= 10)
		step = 10;
	else if(step <= 20)
		step = 20;
	else if(step <= 25)
		step = 25;
	else if(step <= 50)
		step = 50;
	else if(step <= 100)
		step = 100;
	else
		step = 200;

	double first = ceil(minFreq / step) * step;

	for(double f = first; f <= maxFreq; f += step)
	{
		int x = (int)((f - minFreq) * (SG_W - 1) / span);

		m_tft.tft->drawLine(x, SG_H, x, SG_H + 5, TFT_WHITE);

		String s = String((int)f);
		int w = m_tft.tft->textWidth(s);

		m_tft.tft->drawString(s, x - w / 2, SG_H + 7, 1);
	}

	this->m_tft.unlock();
}

template class FFT_detected<double>;
