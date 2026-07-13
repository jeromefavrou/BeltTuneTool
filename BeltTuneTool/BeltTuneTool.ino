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

#include "FFT_detected.hpp"
#include "DI.hpp"
#include "Menu.hpp"
#include <map>
#include <EEPROM.h>

//catalogue de courroie
#include "Belt.h"

TaskHandle_t TaskCore0;
TaskHandle_t TaskCore1;

bool lockTft = false;

TFT_eSPI tft = TFT_eSPI();

TFT_MultiCore tftMultiCore(&tft);

std::shared_ptr<Menu> menu;

SharedPtr<FFT_detected<double>> FFT;


class MainPanel : public Panel
{
public:

    MainPanel(TFT_MultiCore & _tft, vect2 pos, vect2 size): Panel( _tft ,"Main", pos, size)
    {
    }

    void setValues(float freq,float db,float sigma,float tension)
    {
        this->m_tft.lock();

        m_freq = freq;
        m_db = db;
        m_sigma = sigma;
        m_tension = tension;

        this->m_tft.unlock();
    }

    void setVbat(double val)
    {
      this->m_tft.lock();
      vbat = val;
      this->m_tft.unlock();
    }

    void draw() override;

private:

  void drawBattery( )
  {
      // Position : bas droite
      const int X = 225;
      const int Y = 170;

      const int W = 10;
      const int H = 40;

      // Adaptation selon ta batterie
      const double VBAT_MIN = 3.30;   // batterie vide
      const double VBAT_MAX = 4.10;   // batterie pleine

      // Conversion en %
      int percent = (vbat - VBAT_MIN) * 100.0 / (VBAT_MAX - VBAT_MIN);

      percent = constrain(percent, 0, 100);

      // Calcul de la hauteur remplie
      int fillHeight = percent * (H - 2) / 100;

      // Couleur suivant le niveau
      uint16_t color;

      if(percent > 50)
          color = TFT_GREEN;
      else if(percent > 20)
          color = TFT_YELLOW;
      else
          color = TFT_RED;

      // Efface la zone
      m_tft.tft->fillRect(X - 20, Y - 15, 40, H + 25, TFT_BLACK);

      // Contour batterie
      m_tft.tft->drawRect(X, Y, W, H, TFT_WHITE);

      // Petit plot supérieur
      m_tft.tft->fillRect(X + W/4, Y - 4, W/2, 4, TFT_WHITE);

      // Partie vide
      m_tft.tft->fillRect(X + 1,
                  Y + 1,
                  W - 2,
                  H - 2,
                  TFT_BLACK);

      // Partie remplie (depuis le bas)
      m_tft.tft->fillRect(X + 1,
                  Y + H - 1 - fillHeight,
                  W - 2,
                  fillHeight,
                  color);

      // Pourcentage
      m_tft.tft->setTextColor(TFT_WHITE, TFT_BLACK);
      m_tft.tft->drawCentreString(String(percent) + "%",X + W/2,Y + H + 2,1);
  }

    float m_freq = 0;
    float m_db = 0;
    float m_sigma = 0;
    float m_tension = 0;

    float vbat = 0;
};

void MainPanel::draw()
{

    this->m_tft.lock();


    
    m_tft.tft->fillRect(
        m_position.x,
        m_position.y,
        m_size.x,
        m_size.y,
        TFT_BLACK);


    this->drawBattery();

    //---------------------------------
    // Première ligne
    //---------------------------------

    int y = m_position.y + 6;

    m_tft.tft->setTextColor(TFT_WHITE, TFT_BLACK);

    m_tft.tft->drawCentreString(
        String(m_tension,1) + "N",
        40,
        y,
        2);

    m_tft.tft->drawCentreString(
        String(m_db,1) + "dB",
        120,
        y,
        2);

    m_tft.tft->drawCentreString(
        String(m_sigma,2),
        200,
        y,
        2);

    //---------------------------------
    // Tension
    //---------------------------------

    m_tft.tft->setTextColor(TFT_GREEN,TFT_BLACK);

    m_tft.tft->drawCentreString(
        String(m_freq,1) + " Hz",
        m_position.x + m_size.x/2,
        m_position.y + 30,
        4);



      

    this->m_tft.unlock();
}

