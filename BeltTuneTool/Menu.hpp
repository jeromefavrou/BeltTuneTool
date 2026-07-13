
#ifndef MENU_h
#define MENU_h

#include "TFT_MultiCore.h"
#include <string>
#include <vector>
#include <memory>


class vect2
{
    public:

    vect2(int _x, int _y):x(_x),y(_y){};

    int x;
    int y;
};


class Panel
{
public:
    Panel(TFT_MultiCore& tft, std::string name,vect2 position, vect2 size): m_name(name),m_position(position),m_size(size),m_tft(tft){}

    virtual ~Panel(){}

    virtual void draw()
    {
    }

    void drawOnglet(vect2 pos,vect2 size,bool selected)
    {
        this->m_tft.lock();

        uint16_t bg = selected ? (selPanel ? TFT_RED : TFT_BLUE) : TFT_DARKGREY;
        uint16_t fg = TFT_WHITE;

        m_tft.tft->fillRoundRect(pos.x,pos.y,size.x,size.y,4,bg);

        m_tft.tft->drawRoundRect(pos.x,pos.y,size.x,size.y,4,TFT_WHITE);

        m_tft.tft->setTextColor(fg,bg);

        m_tft.tft->drawCentreString(m_name.c_str(),pos.x + size.x/2,pos.y + 6,2);

        this->m_tft.unlock();
    }

    std::string name() const { return m_name; }

    virtual void onEnter(){}
    virtual void onLeave(){}

    virtual void nextSetting()
    {
    }

    virtual void fwdSetting()
    {
    }

    virtual void bwdSetting()
    {
    }

    virtual void fwdSpeedSetting()
    {
    }

    virtual void bwdSpeedSetting()
    {
    }
    bool selPanel = false;

protected:

    std::string m_name;
    vect2 m_position;
    vect2 m_size;

    uint8_t m_selected = 0;

    TFT_MultiCore & m_tft;

};


class Menu
{
  public:
  
    Menu(  std::string name, vect2 position, vect2 size):m_name(name),m_position(position),m_size(size){m_selected = 0;};

    void addPanel(std::shared_ptr<Panel> panel)
    {
        m_panels.push_back(panel);
    }

    void removePanel(unsigned int index)
    {
        if( index >= m_panels.size() )
            return ;

        m_panels.erase(m_panels.begin() + index);
    }

void draw()
  {
      if(m_panels.empty())
          return;

      // Dessine le panel courant
      m_panels[m_selected]->draw();

      // ---------------------
      // Onglets
      // ---------------------

      const int h = 20;
    const int w = m_size.x / m_panels.size();

      for(size_t i=0;i<m_panels.size();i++)
      {
          m_panels[i]->drawOnglet(vect2(m_position.x + i*w,m_position.y + m_size.y-h),vect2(w,h),i==m_selected);
      }
  }

    void nextPanel()
    {
        if(m_panels.empty())
            return;
       
        m_panels[m_selected]->onLeave();

        m_selected = (m_selected + 1) % m_panels.size();

        m_panels[m_selected]->onEnter();

        this->draw();

    }

    uint8_t getSelect() const
    {
      return this->m_selected;
    }

    
    std::shared_ptr<Panel> getPanelSelected()
    {
      return m_panels[this->getSelect()];
    }



  protected:

    std::string m_name;
    vect2 m_position;
    vect2 m_size;

    std::vector< std::shared_ptr<Panel> > m_panels;

    uint8_t m_selected = 0;

};

#endif
