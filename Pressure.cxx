// Pressure.cxx: implementation of the cxxPressure class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#pragma warning(disable : 4786)	// disable truncation warning (Only used by debugger)
#endif
#include <cassert>				// assert
#include <algorithm>			// std::sort

#include "Utils.h"				// define first
#include "Parser.h"
#include "Phreeqc.h"
#include "Pressure.h"
#include "phqalloc.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cxxPressure::cxxPressure(PHRQ_io *io)
	//
	// default constructor for cxxPressure 
	//
:	cxxNumKeyword(io)
{
	count = 0;
	equalIncrements = false;
}

cxxPressure::~cxxPressure()
{
}
int
cxxPressure::read(CParser & parser)
{
/*
 *      Reads pressure data for reaction steps
 *
 *      Arguments:
 *	 none
 *
 *      Returns:
 *	 KEYWORD if keyword encountered, input_error may be incremented if
 *		    a keyword is encountered in an unexpected position
 *	 EOF     if eof encountered while reading mass balance concentrations
 *	 ERROR   if error occurred reading data
 *
 */
	// Number and description set in read_reaction_pressure

	CParser::LINE_TYPE lt;
	bool done = false;
	for (;;)
	{
		// new line
		//LINE_TYPE check_line(const std::string & str, bool allow_empty,
		//				 bool allow_eof, bool allow_keyword, bool print);
		std::istream::pos_type ptr;
		std::istream::pos_type next_char = 0;
		std::string token, str;
		lt = parser.check_line(str, false, true, true, true);

		if (lt == CParser::LT_EMPTY || 
			lt == CParser::LT_KEYWORD ||
			lt == CParser::LT_EOF)
		{
			break;
		}
		if (lt == CParser::LT_OPTION)
		{
			this->error_msg("Expected numeric value for pressures.", CParser::OT_CONTINUE);
			break;
		}

		if (done)
		{
			this->error_msg("Unknown input following equal increment definition.", CParser::OT_CONTINUE);
			continue;
		}

		// LT_OK

		for (;;)
		{
			// new token
			std::string token;
			CParser::TOKEN_TYPE k =	parser.copy_token(token, next_char);

			// need new line
			if (k == CParser::TT_EMPTY)
			{
				break;
			}

			// read a pressure
			if (k == CParser::TT_DIGIT)
			{
				std::istringstream iss(token);
				LDBLE d;
				if (!(iss >> d))
				{
					this->error_msg("Expected numeric value for pressures.",
									 CParser::OT_CONTINUE);
				}
				else
				{
					this->pressures.push_back(d);
				}
				continue;
			}

			// non digit, must be "in"
			if (k == CParser::TT_UPPER || k == CParser::TT_LOWER)
			{
				if (this->pressures.size() != 2)
				{
					this->error_msg("To define equal increments, exactly two pressures should be defined.", CONTINUE);
				}
				else
				{
					int i = parser.copy_token(token, next_char);
					if (i == EMPTY)
					{
						error_msg("To define equal increments, define 'in n steps'.", CONTINUE);
					}
					else
					{
						std::istringstream iss(token);
						if ((iss >> i) && i > 0)
						{
							this->equalIncrements = true;
							this->count = i;
						}
						else
						{
							error_msg("Unknown input for pressure steps.", CONTINUE);
						}
					}
					done = true;
				}
				if (k == CParser::TT_UNKNOWN)
				{
					error_msg("Unknown input for pressure steps.", CONTINUE);
				}
			}
		} // tokens
	} // lines
	return lt;
}

void
cxxPressure::dump_raw(std::ostream & s_oss, unsigned int indent, int *n_out) const
{
	//const char    ERR_MESSAGE[] = "Packing temperature message: %s, element not found\n";
	unsigned int i;
	s_oss.precision(DBL_DIG - 1);
	std::string indent0(""), indent1(""), indent2("");
	for (i = 0; i < indent; ++i)
		indent0.append(Utilities::INDENT);
	for (i = 0; i < indent + 1; ++i)
		indent1.append(Utilities::INDENT);
	for (i = 0; i < indent + 2; ++i)
		indent2.append(Utilities::INDENT);

	s_oss << indent0;
	int n_user_local = (n_out != NULL) ? *n_out : this->n_user;
	s_oss << "REACTION_PRESSURE_RAW        " << n_user_local << " " << this->description << std::endl;

	s_oss << indent1;
	s_oss << "-count              " << this->count << std::endl;

	s_oss << indent1;
	s_oss << "-equal_increments   " << this->equalIncrements << std::endl;

	// Temperature element and attributes

	s_oss << indent1;
	s_oss << "-pressures          " << std::endl;
	{
		int i = 0;
		s_oss << indent2;
		for (std::vector < double >::const_iterator it = this->pressures.begin();
			 it != this->pressures.end(); it++)
		{
			if (i++ == 5)
			{
				s_oss << std::endl;
				s_oss << indent2;
				i = 0;
			}
			s_oss << *it << " ";
		}
		s_oss << std::endl;
	}
}

