#include "headers/PanelHerited.hpp"

#include "headers/FFT_detected.hpp"

#include <EEPROM.h>

MainPanel::MainPanel(TFT_MultiCore & _tft, vect2 pos, vect2 size): Panel(_tft, "Main", pos, size)
{
}

void MainPanel::setValues(float freq, float db, float sigma, float tension)
{
	this->m_tft.lock();

	m_freq = freq;
	m_db = db;
	m_sigma = sigma;
	m_tension = tension;

	this->m_tft.unlock();
}

void MainPanel::setVbat(double val)
{
	this->m_tft.lock();
	vbat = val;
	this->m_tft.unlock();
}

void MainPanel::drawBattery()
{
	const int X = 225;
	const int Y = 170;
	const int W = 10;
	const int H = 40;
	const double VBAT_MIN = 3.30;
	const double VBAT_MAX = 4.10;

	int percent = (vbat - VBAT_MIN) * 100.0 / (VBAT_MAX - VBAT_MIN);
	percent = constrain(percent, 0, 100);

	int fillHeight = percent * (H - 2) / 100;
	uint16_t color;

	if(percent > 50)
		color = TFT_GREEN;
	else if(percent > 20)
		color = TFT_YELLOW;
	else
		color = TFT_RED;

	m_tft.tft->fillRect(X - 20, Y - 15, 40, H + 25, TFT_BLACK);
	m_tft.tft->drawRect(X, Y, W, H, TFT_WHITE);
	m_tft.tft->fillRect(X + W / 4, Y - 4, W / 2, 4, TFT_WHITE);
	m_tft.tft->fillRect(X + 1, Y + 1, W - 2, H - 2, TFT_BLACK);
	m_tft.tft->fillRect(X + 1, Y + H - 1 - fillHeight, W - 2, fillHeight, color);
	m_tft.tft->setTextColor(TFT_WHITE, TFT_BLACK);
	m_tft.tft->drawCentreString(String(percent) + "%", X + W / 2, Y + H + 2, 1);
}

void MainPanel::draw()
{
	this->m_tft.lock();

	m_tft.tft->fillRect(m_position.x, m_position.y, m_size.x, m_size.y, TFT_BLACK);
	this->drawBattery();

	int y = m_position.y + 6;

	m_tft.tft->setTextColor(TFT_ORANGE, TFT_BLACK);
	m_tft.tft->drawCentreString(String(m_tension, 1) + " N", 40, y, 2);
    m_tft.tft->setTextColor(TFT_WHITE, TFT_BLACK);
	m_tft.tft->drawCentreString(String(m_db, 1) + " dB", 120, y, 2);
	m_tft.tft->drawCentreString(String(m_sigma, 2) +" dB", 200, y, 2);

	m_tft.tft->setTextColor(TFT_GREEN, TFT_BLACK);
	m_tft.tft->drawCentreString(String(m_freq, 1) + " Hz", m_position.x + m_size.x / 2, m_position.y + 30, 4);

	this->m_tft.unlock();
}

SettingPanel::Setting::Setting()
	: mu(0.77f),
	  L(0.5f),
	  seui_db(25.0f),
	  minHz(60.0f),
	  maxHz(300.0f),
	  nHarmonics(3),
	  nNoise(33),
	  SaveSetting(false),
	  LearnNoise(false),
	  Smode(false)
{
}

SettingPanel::SettingPanel(TFT_MultiCore & _tft, std::shared_ptr<FFT_detected<double>> fft, vect2 pos, vect2 size)
		: Panel(_tft, "Setting", pos, size),
			m_fft(fft)
{
	setting = Setting();
}

void SettingPanel::nextSetting()
{
	this->m_tft.lock();
	m_selected = (m_selected + 1) % 10;
	this->m_tft.unlock();
}

void SettingPanel::fwdSetting()
{
	this->m_tft.lock();

	switch(m_selected)
	{
		case 0: setting.mu += 0.001; break;
		case 1: setting.L += 0.001; break;
		case 2: setting.LearnNoise = !setting.LearnNoise; break;
		case 3: setting.Smode = !setting.Smode; break;
		case 4: setting.seui_db += 1; break;
		case 5: setting.nHarmonics += 1; break;
		case 6: setting.minHz += 1; break;
		case 7: setting.maxHz += 1; break;
		case 8: setting.nNoise += 1; break;
		case 9: setting.SaveSetting = true; break;
	}

	mapSettingInLimite();
	this->m_tft.unlock();

	if(m_selected == 6 || m_selected == 7)
	{
		m_fft->mappingFreq(setting.minHz, setting.maxHz);
	}
}

