#pragma once

#include <Arduino.h>
#include <driver/i2s.h>
#include <arduinoFFT.h>
#include <array>
#include <algorithm>
#include <memory>
#include "TFT_MultiCore.hpp"

#define I2S_WS   25
#define I2S_SD   32
#define I2S_SCK  33

#define SAMPLES             4096
#define SAMPLING_FREQUENCY  8000

#define FFT_SIZE      4096
#define BLOCK_SIZE     2048

class HpsData
{
  public:

    double score;
    std::array<double, 3> lastMesures{0,0,0};
    double stdDev;
};

template<typename T> class FFT_detected
{
  public :

    FFT_detected(TFT_MultiCore& tft, int sampleRate, int samples, int pin_WS, int pin_SD, int pin_SCK);
    void mappingFreq(T minFreq, T maxFreq);
    bool readSamples();
    void buildFFTBuffer();
    void NoiseAquisition(int n = 33 , int bias = 5);
    void computeFFT();
    void subNoiseBackground();
    HpsData Hps(T thresholdDb = 30, uint8_t nHarmonics = 3);
    void updateSG();
    void updateSpectrum(double dbLimite = 30, double lastFondamental = 0, bool lastBest = false, bool _selected = false);
    ~FFT_detected();
  private: 

    double correctionINMP441(double f);
    uint16_t dbToColor(double db);
  protected : 

  HpsData data;

    TFT_MultiCore &m_tft;

    T* _ringBuffer;

uint16_t _writeIndex = 0;
bool _bufferFull = false;

  uint16_t* _sgBuffer;
static constexpr int SG_W = 240;
static constexpr int SG_H = 120;

    int _pin_WS;
    int _pin_SD;
    int _pin_SCK;

    int _sampleRate;
    int _samples;

    T _minfreq;
    T _maxfreq;

    uint _minBinfreq;
    uint _maxBinfreq;

    double _referenceSpectrum[SG_W];
    bool _referenceValid = false;



    uint32_t *_i2sReadBuffer;
    uint32_t *_i2sReadNoiseBuffer;
        size_t _i2sReadBufferSize;

    T* _vRealNoise;

    T meanNoise;


    T * _vReal;
    T * _vImag;
    T* _backgroundNoise;

    std::shared_ptr<ArduinoFFT<T>> _FFT;

    void drawFreqScale(double minFreq, double maxFreq);


};
