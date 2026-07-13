
#ifndef Ebool_h
#define Ebool_h

class Ebool
{
public:
    Ebool(void):m_ebool(uint8_t(0)){}

    Ebool(bool const & cpy):m_ebool(static_cast<uint8_t>(cpy)){}
    Ebool(Ebool const & cpy):m_ebool(cpy.get_Ebool()){}
    Ebool operator=(Ebool const & cpy)
    {
        this->m_ebool=cpy.get_Ebool();
        return *this;
    }
    Ebool operator=(bool const & cpy)
    {
        this->set_bool(cpy);
        return *this;
    }

    bool p(void) const {return this->m_ebool%2 > (this->m_ebool>>1)%2;}
    bool n(void) const {return this->m_ebool%2 < (this->m_ebool>>1)%2;}
    bool stat(void)const{return this->m_ebool%2==1?true:false;}

    void clear(void){this->m_ebool=uint8_t(0);}

	virtual ~Ebool(void)
	{
	}

protected:

	void set_bool(bool const & cpy)
    {
		this->m_ebool=(this->m_ebool << 1) + static_cast<uint8_t>(cpy?1:0);
    }

	uint8_t get_Ebool(void) const {return this->m_ebool;}

private:
    uint8_t m_ebool;
};

#endif

