
#pragma once

#include <stdint.h>

class Ebool
{
public:
    Ebool(void);
    Ebool(bool const & cpy);
    Ebool(Ebool const & cpy);
    Ebool operator=(Ebool const & cpy);
    Ebool operator=(bool const & cpy);

    bool p(void) const;
    bool n(void) const;
    bool stat(void) const;

    void clear(void);

	virtual ~Ebool(void);

protected:

    void set_bool(bool const & cpy);

    uint8_t get_Ebool(void) const;

private:
    uint8_t m_ebool;
};