std::shared_ptr<MainPanel> mainPanel;




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

      Setting()
          : mu(0.77f),
            L(0.5f),
            Smode(false),
            seui_db(25.0f),
            minHz(60.0f),
            maxHz(300.0f),
            nHarmonics(3),
            nNoise(33),
            SaveSetting(false),
            LearnNoise(false)
      {
      }
   };


    SettingPanel(TFT_MultiCore & _tft, vect2 pos, vect2 size): Panel(_tft, "Setting", pos, size)
    {
      setting = Setting();
    }
    

    void nextSetting() 
    {
          this->m_tft.lock();
      m_selected = (m_selected + 1) % 10;

      this->m_tft.unlock();
    }

    void fwdSetting() 
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
        case 8: setting.nNoise +=1 ; break;
        case 9: setting.SaveSetting = true; break;
      }

      mapSettingInLimite();

      this->m_tft.unlock();

                  //si minhz ou maxhz modifié on recalcul le spectre

      if(m_selected == 6 || m_selected == 7)
      {
        FFT->mappingFreq(setting.minHz,setting.maxHz);
      }

    }

    void bwdSetting()
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
         case 8: setting.nNoise -=1 ; break;
         case 9: setting.SaveSetting = true; break;  
      }
      mapSettingInLimite();

      


      this->m_tft.unlock();

            //si minhz ou maxhz modifié on recalcul le spectre

      if(m_selected == 6 || m_selected == 7)
      {
        FFT->mappingFreq(setting.minHz,setting.maxHz);
      }

    }

    void fwdSpeedSetting()
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
         case 8: setting.nNoise +=5 ; break;
      }
      mapSettingInLimite();

      this->m_tft.unlock();

                  //si minhz ou maxhz modifié on recalcul le spectre

      if(m_selected == 6 || m_selected == 7)
      {
        FFT->mappingFreq(setting.minHz,setting.maxHz);
      }

    }

    void bwdSpeedSetting()
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
         case 8: setting.nNoise -=5 ; break;
      }

      mapSettingInLimite();

      this->m_tft.unlock();

                  //si minhz ou maxhz modifié on recalcul le spectre

      if(m_selected == 6 || m_selected == 7)
      {
        FFT->mappingFreq(setting.minHz,setting.maxHz);
      }

    }

     void saveToEEPROM()
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

    void loadFromEEPROM()
    {

          this->m_tft.lock();
      EEPROM.begin(512);
      EEPROM.get(0, this->setting);
      EEPROM.end();

      if (this->setting.magic != 0x12345678)
      {
          this->setting = Setting();

      }

      this->setting.SaveSetting = false;

        this->m_tft.unlock();
    }



    void draw() override;

    Setting setting;

    private:

    void mapSettingInLimite(void)
    {


      // Masse linéique (kg/m)
      if(setting.mu < 0.001f) setting.mu = 0.001f;
      if(setting.mu > 5.0f)   setting.mu = 5.0f;

      // Longueur de courroie (m)
      if(setting.L < 0.05f) setting.L = 0.05f;
      if(setting.L > 5.0f)  setting.L = 5.0f;

      // Seuil en dB
      if(setting.seui_db < 1.0f)  setting.seui_db = 0.0f;
      if(setting.seui_db > 80.0f) setting.seui_db = 80.0f;

      // Fréquence minimale
      if(setting.minHz < 30.0f)
          setting.minHz = 30.0f;

      // Fréquence maximale
      if(setting.maxHz > 4000.0f)
          setting.maxHz = 4000.0f;

      // Toujours min < max
      if(setting.minHz >= setting.maxHz)
          setting.minHz = setting.maxHz - 1.0f;

      if(setting.minHz < 1.0f)
          setting.minHz = 1.0f;

      // Nombre d'harmoniques
      if(setting.nHarmonics < 2)
          setting.nHarmonics = 2;

      if(setting.nHarmonics > 10)
          setting.nHarmonics = 10;

      // Nombre de mesures de bruit
      if(setting.nNoise < 1)
          setting.nNoise = 1;

      if(setting.nNoise > 200)
          setting.nNoise = 200;


    }


};

