#include "headers/Menu.hpp"

// Constructor for vect2
vect2::vect2(int _x, int _y):x(_x), y(_y)
{
}
Panel::Panel(TFT_MultiCore& tft, std::string name, vect2 position, vect2 size)
	: m_name(name), m_position(position), m_size(size), m_tft(tft)
{
}
Panel::~Panel()
{
}
void Panel::draw()
{
}
void Panel::drawOnglet(vect2 pos, vect2 size, bool selected)
{
	this->m_tft.lock();
	uint16_t bg = selected ? (selPanel ? TFT_RED : TFT_BLUE) : TFT_DARKGREY;
	uint16_t fg = TFT_WHITE;

	m_tft.tft->fillRoundRect(pos.x, pos.y, size.x, size.y, 4, bg);
	m_tft.tft->drawRoundRect(pos.x, pos.y, size.x, size.y, 4, TFT_WHITE);
	m_tft.tft->setTextColor(fg, bg);
	m_tft.tft->drawCentreString(m_name.c_str(), pos.x + size.x / 2, pos.y + 6, 2);

	this->m_tft.unlock();
}
std::string Panel::name() const
{
	return m_name;
}
void Panel::onEnter()
{
}
void Panel::onLeave()
{
}
void Panel::nextSetting()
{
}
void Panel::fwdSetting()
{
}
void Panel::bwdSetting()
{
}
void Panel::fwdSpeedSetting()
{
}
void Panel::bwdSpeedSetting()
{
}
Menu::Menu(std::string name, vect2 position, vect2 size)
	: m_name(name), m_position(position), m_size(size)
{
	m_selected = 0;
}
void Menu::addPanel(std::shared_ptr<Panel> panel)
{
	m_panels.push_back(panel);
}
void Menu::removePanel(unsigned int index)
{
	if(index >= m_panels.size())
		return;
	m_panels.erase(m_panels.begin() + index);
}
void Menu::draw()
{
	if(m_panels.empty())
		return;
	m_panels[m_selected]->draw();

	const int h = 20;
	const int w = m_size.x / m_panels.size();
	for(size_t i = 0; i < m_panels.size(); i++)
	{
		m_panels[i]->drawOnglet(vect2(m_position.x + i * w, m_position.y + m_size.y - h), vect2(w, h), i == m_selected);
	}
}
void Menu::nextPanel()
{
	if(m_panels.empty())
		return;
	m_panels[m_selected]->onLeave();

	m_selected = (m_selected + 1) % m_panels.size();
	m_panels[m_selected]->onEnter();

	this->draw();
}
uint8_t Menu::getSelect() const
{
	return this->m_selected;
}
std::shared_ptr<Panel> Menu::getPanelSelected()
{
	return m_panels[this->getSelect()];
}