void SettingPanel::bwdSetting()
{
	this->m_tft.lock();

	switch(m_selected)
	{
		case 0: setting.mu -= 0.001; break;
		case 1: setting.L -= 0.001; break;
		case 2: setting.LearnNoise = !setting.LearnNoise; break;
		case 3: setting.Smode = !setting.Smode; break;
		case 4: setting.seui_db -= 1; break;
		case 5: setting.nHarmonics -= 1; break;
		case 6: setting.minHz -= 1; break;
		case 7: setting.maxHz -= 1; break;
		case 8: setting.nNoise -= 1; break;
		case 9: setting.SaveSetting = true; break;
	}

	mapSettingInLimite();
	this->m_tft.unlock();

	if(m_selected == 6 || m_selected == 7)
	{
		m_fft->mappingFreq(setting.minHz, setting.maxHz);
	}
}

void SettingPanel::fwdSpeedSetting()
{
	this->m_tft.lock();

	switch(m_selected)
	{
		case 0: setting.mu += 0.01; break;
		case 1: setting.L += 0.1; break;
		case 4: setting.seui_db += 1; break;
		case 5: setting.nHarmonics += 1; break;
		case 6: setting.minHz += 10; break;
		case 7: setting.maxHz += 10; break;
		case 8: setting.nNoise += 5; break;
	}

	mapSettingInLimite();
	this->m_tft.unlock();

	if(m_selected == 6 || m_selected == 7)
	{
		m_fft->mappingFreq(setting.minHz, setting.maxHz);
	}
}

void SettingPanel::bwdSpeedSetting()
{
	this->m_tft.lock();

	switch(m_selected)
	{
		case 0: setting.mu -= 0.01; break;
		case 1: setting.L -= 0.1; break;
		case 4: setting.seui_db -= 1; break;
		case 5: setting.nHarmonics -= 1; break;
		case 6: setting.minHz -= 10; break;
		case 7: setting.maxHz -= 10; break;
		case 8: setting.nNoise -= 5; break;
	}

	mapSettingInLimite();
	this->m_tft.unlock();

	if(m_selected == 6 || m_selected == 7)
	{
		m_fft->mappingFreq(setting.minHz, setting.maxHz);
	}
}

void SettingPanel::saveToEEPROM()
{
	if(!this->setting.SaveSetting)
		return;

	this->m_tft.lock();

	EEPROM.begin(512);
	EEPROM.put(0, this->setting);
	EEPROM.commit();
	EEPROM.end();

	this->setting.SaveSetting = false;

	this->m_tft.unlock();

	vTaskDelay(pdMS_TO_TICKS(500));

	this->draw();
}

void SettingPanel::loadFromEEPROM()
{
	this->m_tft.lock();
	EEPROM.begin(512);
	EEPROM.get(0, this->setting);
	EEPROM.end();

	if(this->setting.magic != 0x12345678)
	{
		this->setting = Setting();
	}

	this->setting.SaveSetting = false;

	this->m_tft.unlock();
}

void SettingPanel::mapSettingInLimite(void)
{
	if(setting.mu < 0.001f) setting.mu = 0.001f;
	if(setting.mu > 5.0f) setting.mu = 5.0f;

	if(setting.L < 0.05f) setting.L = 0.05f;
	if(setting.L > 5.0f) setting.L = 5.0f;

	if(setting.seui_db < 1.0f) setting.seui_db = 0.0f;
	if(setting.seui_db > 80.0f) setting.seui_db = 80.0f;

	if(setting.minHz < 30.0f)
		setting.minHz = 30.0f;

	if(setting.maxHz > 4000.0f)
		setting.maxHz = 4000.0f;

	if(setting.minHz >= setting.maxHz)
		setting.minHz = setting.maxHz - 1.0f;

	if(setting.minHz < 1.0f)
		setting.minHz = 1.0f;

	if(setting.nHarmonics < 2)
		setting.nHarmonics = 2;

	if(setting.nHarmonics > 10)
		setting.nHarmonics = 10;

	if(setting.nNoise < 1)
		setting.nNoise = 1;

	if(setting.nNoise > 200)
		setting.nNoise = 200;
}

