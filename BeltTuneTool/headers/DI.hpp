
#pragma once

#include <Arduino.h>
#include "Ebool.hpp"



class DI : public Ebool
{
  public:

  enum class Filter : uint8_t{NONE =0 , PASS_LOW };
  enum class Signal : uint8_t{PULL_DOWN, PULL_UP };
  
  DI(void);

  virtual bool init(uint8_t const _pin, uint8_t const _mode , Signal const & _sig)=0;

  virtual bool read(Filter const type_filter =Filter::NONE)=0;
  
  void set_filter_freq(float freq);

  //front montant avec lecture alterante
  bool p_fixed(void);
  
  //front descendant avec lecture alterante
  bool n_fixed(void);

  int p_fixed_Repeat(uint32_t const delay_ms);

  int n_fixed_Repeat(uint32_t const delay_ms);

  protected:
  
  uint8_t m_pin ;
  uint64_t m_filter;
  float m_freq;

  Signal m_sig;

  uint32_t lastRepeatTime = 0;
  
};


class DIAvr : public DI
{
  public :
    DIAvr(void);
    DIAvr(uint8_t const _pin, uint8_t const _mode , Signal const & _sig);
    DIAvr operator=(bool const & cpy);
    bool init(uint8_t const _pin, uint8_t const _mode , Signal const & _sig);
    bool read(Filter const type_filter =Filter::NONE);



};
