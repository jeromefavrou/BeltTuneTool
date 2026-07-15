#include "headers/DI.hpp"

DI::DI(void):Ebool(false),m_freq(.0),m_pin(0),m_sig(Signal::PULL_UP)
{
	lastRepeatTime = millis();
}

void DI::set_filter_freq(float freq)
{
	this->m_freq = freq;
}

bool DI::p_fixed(void)
{
	bool tmp = this->p();

	if(tmp)
		this->set_bool(true);

	return tmp;
}

bool DI::n_fixed(void)
{
	bool tmp = this->n();

	if(tmp)
		this->set_bool(false);

	return tmp;
}

int DI::p_fixed_Repeat(uint32_t const delay_ms)
{
	if(this->p_fixed() && millis() - lastRepeatTime < delay_ms)
	{
		lastRepeatTime = millis();
		return 1;
	}
	else if(millis() - lastRepeatTime >= delay_ms)
	{
		return 2;
	}

	return 0;
}

int DI::n_fixed_Repeat(uint32_t const delay_ms)
{
	if(this->n_fixed() && millis() - lastRepeatTime < delay_ms)
	{
		lastRepeatTime = millis();
		return 1;
	}
	else if(millis() - lastRepeatTime >= delay_ms)
	{
		lastRepeatTime = millis();
		return 2;
	}

	lastRepeatTime = millis();

	return 0;
}

DIAvr::DIAvr(void):DI()
{
}

DIAvr::DIAvr(uint8_t const _pin, uint8_t const _mode , Signal const & _sig):DI()
{
	this->init(_pin , _mode , _sig);
}

DIAvr DIAvr::operator=(bool const & cpy)
{
	this->set_bool(cpy);
	return *this;
}

bool DIAvr::init(uint8_t const _pin, uint8_t const _mode , Signal const & _sig)
{
	if(_mode != INPUT && _mode != INPUT_PULLUP)
		return false;

	this->m_pin = _pin;
	this->m_sig = _sig;

	pinMode(this->m_pin, _mode);

	this->m_filter = millis();

	return true;
}

bool DIAvr::read(Filter const type_filter)
{
	bool r_value = (bool) digitalRead(this->m_pin);

	if(this->m_sig == Signal::PULL_UP)
		r_value = !r_value;

	switch(type_filter)
	{
		case Filter::NONE:
			this->set_bool(r_value);
			break;

		case Filter::PASS_LOW:
			if(r_value != this->stat() && millis() - this->m_filter >= 1000.0 / this->m_freq)
			{
				this->m_filter = millis();
				this->set_bool(r_value);
			}
			break;

		default:
			this->set_bool(r_value);
			break;
	}

	if(!this->stat())
		lastRepeatTime = millis();

	return this->stat();
}