void SettingPanel::draw()
{
  this->m_tft.lock();



   m_tft.tft->fillRect(
        m_position.x,
        m_position.y,
        m_size.x,
        m_size.y,
        TFT_BLACK);

    const int x1 = m_position.x + 20;
    const int x2 = m_position.x + 142;

    const int y0 = m_position.y + 4;
    const int dy = 14;

    auto drawItem = [&](int index,int x,int y,const String& txt)
    {
        m_tft.tft->setTextColor(index == m_selected ? TFT_GREEN : TFT_WHITE,TFT_BLACK);

        m_tft.tft->drawString(txt, x, y, 2);
    };

    drawItem(0, x1, y0 + 0*dy, "Mu : " + String(setting.mu,3));
    drawItem(1, x2, y0 + 0*dy, "L : " + String(setting.L,3));

    drawItem(2, x1, y0 + 1*dy, "Noise : " + String(setting.LearnNoise ? "Yes" : "No"));
    drawItem(3, x2, y0 + 1*dy, "Mode : " + String(setting.Smode ? "SG" : "SP"));

    drawItem(4, x1, y0 + 2*dy, "dB : " + String(setting.seui_db,0));
    drawItem(5, x2, y0 + 2*dy, "H : " + String(setting.nHarmonics));

    drawItem(6, x1, y0 + 3*dy, "Min : " + String(setting.minHz,0));
    drawItem(7, x2, y0 + 3*dy, "Max : " + String(setting.maxHz,0));

    drawItem(8, x1, y0 + 4*dy, "NBiais : " + String(setting.nNoise));
    drawItem(9, x2, y0 + 4*dy, "SAVE : " + String(setting.SaveSetting ? "Yes" : "No"));

    this->m_tft.unlock();

}


std::shared_ptr<SettingPanel> settingPanel;

struct BeltData
{   
  float linearMass;  // kg/m
  float length;      // m

};

class PresetPanel : public Panel
{
public:



    PresetPanel(TFT_MultiCore & _tft, vect2 pos, vect2 size) : Panel(_tft, "Preset", pos, size)
    {

       
    }

        void nextSetting() 
    {
          this->m_tft.lock();
      m_selected = (m_selected + 1) % 5;

      this->m_tft.unlock();
    }

    void fwdSetting() override
{
    this->m_tft.lock();

    switch (m_selected)
    {
        case 0: // Famille
            f = (f + 1) % BeltCatalogCount;
            p = 0;
            w = 0;
            l = 0;
            break;

        case 1: // Pas
            p = (p + 1) % BeltCatalog[f].pitchCount;
            w = 0;
            l = 0;
            break;

        case 2: // Largeur
            w = (w + 1) %
                BeltCatalog[f].pitches[p].widthCount;
            break;

        case 3: // Longueur
            l = (l + 1) %
                BeltCatalog[f].pitches[p].lengthCount;
            break;

        case 4: // Appliquer
            apply = true;
            this->validate(settingPanel->setting);
            
            break;
    }

    this->m_tft.unlock();

     if(apply)
        FFT->mappingFreq(settingPanel->setting.minHz, settingPanel->setting.maxHz);
}

void bwdSetting() override
{
    this->m_tft.lock();

    switch (m_selected)
    {
        case 0: // Famille
            f = (f + BeltCatalogCount - 1) % BeltCatalogCount;
            p = 0;
            w = 0;
            l = 0;
            break;

        case 1: // Pas
            p = (p + BeltCatalog[f].pitchCount - 1)
                % BeltCatalog[f].pitchCount;
            w = 0;
            l = 0;
            break;

        case 2: // Largeur
            w = (w + BeltCatalog[f].pitches[p].widthCount - 1)
                % BeltCatalog[f].pitches[p].widthCount;
            break;

        case 3: // Longueur
            l = (l + BeltCatalog[f].pitches[p].lengthCount - 1)
                % BeltCatalog[f].pitches[p].lengthCount;
            break;

        case 4: // Appliquer
            apply = true;
            this->validate(settingPanel->setting);
            break;
    }

    this->m_tft.unlock();
    
     if(apply)
          FFT->mappingFreq(settingPanel->setting.minHz, settingPanel->setting.maxHz);
}

void fwdSpeedSetting() 
{
    
    this->m_tft.lock();

    switch (m_selected)
    {
        case 0: // Famille
            f = (f + 1) % BeltCatalogCount;
            p = 0;
            w = 0;
            l = 0;
            break;

        case 1: // Pas
            p = (p + 1) % BeltCatalog[f].pitchCount;
            w = 0;
            l = 0;
            break;

        case 2: // Largeur
            w = (w + 1) %
                BeltCatalog[f].pitches[p].widthCount;
            break;

        case 3: // Longueur
            l = (l + 1) %
                BeltCatalog[f].pitches[p].lengthCount;
            break;

        case 4: // Appliquer
            apply = true;
            this->validate(settingPanel->setting);
            
            break;
    }

    this->m_tft.unlock();

     if(apply)
        FFT->mappingFreq(settingPanel->setting.minHz, settingPanel->setting.maxHz);
}

void bwdSpeedSetting()
{
    this->m_tft.lock();

    switch (m_selected)
    {
        case 0: // Famille
            f = (f + BeltCatalogCount - 1) % BeltCatalogCount;
            p = 0;
            w = 0;
            l = 0;
            break;

        case 1: // Pas
            p = (p + BeltCatalog[f].pitchCount - 1)
                % BeltCatalog[f].pitchCount;
            w = 0;
            l = 0;
            break;

        case 2: // Largeur
            w = (w + BeltCatalog[f].pitches[p].widthCount - 1)
                % BeltCatalog[f].pitches[p].widthCount;
            break;

        case 3: // Longueur
            l = (l + BeltCatalog[f].pitches[p].lengthCount - 1)
                % BeltCatalog[f].pitches[p].lengthCount;
            break;

        case 4: // Appliquer
            apply = true;
            this->validate(settingPanel->setting);
            break;
    }

    this->m_tft.unlock();
    
     if(apply)
          FFT->mappingFreq(settingPanel->setting.minHz, settingPanel->setting.maxHz);
}

