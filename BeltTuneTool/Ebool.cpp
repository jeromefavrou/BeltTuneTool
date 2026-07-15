#include "headers/Ebool.hpp"


/// Initialise l'état interne à 0.
Ebool::Ebool(void): m_ebool(uint8_t(0))
{
}

/// Construit l'objet à partir d'un booléen simple.
Ebool::Ebool(bool const & cpy): m_ebool(static_cast<uint8_t>(cpy))
{
}

/// Construit une copie à partir d'un autre Ebool.
Ebool::Ebool(Ebool const & cpy): m_ebool(cpy.get_Ebool())
{
}

/// Affecte la valeur interne depuis un autre Ebool.
Ebool Ebool::operator=(Ebool const & cpy)
{
	this->m_ebool = cpy.get_Ebool();
	return *this;
}

/// Affecte un booléen et historise la nouvelle valeur.
Ebool Ebool::operator=(bool const & cpy)
{
	this->set_bool(cpy);
	return *this;
}

/// Retourne vrai si l'état précédent était inférieur à l'état courant.
bool Ebool::p(void) const
{
	return this->m_ebool % 2 > (this->m_ebool >> 1) % 2;
}

/// Retourne vrai si l'état précédent était supérieur à l'état courant.
bool Ebool::n(void) const
{
	return this->m_ebool % 2 < (this->m_ebool >> 1) % 2;
}

/// Retourne l'état booléen courant.
bool Ebool::stat(void) const
{
	return this->m_ebool % 2 == 1 ? true : false;
}

/// Réinitialise complètement l'état interne.
void Ebool::clear(void)
{
	this->m_ebool = uint8_t(0);
}

/// Détruit l'objet Ebool.
Ebool::~Ebool(void)
{
}

/// Empile la nouvelle valeur booléenne dans l'historique binaire.
void Ebool::set_bool(bool const & cpy)
{
	this->m_ebool = (this->m_ebool << 1) + static_cast<uint8_t>(cpy ? 1 : 0);
}

/// Retourne la valeur brute stockée dans l'objet.
uint8_t Ebool::get_Ebool(void) const
{
	return this->m_ebool;
}

