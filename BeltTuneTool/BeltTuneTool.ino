/*MIT License

Copyright (c) 2026 Favrou jérôme

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "headers/DI.hpp"
#include "headers/PanelHerited.hpp"

// Parametres centralises de l'application.
namespace AppConfig
{
constexpr uint8_t kMenuMainIndex = 0;
constexpr uint8_t kMenuSettingIndex = 1;
constexpr uint8_t kMenuPresetIndex = 2;

constexpr uint8_t kButtonSelectPin = 26;
constexpr uint8_t kButtonBackwardPin = 27;
constexpr uint8_t kButtonForwardPin = 14;
constexpr uint8_t kBatteryAdcPin = 34;

constexpr uint16_t kScreenWidth = 240;
constexpr uint16_t kScreenHeight = 240;
constexpr uint16_t kHeaderHeight = 142;
constexpr uint16_t kTabHeight = 20;
constexpr uint16_t kPanelHeight = kScreenHeight - kHeaderHeight - kTabHeight;
constexpr uint16_t kMenuHeight = kScreenHeight - kHeaderHeight;
constexpr uint16_t kTopInfoHeight = 120;

constexpr uint32_t kInputPollDelayMs = 10;
constexpr uint32_t kLongPressDelayMs = 1000;
constexpr uint32_t kInitialRepeatDelayMs = 300;
constexpr uint32_t kUiRefreshDelayMs = 100;
constexpr uint32_t kStartupScreenDelayMs = 5000;
constexpr uint32_t kUsageScreenDelayMs = 8000;

constexpr float kAdcReferenceVoltage = 3.3f;
constexpr float kAdcMaxValue = 4095.0f;
constexpr float kBatteryDividerRatio = 2.0f;
constexpr float kBatteryCalibrationFactor = 1.08f;
constexpr float kBatteryPercentMinVoltage = 3.2f;
constexpr float kBatteryPercentMaxVoltage = 4.1f;
constexpr float kBatteryCriticalPercent = 10.0f;

constexpr uint64_t kDeepSleepWakeupUs = 3600ULL * 1000000ULL;

constexpr BaseType_t kTaskPriority = 1;
constexpr uint32_t kTaskCore0StackSize = 4096;
constexpr uint32_t kTaskCore1StackSize = 8096;
constexpr uint8_t kTaskCore0Id = 0;
constexpr uint8_t kTaskCore1Id = 1;

constexpr int kCenterX = kScreenWidth / 2;
constexpr int kStartupTitleY = 20;
constexpr int kStartupVersionY = 50;
constexpr int kStartupCopyrightY = 80;
constexpr int kStartupLicenseY = 110;
constexpr int kStartupRepoY = 140;

constexpr int kUsageSelectShortY = 10;
constexpr int kUsageSelectLongY = 30;
constexpr int kUsageAdjustY = 50;
constexpr int kUsageAdjustFastY = 70;

constexpr int kButtonLegendY = 220;
constexpr int kButtonLegendWidth = 44;
constexpr int kButtonLegendHeight = 18;
constexpr int kButtonLegendRadius = 4;
constexpr int kButtonLegendTextYOffset = 5;
constexpr int kSelectLegendX = 18;
constexpr int kBackwardLegendX = 98;
constexpr int kForwardLegendX = 178;

constexpr int kBiasNoise = 5;

constexpr double kTensionFactor = 4.0;
constexpr uint8_t kSpectrumSelectedValue = 0;

constexpr char kMainMenuName[] = "Main";
constexpr char kTaskCore0Name[] = "Task0";
constexpr char kTaskCore1Name[] = "Task1";

constexpr char kAppTitle[] = "BeltTuneTool";
constexpr char kAppVersion[] = "Version 1.0";
constexpr char kAppCopyright[] = "2026 - Jerome Favrou";
constexpr char kAppRepository[] = "github.com/jeromefavrou/BeltTuneTool";
constexpr char kAppLicense[] = "Licence : MIT";

constexpr char kUsageSelectShort[] = "SEL court : changer d'onglet";
constexpr char kUsageSelectLong[] = "SEL long : entrer/sortir du menu";
constexpr char kUsageAdjust[] = "FW/BW : modifier la valeur";
constexpr char kUsageAdjustFast[] = "FW/BW maintenu : defilement rapide";

constexpr char kSelectLabel[] = "SEL";
constexpr char kBackwardLabel[] = "BWD";
constexpr char kForwardLabel[] = "FWD";
}


// Taches FreeRTOS et ressources partagees entre acquisition et interface.
TaskHandle_t TaskCore0;
TaskHandle_t TaskCore1;

bool lockTft = false;

TFT_eSPI tft = TFT_eSPI();

TFT_MultiCore tftMultiCore(&tft);

std::shared_ptr<Menu> menu;

std::shared_ptr<FFT_detected<double>> FFT;


std::shared_ptr<MainPanel> mainPanel;
std::shared_ptr<SettingPanel> settingPanel;
std::shared_ptr<PresetPanel> presetPanel;

// Etat runtime expose a l'interface principale.
double last_freq = 0;
double seui_db = 25;

double mu = 0.77;      // kg/m
double L  = 0.5;       // m
bool Smode = false;

double tension = 0;
double accel = 0;
uint8_t functSel = 0;
bool selPanel = false;

// Gere les boutons et la navigation dans les panneaux de configuration.
void tacheCore0(void * parameter) 
{
  
  DIAvr bp_sel;
  DIAvr bp_bw;
  DIAvr bp_fw;

  bp_sel.init(AppConfig::kButtonSelectPin, INPUT_PULLUP , DI::Signal::PULL_UP);
  bp_bw.init(AppConfig::kButtonBackwardPin, INPUT_PULLUP, DI::Signal::PULL_UP);
  bp_fw.init(AppConfig::kButtonForwardPin, INPUT_PULLUP , DI::Signal::PULL_UP );

  uint32_t lastRepeatTime = 0;
  uint32_t repeatDelay = AppConfig::kInitialRepeatDelayMs;

  while (true) 
  {
    vTaskDelay(pdMS_TO_TICKS(AppConfig::kInputPollDelayMs));
     bp_sel.read();
     bp_bw.read();
     bp_fw.read();

   
    // Enregistre les reglages si l'utilisateur a demande une sauvegarde.
    settingPanel->saveToEEPROM();
         

      if( bp_sel.n() && !selPanel )
      {
            menu->nextPanel();

            continue;

      } 
      else if( bp_sel.n_fixed() && selPanel )
      {
        // En mode edition, un appui select passe au champ suivant.
          int raw = analogRead(AppConfig::kBatteryAdcPin);

          // Conversion approximative pour ADC 12 bits
          float adcVoltage = raw * AppConfig::kAdcReferenceVoltage / AppConfig::kAdcMaxValue;
           
          // On multiplie par 2 car le pont diviseur divise la tension par 2
          float batteryVoltage = adcVoltage * AppConfig::kBatteryDividerRatio * AppConfig::kBatteryCalibrationFactor; 
          mainPanel->setVbat( batteryVoltage );

          if( menu->getSelect() == 2)
          {
              presetPanel->nextSetting();
              presetPanel->draw();

              presetPanel->apply  =false;

          }
          else if( menu->getSelect() == 1)
          { 
             
            menu->getPanelSelected()->nextSetting();
            menu->getPanelSelected()->draw();
          }

            
          continue;
        
        
      }
      else if( bp_sel.p_fixed_Repeat(AppConfig::kLongPressDelayMs) == 2 && (menu->getSelect() == AppConfig::kMenuSettingIndex || menu->getSelect() == AppConfig::kMenuPresetIndex) )
      {

        // Bascule entre navigation d'onglets et edition du panneau courant.
          selPanel = !selPanel;

          tftMultiCore.lock();
          menu->getPanelSelected()->selPanel = selPanel;
          tftMultiCore.unlock();

          menu->draw();

          bp_fw.clear();
          bp_fw.clear();
          bp_sel.clear();
          
          vTaskDelay(pdMS_TO_TICKS(AppConfig::kLongPressDelayMs));

          continue;
      }

      if( (menu->getSelect() == AppConfig::kMenuSettingIndex || menu->getSelect() == AppConfig::kMenuPresetIndex) && selPanel )
      {

        if( bp_fw.p_fixed_Repeat(repeatDelay) == 1 )
        {
          menu->getPanelSelected()->fwdSetting();
          menu->getPanelSelected()->draw();
        }
        else if( bp_bw.p_fixed_Repeat(repeatDelay) == 1 )
        {
          menu->getPanelSelected()->bwdSetting();
          menu->getPanelSelected()->draw();

        }
        if( bp_fw.p_fixed_Repeat(repeatDelay) == 2 )
        {
          menu->getPanelSelected()->fwdSpeedSetting();
          menu->getPanelSelected()->draw();

        
          vTaskDelay(pdMS_TO_TICKS(repeatDelay/2));

        }
        else if( bp_bw.p_fixed_Repeat(repeatDelay) == 2 )
        {
          menu->getPanelSelected()->bwdSpeedSetting();
          menu->getPanelSelected()->draw();

          vTaskDelay(pdMS_TO_TICKS(repeatDelay/2));

        }

        continue;

      }

  
  }
}

// Gere l'acquisition audio, la FFT et le rendu des donnees detectees.
void tacheCore1(void * parameter)
{
    tftMultiCore.lock();
    settingPanel->setting.LearnNoise = false;
    tftMultiCore.unlock();

    while(true)
    {
        // Apprend le bruit de fond avant de lancer la detection nominale.
        if(!settingPanel->setting.LearnNoise)
        {
            FFT->NoiseAquisition(settingPanel->setting.nNoise , AppConfig::kBiasNoise);

            tftMultiCore.lock();
            settingPanel->setting.LearnNoise = true;
            tftMultiCore.unlock();
        }

        // Traite une nouvelle fenetre FFT quand assez d'echantillons sont prets.
        if(FFT->readSamples())
        {
            FFT->buildFFTBuffer();

            FFT->computeFFT();

            FFT->subNoiseBackground();

            HpsData hpsData =
                FFT->Hps(
                    seui_db,
                    settingPanel->setting.nHarmonics);

            bool lastBest = false;

            if(hpsData.lastMesures[0] >= settingPanel->setting.minHz &&
               hpsData.lastMesures[0] <= settingPanel->setting.maxHz)
            {
                last_freq = hpsData.lastMesures[0];

                lastBest = true;

                double tension =
                    AppConfig::kTensionFactor *
                    settingPanel->setting.mu *
                    settingPanel->setting.L *
                    settingPanel->setting.L *
                    last_freq *
                    last_freq;

                if(menu->getSelect() == AppConfig::kMenuMainIndex)
                {
                    mainPanel->setValues(
                        last_freq,
                        hpsData.score,
                        hpsData.stdDev,
                        tension);

                    mainPanel->draw();
                }
            }

            if(settingPanel->setting.Smode)
                FFT->updateSG();
            else
                FFT->updateSpectrum(
                    settingPanel->setting.seui_db,
                    last_freq,
                    lastBest,
                    functSel == AppConfig::kSpectrumSelectedValue);
        }

              // Rend explicitement la main au scheduler entre deux traitements.
        taskYIELD();
    }
}

          // Affiche l'ecran d'accueil dans la zone haute avant l'initialisation complete.
void drawStartupScreen(TFT_eSPI &tft)
{
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(2);

    tft.drawCentreString(AppConfig::kAppTitle, AppConfig::kCenterX, AppConfig::kStartupTitleY, 2);
    tft.drawCentreString(AppConfig::kAppVersion, AppConfig::kCenterX, AppConfig::kStartupVersionY, 2);
    tft.drawCentreString(AppConfig::kAppCopyright, AppConfig::kCenterX, AppConfig::kStartupCopyrightY, 2);
    tft.drawCentreString(AppConfig::kAppRepository, AppConfig::kCenterX, AppConfig::kStartupRepoY, 1);
    tft.drawCentreString(AppConfig::kAppLicense, AppConfig::kCenterX, AppConfig::kStartupLicenseY, 2);

}

  // Affiche l'aide d'utilisation et le rappel visuel des boutons physiques.
void drawUsageInfo(TFT_eSPI &tft)
{
    tft.fillRect(0, 0, AppConfig::kScreenWidth, AppConfig::kTopInfoHeight, TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.drawCentreString(AppConfig::kUsageSelectShort, AppConfig::kCenterX, AppConfig::kUsageSelectShortY, 1);
    tft.drawCentreString(AppConfig::kUsageSelectLong, AppConfig::kCenterX, AppConfig::kUsageSelectLongY, 1);
    tft.drawCentreString(AppConfig::kUsageAdjust, AppConfig::kCenterX, AppConfig::kUsageAdjustY, 1);
    tft.drawCentreString(AppConfig::kUsageAdjustFast, AppConfig::kCenterX, AppConfig::kUsageAdjustFastY, 1);

    tft.drawRoundRect(AppConfig::kSelectLegendX, AppConfig::kButtonLegendY, AppConfig::kButtonLegendWidth, AppConfig::kButtonLegendHeight, AppConfig::kButtonLegendRadius, TFT_WHITE);
    tft.drawCentreString(AppConfig::kSelectLabel, AppConfig::kSelectLegendX + AppConfig::kButtonLegendWidth / 2, AppConfig::kButtonLegendY + AppConfig::kButtonLegendTextYOffset, 1);

    tft.drawRoundRect(AppConfig::kBackwardLegendX, AppConfig::kButtonLegendY, AppConfig::kButtonLegendWidth, AppConfig::kButtonLegendHeight, AppConfig::kButtonLegendRadius, TFT_WHITE);
    tft.drawCentreString(AppConfig::kBackwardLabel, AppConfig::kBackwardLegendX + AppConfig::kButtonLegendWidth / 2, AppConfig::kButtonLegendY + AppConfig::kButtonLegendTextYOffset, 1);

    tft.drawRoundRect(AppConfig::kForwardLegendX, AppConfig::kButtonLegendY, AppConfig::kButtonLegendWidth, AppConfig::kButtonLegendHeight, AppConfig::kButtonLegendRadius, TFT_WHITE);
    tft.drawCentreString(AppConfig::kForwardLabel, AppConfig::kForwardLegendX + AppConfig::kButtonLegendWidth / 2, AppConfig::kButtonLegendY + AppConfig::kButtonLegendTextYOffset, 1);
}

  // Efface l'ecran complet avant un nouvel affichage plein ecran.
void clearScreen(TFT_eSPI &tft)
{
    tft.fillRect(0, 0, AppConfig::kScreenWidth, AppConfig::kScreenHeight, TFT_BLACK);
} 


  // Initialise l'ecran, la batterie, les panneaux et les taches FreeRTOS.
void setup()
{
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    analogSetPinAttenuation(AppConfig::kBatteryAdcPin, ADC_11db);

     int raw = analogRead(AppConfig::kBatteryAdcPin);

    // Mesure instantanee de la batterie a l'allumage.
    float adcVoltage = raw * AppConfig::kAdcReferenceVoltage / AppConfig::kAdcMaxValue;
     
    // Compensation du pont diviseur et de la calibration ADC.
    float batteryVoltage = adcVoltage * AppConfig::kBatteryDividerRatio * AppConfig::kBatteryCalibrationFactor; 
    


     tft.init();
     tft.setRotation(0);
     tft.fillScreen(TFT_BLACK);    
     
     
    // Conversion de la tension en estimation de pourcentage Li-Ion.
    float batteryPercent = (batteryVoltage - AppConfig::kBatteryPercentMinVoltage) / (AppConfig::kBatteryPercentMaxVoltage - AppConfig::kBatteryPercentMinVoltage) * 100.0f;
    batteryPercent = constrain(batteryPercent, 0.0f, 100.0f);
    
    if (batteryPercent < AppConfig::kBatteryCriticalPercent)
    {
        tft.fillScreen(TFT_BLACK);
        digitalWrite(TFT_BL, LOW);
    
      delay(AppConfig::kUiRefreshDelayMs);
    
      esp_sleep_enable_timer_wakeup(AppConfig::kDeepSleepWakeupUs);
    
        esp_deep_sleep_start();
    }

    // Cree l'instance FFT partagee entre la tache audio et les panneaux.
    FFT = std::shared_ptr<FFT_detected<double>>( new FFT_detected<double>(tftMultiCore,  SAMPLING_FREQUENCY, SAMPLES, I2S_WS, I2S_SD, I2S_SCK)); 

    drawStartupScreen(tft);
    delay(AppConfig::kStartupScreenDelayMs);
    clearScreen(tft);

    drawUsageInfo(tft);
    delay(AppConfig::kUsageScreenDelayMs);
    clearScreen(tft);


    // Instancie le menu et les panneaux utilises par l'interface.
    menu = std::shared_ptr<Menu>(new Menu( AppConfig::kMainMenuName, vect2(0, AppConfig::kHeaderHeight), vect2(AppConfig::kScreenWidth, AppConfig::kMenuHeight)));

     mainPanel = std::shared_ptr<MainPanel>(new MainPanel( tftMultiCore, vect2(0, AppConfig::kHeaderHeight), vect2(AppConfig::kScreenWidth, AppConfig::kPanelHeight)));
    settingPanel = std::shared_ptr<SettingPanel>(new SettingPanel( tftMultiCore, FFT, vect2(0, AppConfig::kHeaderHeight), vect2(AppConfig::kScreenWidth, AppConfig::kPanelHeight)));
     presetPanel = std::shared_ptr<PresetPanel>(new PresetPanel( tftMultiCore, FFT, settingPanel, vect2(0, AppConfig::kHeaderHeight), vect2(AppConfig::kScreenWidth, AppConfig::kPanelHeight)));
    
    
    settingPanel->loadFromEEPROM();
    
    menu->addPanel(mainPanel);
    menu->addPanel(settingPanel);
    menu->addPanel(presetPanel);

    mainPanel->setVbat( batteryVoltage );
    menu->draw();

    delay(AppConfig::kUiRefreshDelayMs);


    FFT->mappingFreq(settingPanel->setting.minHz, settingPanel->setting.maxHz);

    
    // Demarre la navigation utilisateur sur le coeur 0.
    xTaskCreatePinnedToCore(tacheCore0, AppConfig::kTaskCore0Name, AppConfig::kTaskCore0StackSize, NULL, AppConfig::kTaskPriority, &TaskCore0, AppConfig::kTaskCore0Id);

    // Demarre le traitement audio et l'analyse FFT sur le coeur 1.
    xTaskCreatePinnedToCore(tacheCore1, AppConfig::kTaskCore1Name, AppConfig::kTaskCore1StackSize, NULL, AppConfig::kTaskPriority, &TaskCore1, AppConfig::kTaskCore1Id);
}

  // Toute l'application tourne dans les taches FreeRTOS; la loop reste idle.
void loop()
{
    vTaskDelay(portMAX_DELAY);
}
