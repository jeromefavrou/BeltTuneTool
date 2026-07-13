
#ifndef DI_h
#define DI_h

#include <Ebool.h> // https://github.com/jeromefavrou/Ebool
#include <SharedPtr.hpp>


class DI : public Ebool
{
  public:

  enum class Filter : uint8_t{NONE =0 , PASS_LOW };
  enum class Signal : uint8_t{PULL_DOWN, PULL_UP };
  
  DI(void):Ebool(false),m_freq(.0),m_pin(0),m_sig(Signal::PULL_UP)
  {
    lastRepeatTime = millis();
  }

  virtual bool init(uint8_t const _pin, uint8_t const _mode , Signal const & _sig)=0;

  virtual bool read(Filter const type_filter =Filter::NONE)=0;
  
  void set_filter_freq(float freq)
  {
    this->m_freq= freq;
  }

  //front montant avec lecture alterante
  bool p_fixed(void)
  {
    bool tmp = this->p();

    if(tmp)
      this->set_bool(true);
    
    return tmp;
  }
  
  //front descendant avec lecture alterante
  bool n_fixed(void)
  {
    bool tmp = this->n();

    if(tmp)
      this->set_bool(false);
    
    return tmp;
  }

  int p_fixed_Repeat(uint32_t const delay_ms)
  {
    if(this->p_fixed() && millis()-lastRepeatTime < delay_ms)
    {
      lastRepeatTime = millis();

      return 1;
    }
    else if(millis()-lastRepeatTime >= delay_ms)
    {

      return 2;
    }

    return 0;
  }

  int n_fixed_Repeat(uint32_t const delay_ms)
  {
    if(this->n_fixed() && millis()-lastRepeatTime < delay_ms)
    {
      lastRepeatTime = millis();

      return 1;
    }
    else if(millis()-lastRepeatTime >= delay_ms)
    {
      lastRepeatTime = millis();
      return 2;
    }

    lastRepeatTime = millis();

    return 0;
  }

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
    DIAvr(void):DI()
    {

    }

    DIAvr(uint8_t const _pin, uint8_t const _mode , Signal const & _sig):DI()
    {
      this->init(_pin , _mode , _sig);
    }

    DIAvr operator=(bool const & cpy)
    {
      this->set_bool(cpy);
      return *this;
    }

    bool init(uint8_t const _pin, uint8_t const _mode , Signal const & _sig)
    {
      if(_mode != INPUT && _mode != INPUT_PULLUP)
        return false;

      this->m_pin = _pin;
      this->m_sig = _sig;
      
      pinMode(this->m_pin  , _mode);

      this->m_filter=millis();
      
      return true;
    }

    bool read(Filter const type_filter =Filter::NONE)
    {
      
      bool r_value = (bool) digitalRead(this->m_pin);

      if(this->m_sig == Signal::PULL_UP)
        r_value = !r_value;

      switch(type_filter)
      {
        case Filter::NONE :
          this->set_bool(r_value);
        break;
        

        case Filter::PASS_LOW :
        if( r_value != this->stat() && millis() - this->m_filter >= 1000.0/this->m_freq)
        {
            this->m_filter = millis();
            this->set_bool(r_value);
        }
          
        break;

        default : this->set_bool(r_value);  break;
        
      }

      if(!this->stat())
        lastRepeatTime = millis();
        
     return this->stat();
    }



};

template<typename mcpT> class DIMcp : public DI
{
  protected :
    SharedPtr<mcpT> m_mcp;

  public :
    DIMcp(SharedPtr<mcpT> & mcp):DI(),m_mcp(mcp)
    {

    }

    DIMcp(SharedPtr<mcpT> & mcp,uint8_t const _pin, uint8_t const _mode , Signal const & _sig):DI(),m_mcp(mcp)
    {
      this->init(_pin , _mode , _sig);
    }

    DIMcp operator=(bool const & cpy)
    {
      this->set_bool(cpy);
      return *this;
    }

    bool init(uint8_t const _pin, uint8_t const _mode , Signal const & _sig)
    {
      if(_mode != INPUT && _mode != INPUT_PULLUP || m_mcp.isNull())
        return false;

      this->m_pin = _pin;
      this->m_sig = _sig;
      
      m_mcp->pinMode(this->m_pin  , _mode);

      this->m_filter=millis();
      
      return true;
    }

    bool read(Filter const type_filter =Filter::NONE)
    {
      if( m_mcp.isNull())
        return this->stat();

      bool r_value = (bool) m_mcp->digitalRead(this->m_pin);

      if(this->m_sig == Signal::PULL_UP)
        r_value = !r_value;

      switch(type_filter)
      {
        case Filter::NONE :
          this->set_bool(r_value);
        break;
        

        case Filter::PASS_LOW :
        if( r_value != this->stat() && millis() - this->m_filter >= 1000.0/this->m_freq)
        {
            this->m_filter = millis();
            this->set_bool(r_value);
        }
          
        break;

        default : this->set_bool(r_value);  break;
        
      }
      
     return this->stat();
    }
};

#endif