    void draw() override;

    BeltData getSelected() const;
    bool apply = false;

    protected : 

    void validate(SettingPanel::Setting & _setting)
    {

      _setting.mu = BeltCatalog[f].pitches[p].widths[w].linearMass;
      _setting.L = BeltCatalog[f].pitches[p].lengths[l].mm / 2000.0f; // 2 longeur de corde maximale = 1/2 de la longeur de la courroie dans le cas d'une poulie infiniment petite

      //calcule de la frequance maxi et mini possible pour un corde de 1/2 de L et pour une corde de 5% de L
      const float Lmax = _setting.L;


    _setting.minHz = (1.0f / (2.0f * Lmax)) *
                     sqrtf( 100.0 / _setting.mu);

    if(_setting.minHz <30 )
        _setting.minHz = 30;

    _setting.maxHz = _setting.minHz *3 ;

     if(_setting.maxHz > 4000 )
        _setting.maxHz = 4000;


    }
private:

    size_t m_selected = 0;

    unsigned int f = 0; // index du type de courroie
    unsigned int w = 0; // index de la largeur
    unsigned int l = 0; // index de la longueur
    unsigned int p = 0; // index du pas



};

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

    m_tft.tft->fillRect(
        m_position.x,
        m_position.y,
        m_size.x,
        m_size.y,
        TFT_BLACK);

    const BeltPitch& pitch = BeltCatalog[f].pitches[p];

    const int x = m_position.x + 8;
    int y = m_position.y + 15;
    const int dy = 14;

    auto drawItem = [&](int index, const String& txt)
    {
        m_tft.tft->setTextColor(
            (m_selected == index) ? TFT_GREEN : TFT_WHITE,
            TFT_BLACK);

        m_tft.tft->drawString(txt, x, y, 2);

        y += dy;
    };

    drawItem(
        0,
        "Famille : " + String(BeltCatalog[f].name));

    drawItem(
        1,
        "Pas : " + String(pitch.name));

    drawItem(
        2,
        "Largeur : " +
        String(pitch.widths[w].mm) + " mm");

    drawItem(
        3,
        "Longueur : " +
        String(pitch.lengths[l].mm) + " mm");


    m_tft.tft->setTextColor((m_selected == 4) ? TFT_GREEN : TFT_WHITE,TFT_BLACK);

        m_tft.tft->drawString(String(!apply?"VALIDER":"<VALIDER>"), m_position.x +170 , m_position.y+30, 2);

    //---------------------------------
    // Résumé
    //---------------------------------

    BeltData data = getSelected();


    m_tft.tft->setTextColor(TFT_BLUE, TFT_BLACK);

    m_tft.tft->drawString(
        "Mu : " + String(data.linearMass,4) + " kg/m",
        x,
        m_position.y ,
        2);

    this->m_tft.unlock();
}
std::shared_ptr<PresetPanel> presetPanel;