void SettingPanel::draw()
{
	this->m_tft.lock();

	m_tft.tft->fillRect(m_position.x, m_position.y, m_size.x, m_size.y, TFT_BLACK);

	const int x1 = m_position.x + 20;
	const int x2 = m_position.x + 142;
	const int y0 = m_position.y + 4;
	const int dy = 14;

	auto drawItem = [&](int index, int x, int y, const String& txt)
	{
		m_tft.tft->setTextColor(index == m_selected ? TFT_GREEN : TFT_WHITE, TFT_BLACK);
		m_tft.tft->drawString(txt, x, y, 2);
	};

	drawItem(0, x1, y0 + 0 * dy, "Mu : " + String(setting.mu, 3));
	drawItem(1, x2, y0 + 0 * dy, "L : " + String(setting.L, 3));
	drawItem(2, x1, y0 + 1 * dy, "Noise : " + String(setting.LearnNoise ? "Yes" : "No"));
	drawItem(3, x2, y0 + 1 * dy, "Mode : " + String(setting.Smode ? "SG" : "SP"));
	drawItem(4, x1, y0 + 2 * dy, "dB : " + String(setting.seui_db, 0));
	drawItem(5, x2, y0 + 2 * dy, "H : " + String(setting.nHarmonics));
	drawItem(6, x1, y0 + 3 * dy, "Min F : " + String(setting.minHz, 0));
	drawItem(7, x2, y0 + 3 * dy, "Max F : " + String(setting.maxHz, 0));
	drawItem(8, x1, y0 + 4 * dy, "NBiais : " + String(setting.nNoise));
	drawItem(9, x2, y0 + 4 * dy, "SAVE : " + String(setting.SaveSetting ? "Yes" : "No"));

	this->m_tft.unlock();
}

PresetPanel::PresetPanel(TFT_MultiCore & _tft, std::shared_ptr<FFT_detected<double>> fft, std::shared_ptr<SettingPanel> settingPanel, vect2 pos, vect2 size)
		: Panel(_tft, "Preset", pos, size),
			m_fft(fft),
			m_settingPanel(settingPanel)
{
}

void PresetPanel::nextSetting()
{
	this->m_tft.lock();
	m_selected = (m_selected + 1) % 5;
	this->m_tft.unlock();
}

void PresetPanel::fwdSetting()
{
	this->m_tft.lock();

	switch(m_selected)
	{
		case 0:
			f = (f + 1) % BeltCatalogCount;
			p = 0;
			w = 0;
			l = 0;
			break;
		case 1:
			p = (p + 1) % BeltCatalog[f].pitchCount;
			w = 0;
			l = 0;
			break;
		case 2:
			w = (w + 1) % BeltCatalog[f].pitches[p].widthCount;
			break;
		case 3:
			l = (l + 1) % BeltCatalog[f].pitches[p].lengthCount;
			break;
		case 4:
			apply = true;
			this->validate(m_settingPanel->setting);
			break;
	}

	this->m_tft.unlock();

	if(apply)
		m_fft->mappingFreq(m_settingPanel->setting.minHz, m_settingPanel->setting.maxHz);
}

void PresetPanel::bwdSetting()
{
	this->m_tft.lock();

	switch(m_selected)
	{
		case 0:
			f = (f + BeltCatalogCount - 1) % BeltCatalogCount;
			p = 0;
			w = 0;
			l = 0;
			break;
		case 1:
			p = (p + BeltCatalog[f].pitchCount - 1) % BeltCatalog[f].pitchCount;
			w = 0;
			l = 0;
			break;
		case 2:
			w = (w + BeltCatalog[f].pitches[p].widthCount - 1) % BeltCatalog[f].pitches[p].widthCount;
			break;
		case 3:
			l = (l + BeltCatalog[f].pitches[p].lengthCount - 1) % BeltCatalog[f].pitches[p].lengthCount;
			break;
		case 4:
			apply = true;
			this->validate(m_settingPanel->setting);
			break;
	}

	this->m_tft.unlock();

	if(apply)
		m_fft->mappingFreq(m_settingPanel->setting.minHz, m_settingPanel->setting.maxHz);
}