void
cxxPressure::read_raw(CParser & parser)
{
	// clear steps for modify operation, if pressures are read
	bool cleared_once = false;
	double d;
	CParser::TOKEN_TYPE k;
	static std::vector < std::string > vopts;
	if (vopts.empty())
	{
		vopts.reserve(5);
		vopts.push_back("pressures");	        //0
		vopts.push_back("equal_increments");	//1
		vopts.push_back("count");	            //2
	}

	std::istream::pos_type ptr;
	std::istream::pos_type next_char = 0;
	std::string token;
	int opt_save;
	bool useLastLine(false);

	// Number and description set in read_reaction_pressure_raw

	opt_save = CParser::OPT_ERROR;
	bool equalIncrements_defined(false);
	bool count_defined(false);

	for (;;)
	{
		int opt;
		if (useLastLine == false)
		{
			opt = parser.get_option(vopts, next_char);
		}
		else
		{
			opt = parser.getOptionFromLastLine(vopts, next_char);
		}
		if (opt == CParser::OPT_DEFAULT)
		{
			opt = opt_save;
		}
		switch (opt)
		{
		case CParser::OPT_EOF:
			break;
		case CParser::OPT_KEYWORD:
			break;
		case CParser::OPT_DEFAULT:
		case CParser::OPT_ERROR:
			opt = CParser::OPT_EOF;
			parser.error_msg("Unknown input in REACTION_PRESSURE_RAW keyword.",
							 CParser::OT_CONTINUE);
			parser.error_msg(parser.line().c_str(), CParser::OT_CONTINUE);
			useLastLine = false;
			break;

		case 0:				// pressures
			if (!cleared_once) 
			{
				this->pressures.clear();
				cleared_once = true;
			}
			while ((k =	parser.copy_token(token, next_char)) == CParser::TT_DIGIT)
			{
				std::istringstream iss(token);
				if (!(iss >> d))
				{
					parser.incr_input_error();
					parser.error_msg("Expected numeric value for pressures.",
									 CParser::OT_CONTINUE);
				}
				else
				{
					this->pressures.push_back(d);
				}
			}
			opt_save = 0;
			useLastLine = false;
			break;

		case 1:				// equal_increments
			if (!(parser.get_iss() >> this->equalIncrements))
			{
				this->equalIncrements = 0;
				parser.incr_input_error();
				parser.error_msg("Expected boolean value for equalIncrements.", CParser::OT_CONTINUE);
			}
			opt_save = CParser::OPT_DEFAULT;
			useLastLine = false;
			equalIncrements_defined = true;
			break;

		case 2:				// count
			if (!(parser.get_iss() >> this->count))
			{
				this->count = 0;
				parser.incr_input_error();
				parser.error_msg("Expected integer value for count.", CParser::OT_CONTINUE);
			}
			opt_save = CParser::OPT_DEFAULT;
			useLastLine = false;
			count_defined = true;
			break;
		}
		if (opt == CParser::OPT_EOF || opt == CParser::OPT_KEYWORD)
			break;
	}
	// members that must be defined
	if (equalIncrements_defined == false)
	{
		parser.incr_input_error();
		parser.error_msg("Equal_increments not defined for REACTION_PRESSURE_RAW input.", 
			CParser::OT_CONTINUE);
	}
	if (count_defined == false)
	{
		parser.incr_input_error();
		parser.error_msg("Count_temps not defined for REACTION_PRESSURE_RAW input.",
			 CParser::OT_CONTINUE);
	}
}
#ifdef SKIP
void
cxxPressure::dump_xml(std::ostream & s_oss, unsigned int indent) const const
{
	//const char    ERR_MESSAGE[] = "Packing temperature message: %s, element not found\n";
	unsigned int i;
	s_oss.precision(DBL_DIG - 1);
	std::string indent0(""), indent1(""), indent2("");
	for (i = 0; i < indent; ++i)
		indent0.append(Utilities::INDENT);
	for (i = 0; i < indent + 1; ++i)
		indent1.append(Utilities::INDENT);
	for (i = 0; i < indent + 2; ++i)
		indent2.append(Utilities::INDENT);

	// Temperature element and attributes
	s_oss << indent0;
	s_oss << "<temperature " << std::endl;

	s_oss << indent1;
	s_oss << "pitzer_temperature_gammas=\"" << this->
		pitzer_temperature_gammas << "\"" << std::endl;

	// components
	s_oss << indent1;
	s_oss << "<component " << std::endl;
	for (std::list < cxxPressureComp >::const_iterator it =
		 temperatureComps.begin(); it != temperatureComps.end(); ++it)
	{
		it->dump_xml(s_oss, indent + 2);
	}

	return;
}
#endif