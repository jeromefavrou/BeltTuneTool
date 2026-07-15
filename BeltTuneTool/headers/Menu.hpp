
#pragma once

#include <stdint.h>
#include "TFT_MultiCore.hpp"
#include <string>
#include <vector>
#include <memory>


class vect2
{
    public:

    vect2(int _x, int _y);

    int x;
    int y;
};


class Panel
{
public:
    Panel(TFT_MultiCore& tft, std::string name, vect2 position, vect2 size);

    virtual ~Panel();

    virtual void draw();

    void drawOnglet(vect2 pos, vect2 size, bool selected);

    std::string name() const;

    virtual void onEnter();
    virtual void onLeave();

    virtual void nextSetting();

    virtual void fwdSetting();

    virtual void bwdSetting();

    virtual void fwdSpeedSetting();

    virtual void bwdSpeedSetting();
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
  
        Menu(std::string name, vect2 position, vect2 size);

        void addPanel(std::shared_ptr<Panel> panel);

        void removePanel(unsigned int index);

        void draw();

        void nextPanel();

        uint8_t getSelect() const;

    
        std::shared_ptr<Panel> getPanelSelected();



  protected:

    std::string m_name;
    vect2 m_position;
    vect2 m_size;

    std::vector< std::shared_ptr<Panel> > m_panels;

    uint8_t m_selected = 0;

};
