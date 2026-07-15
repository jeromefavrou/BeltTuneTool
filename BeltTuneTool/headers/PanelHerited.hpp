#pragma once

#include "Menu.hpp"
#include "Belt.hpp"
#include "FFT_detected.hpp"
#include <EEPROM.h>

class MainPanel : public Panel
{
public:

    MainPanel(TFT_MultiCore & _tft, vect2 pos, vect2 size);

    void setValues(float freq, float db, float sigma, float tension);

    void setVbat(double val);

    void draw() override;

private:

    void drawBattery();

    float m_freq = 0;
    float m_db = 0;
    float m_sigma = 0;
    float m_tension = 0;

    float vbat = 0;
};

class SettingPanel : public Panel
{
  public:
   struct Setting
   {
       uint32_t magic = 0x12345678;

      float mu;
      float L;
      float seui_db;
      float minHz;
      float maxHz;
      
      unsigned int nHarmonics;
      unsigned int nNoise;
      
      bool SaveSetting;
      bool LearnNoise;
      bool Smode;

            Setting();
   };


        SettingPanel(TFT_MultiCore & _tft, std::shared_ptr<FFT_detected<double>> fft, vect2 pos, vect2 size);
    
        void nextSetting() override;

        void fwdSetting() override;

        void bwdSetting() override;

        void fwdSpeedSetting() override;

        void bwdSpeedSetting() override;

        void saveToEEPROM();

        void loadFromEEPROM();



    void draw() override;

    Setting setting;

    private:

    std::shared_ptr<FFT_detected<double>> m_fft;

    void mapSettingInLimite(void);


};

struct BeltData
{   
  float linearMass;  // kg/m
  float length;      // m

};

class PresetPanel : public Panel
{
public:

    PresetPanel(TFT_MultiCore & _tft, std::shared_ptr<FFT_detected<double>> fft, std::shared_ptr<SettingPanel> settingPanel, vect2 pos, vect2 size);

    void nextSetting() override;

    void fwdSetting() override;

    void bwdSetting() override;

    void fwdSpeedSetting() override;

    void bwdSpeedSetting() override;

    void draw() override;

    BeltData getSelected() const;
    bool apply = false;

    protected : 

    void validate(SettingPanel::Setting & _setting);
private:

    std::shared_ptr<FFT_detected<double>> m_fft;
    std::shared_ptr<SettingPanel> m_settingPanel;

    size_t m_selected = 0;

    unsigned int f = 0; // index du type de courroie
    unsigned int w = 0; // index de la largeur
    unsigned int l = 0; // index de la longueur
    unsigned int p = 0; // index du pas



};