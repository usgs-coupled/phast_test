#if defined(__WPHAST__)
#include "../StdAfx.h"
#endif

#define EXTERNAL extern
#include "hstinpt.h"
#undef EXTERNAL

#include <cassert>

// Note: No header files should follow the next three lines
#if defined(_WIN32) && defined(_DEBUG)
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

cunit::cunit(const char *m_si):
input(0),
si(0),
input_to_si(1.0),
input_to_user(1.0),
defined(FALSE)
{
	this->si = new char[::strlen(m_si) + 1];
	::strcpy(this->si, m_si);
}

cunit::~cunit(void)
{
	delete[] si;
	delete[] input;
}

cunit::cunit(const cunit & src):
input(0),
si(0),
input_to_si(src.input_to_si),
input_to_user(src.input_to_user),
defined(src.defined)
{
	if (src.input)
	{
		this->input = new char[::strlen(src.input) + 1];
		::strcpy(this->input, src.input);
	}
	if (src.si)
	{
		this->si = new char[::strlen(src.si) + 1];
		::strcpy(this->si, src.si);
	}
}

cunit & cunit::operator=(const cunit & rhs)
{
	if (this != &rhs)
	{
		delete[] this->input;
		this->input = 0;
		if (rhs.input)
		{
			this->input = new char[::strlen(rhs.input) + 1];
			::strcpy(this->input, rhs.input);
		}

		delete[] this->si;
		this->si = 0;
		if (rhs.si)
		{
			this->si = new char[::strlen(rhs.si) + 1];
			::strcpy(this->si, rhs.si);
		}

		this->input_to_si = rhs.input_to_si;
		this->input_to_user = rhs.input_to_user;
		this->defined = rhs.defined;
	}
	return *this;
}

int
cunit::set_input(const char *m_input)
{
	if (!m_input)
		return ERROR;

	if (this->defined == TRUE)
	{
		assert(this->input);
		delete[] this->input;
	}
	this->defined = TRUE;
	this->input = new char[::strlen(m_input) + 1];
	::strcpy(this->input, m_input);

	assert(this->si);
	int n =::units_conversion(this->input, this->si, &this->input_to_si,
							  FALSE);
	if (n != OK)
	{
		delete[] this->input;
		this->input = 0;
		this->defined = FALSE;
	}
	else
	{
		if (this->input_to_si == 1.0)
		{
			delete[] this->input;
			this->input = 0;
			this->defined = FALSE;
		}
	}
	return n;
}

const char *
cunit::c_str(void) const
{
	if (this->defined == TRUE)
	{
		assert(this->input);
		return this->input;
	}
	else
	{
		assert(this->si);
		return this->si;
	}
}

void
cunit::define(const char *m_input)
{
	assert(m_input);
	if (!m_input)
		return;
	if (this->defined == TRUE)
	{
		assert(this->input);
		delete[] this->input;
	}
	this->defined = TRUE;
	this->input = new char[::strlen(m_input) + 1];
	::strcpy(this->input, m_input);
}

void
cunit::undefine(void)
{
	if (this->defined == TRUE)
	{
		assert(this->input);
	}
	delete[] this->input;
	this->input = 0;
	this->defined = FALSE;
}
