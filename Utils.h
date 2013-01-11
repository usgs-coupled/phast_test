#if !defined(UTILITIES_H_INCLUDED)
#define UTILITIES_H_INCLUDED

#include <string>
#include <sstream>				// std::istringstream std::ostringstream
#include <ostream>				// std::ostream
#include <istream>				// std::istream
#include <map>					// std::map

namespace Utilities
{

	const char INDENT[] = "  ";

	int strcmp_nocase(const char *str1, const char *str2);

	int strcmp_nocase_arg1(const char *str1, const char *str2);

	void str_tolower(std::string & str);
	void str_toupper(std::string & str);
	std::string pad_right(const std::string & str, size_t l);
	bool replace(const char *str1, const char *str2, std::string & str);

	void squeeze_white(std::string & s_l);
	double convert_time(double t, std::string in, std::string out);
	double safe_exp(double t);
}
#endif // UTILITIES_H_INCLUDED