double last_freq = 0;
double seui_db = 25;

double mu = 0.77;      // kg/m
double L  = 0.5;       // m
bool Smode = false;

double tension = 0;
double accel = 0;
uint8_t functSel = 0;
bool selPanel = false;

void tacheCore0(void * parameter) 
{
  
  DIAvr bp_sel;
  DIAvr bp_bw;
  DIAvr bp_fw;

  bp_sel.init(26, INPUT_PULLUP , DI::Signal::PULL_UP);
  bp_bw.init(27, INPUT_PULLUP, DI::Signal::PULL_UP);
  bp_fw.init(14, INPUT_PULLUP , DI::Signal::PULL_UP );

  uint32_t lastRepeatTime = 0;
  uint32_t repeatDelay = 300;   // ms (départ lent)

  while (true) 
  {
    vTaskDelay(pdMS_TO_TICKS(10)); // vérification toutes les 10 ms
     bp_sel.read();
     bp_bw.read();
     bp_fw.read();

   
    //si option a true
    settingPanel->saveToEEPROM();
         

      if( bp_sel.n() && !selPanel )
      {
            menu->nextPanel();

            continue;

      } 
      else if( bp_sel.n_fixed() && selPanel )
      {
        //on navigue dans le panel
          int raw = analogRead(34);

          // Conversion approximative pour ADC 12 bits
          float adcVoltage = raw * 3.3 / 4095.0;
           
          // On multiplie par 2 car le pont diviseur divise la tension par 2
          float batteryVoltage = adcVoltage * 2.0  *1.08; 
          mainPanel->setVbat( batteryVoltage );

          if( menu->getSelect() == 2)
          {
              presetPanel->nextSetting();
              presetPanel->draw();

              presetPanel->apply  =false;

             // settingPanel->setting.L = beltData.length / 2.0; // longeure de corde au plus long 1/2 de la longeur courroie dans le cas de poulie infiniment petite
             // settingPanel->setting.mu = beltData.linearMass;
          }
          else if( menu->getSelect() == 1)
          { 
             
            menu->getPanelSelected()->nextSetting();
            menu->getPanelSelected()->draw();
          }

            
          continue;
        
        
      }
      else if( bp_sel.p_fixed_Repeat(1000 ) == 2 && (menu->getSelect() == 1 || menu->getSelect() == 2) )
      {

        //changement de navigaton
          selPanel = !selPanel;

          tftMultiCore.lock();
          menu->getPanelSelected()->selPanel = selPanel;
          tftMultiCore.unlock();

          menu->draw();

          bp_fw.clear();
          bp_fw.clear();
          bp_sel.clear();
          
          vTaskDelay(pdMS_TO_TICKS(1000));

          continue;
      }

      if( (menu->getSelect() == 1 || menu->getSelect() == 2) && selPanel )
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



void tacheCore1(void * parameter)
{
    tftMultiCore.lock();
    settingPanel->setting.LearnNoise = false;
    tftMultiCore.unlock();

    while(true)
    {
        // Acquisition du bruit
        if(!settingPanel->setting.LearnNoise)
        {
            FFT->NoiseAquisition(settingPanel->setting.nNoise);

            tftMultiCore.lock();
            settingPanel->setting.LearnNoise = true;
            tftMultiCore.unlock();
        }

        // Lecture d'un bloc de 512 échantillons
        if(FFT->readSamples())
        {
            // Construit les 4096 derniers échantillons
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
                    4.0 *
                    settingPanel->setting.mu *
                    settingPanel->setting.L *
                    settingPanel->setting.L *
                    last_freq *
                    last_freq;

                if(menu->getSelect() == 0)
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
                    functSel == 0);
        }

        // Plus besoin d'attendre 10 ms
        taskYIELD();
    }
}

//affichage des edvertissemnt , version , information diverses au démrarrage du programme dans la partie Haute ( 120 pixels)
void drawStartupScreen(TFT_eSPI &tft)
{
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(2);

    tft.drawCentreString("BeltTuneTool", 120, 20, 2);
    tft.drawCentreString("Version 1.0", 120, 50, 2);
    tft.drawCentreString("2026 - Jerome Favrou", 120, 80, 2);
    tft.drawCentreString("github.com/jeromefavrou/BeltTuneTool", 120, 140, 1);

    //licence

    tft.drawCentreString("Licence : MIT", 120, 110, 2);

}

//Affichage des informations d'utilisation sur les 120 pixel du haut
void drawUsageInfo(TFT_eSPI &tft)
{
    tft.fillRect(0, 0, 240, 120, TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);

     tft.drawCentreString("SEL court : changer d'onglet", 120, 10, 1);
    tft.drawCentreString("SEL long : entrer/sortir du menu", 120, 30, 1);
    tft.drawCentreString("FW/BW : modifier la valeur", 120, 50, 1);
    tft.drawCentreString("FW/BW maintenu : defilement rapide", 120, 70, 1);

    // --------------------------
    // Représentation des boutons
    // --------------------------
    const int y = 220;
    const int w = 44;
    const int h = 18;
    const int r = 4;

    // SEL
    tft.drawRoundRect(18, y, w, h, r, TFT_WHITE);
    tft.drawCentreString("SEL", 18 + w/2, y + 5, 1);

    // BWD
    tft.drawRoundRect(98, y, w, h, r, TFT_WHITE);
    tft.drawCentreString("BWD", 98 + w/2, y + 5, 1);

    // FWD
    tft.drawRoundRect(178, y, w, h, r, TFT_WHITE);
    tft.drawCentreString("FWD", 178 + w/2, y + 5, 1);
}



void clearScreen(TFT_eSPI &tft)
{
    tft.fillRect(0, 0, 240, 240, TFT_BLACK);
} 



void setup()
{
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    analogSetPinAttenuation(34, ADC_11db);

     int raw = analogRead(34);

    // Conversion approximative pour ADC 12 bits
    float adcVoltage = raw * 3.3 / 4095.0;
     
    // On multiplie par 2 car le pont diviseur divise la tension par 2
    float batteryVoltage = adcVoltage * 2.0  *1.08; 
    


     tft.init();
     tft.setRotation(0);
     tft.fillScreen(TFT_BLACK);    
     
     
     // Estimation du pourcentage (Li-Ion 3.0V -> 4.2V)
    float batteryPercent = (batteryVoltage - 3.2f) / (4.1f - 3.2f) * 100.0f;
    batteryPercent = constrain(batteryPercent, 0.0f, 100.0f);
    
    if (batteryPercent < 10.0f)
    {
        tft.fillScreen(TFT_BLACK);
        digitalWrite(TFT_BL, LOW);   // Coupe le rétroéclairage
    
        delay(100);
    
        // Réveil toutes les heures (exemple)
        esp_sleep_enable_timer_wakeup(3600ULL * 1000000ULL);
    
        esp_deep_sleep_start();
    }

    FFT = SharedPtr<FFT_detected<double>>( new FFT_detected<double>(tftMultiCore,  SAMPLING_FREQUENCY, SAMPLES, I2S_WS, I2S_SD, I2S_SCK)); 

    drawStartupScreen(tft);
    delay(5000);
    clearScreen(tft);

    drawUsageInfo(tft);
    delay(8000);
    clearScreen(tft);


    menu = std::shared_ptr<Menu>(new Menu( "Main", vect2(0, 142), vect2(240, 240-142)));

   mainPanel = std::shared_ptr<MainPanel>(new MainPanel( tftMultiCore, vect2(0, 142), vect2(240, 240-142 - 20)));
   settingPanel = std::shared_ptr<SettingPanel>(new SettingPanel( tftMultiCore, vect2(0, 142), vect2(240, 240-142 - 20)));
    presetPanel = std::shared_ptr<PresetPanel>(new PresetPanel( tftMultiCore, vect2(0, 142), vect2(240, 240-142 - 20)));
    
    
    settingPanel->loadFromEEPROM();
    
    menu->addPanel(mainPanel);
    menu->addPanel(settingPanel);
    menu->addPanel(presetPanel);

    mainPanel->setVbat( batteryVoltage );
    menu->draw();

    delay(100);


    FFT->mappingFreq(settingPanel->setting.minHz, settingPanel->setting.maxHz);

    
    //demare le core 0
    xTaskCreatePinnedToCore(tacheCore0,"Task0",4096,NULL, 1, &TaskCore0, 0  );

    //demare le core1 
    xTaskCreatePinnedToCore( tacheCore1,"Task1",8096,NULL,1,&TaskCore1,1 );
}


void loop()
{
    vTaskDelay(portMAX_DELAY);
}