void PresetPanel::fwdSpeedSetting()
{
	this->m_tft.lock();

	switch(m_selected)
	{
		case 0:
			f = (f + 1) % BeltCatalogCount;
			p = 0;
			w = 0;
			l = 0;
			break;
		case 1:
			p = (p + 1) % BeltCatalog[f].pitchCount;
			w = 0;
			l = 0;
			break;
		case 2:
			w = (w + 1) % BeltCatalog[f].pitches[p].widthCount;
			break;
		case 3:
			l = (l + 1) % BeltCatalog[f].pitches[p].lengthCount;
			break;
		case 4:
			apply = true;
			this->validate(m_settingPanel->setting);
			break;
	}

	this->m_tft.unlock();

	if(apply)
		m_fft->mappingFreq(m_settingPanel->setting.minHz, m_settingPanel->setting.maxHz);
}

void PresetPanel::bwdSpeedSetting()
{
	this->m_tft.lock();

	switch(m_selected)
	{
		case 0:
			f = (f + BeltCatalogCount - 1) % BeltCatalogCount;
			p = 0;
			w = 0;
			l = 0;
			break;
		case 1:
			p = (p + BeltCatalog[f].pitchCount - 1) % BeltCatalog[f].pitchCount;
			w = 0;
			l = 0;
			break;
		case 2:
			w = (w + BeltCatalog[f].pitches[p].widthCount - 1) % BeltCatalog[f].pitches[p].widthCount;
			break;
		case 3:
			l = (l + BeltCatalog[f].pitches[p].lengthCount - 1) % BeltCatalog[f].pitches[p].lengthCount;
			break;
		case 4:
			apply = true;
			this->validate(m_settingPanel->setting);
			break;
	}

	this->m_tft.unlock();

	if(apply)
		m_fft->mappingFreq(m_settingPanel->setting.minHz, m_settingPanel->setting.maxHz);
}

void PresetPanel::validate(SettingPanel::Setting & _setting)
{
	_setting.mu = BeltCatalog[f].pitches[p].widths[w].linearMass;
	_setting.L = BeltCatalog[f].pitches[p].lengths[l].mm / 2000.0f;

	const float Lmax = _setting.L;

	_setting.minHz = (1.0f / (2.0f * Lmax)) * sqrtf(100.0 / _setting.mu);

	if(_setting.minHz < 30)
		_setting.minHz = 30;

	_setting.maxHz = _setting.minHz * 3;

	if(_setting.maxHz > 4000)
		_setting.maxHz = 4000;
}

BeltData PresetPanel::getSelected() const
{
	BeltData data;

	const BeltPitch& pitch = BeltCatalog[f].pitches[p];

	data.linearMass = pitch.widths[w].linearMass;
	data.length = pitch.lengths[l].mm / 1000.0f;

	return data;
}

void PresetPanel::draw()
{
	this->m_tft.lock();

	m_tft.tft->fillRect(m_position.x, m_position.y, m_size.x, m_size.y, TFT_BLACK);

	const BeltPitch& pitch = BeltCatalog[f].pitches[p];
	const int x = m_position.x + 8;
	int y = m_position.y + 15;
	const int dy = 14;

	auto drawItem = [&](int index, const String& txt)
	{
		m_tft.tft->setTextColor((m_selected == index) ? TFT_GREEN : TFT_WHITE, TFT_BLACK);
		m_tft.tft->drawString(txt, x, y, 2);
		y += dy;
	};

	drawItem(0, "Famille : " + String(BeltCatalog[f].name));
	drawItem(1, "Pas : " + String(pitch.name));
	drawItem(2, "Largeur : " + String(pitch.widths[w].mm) + " mm");
	drawItem(3, "Longueur : " + String(pitch.lengths[l].mm) + " mm");

	m_tft.tft->setTextColor((m_selected == 4) ? TFT_GREEN : TFT_WHITE, TFT_BLACK);
	m_tft.tft->drawString(String(!apply ? "VALIDER" : "<VALIDER>"), m_position.x + 170, m_position.y + 30, 2);

	BeltData data = getSelected();

	m_tft.tft->setTextColor(TFT_BLUE, TFT_BLACK);
	m_tft.tft->drawString("Mu : " + String(data.linearMass, 4) + " kg/m", x, m_position.y, 2);

	this->m_tft.unlock();
}
