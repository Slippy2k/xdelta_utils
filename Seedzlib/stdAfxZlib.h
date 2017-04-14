
#include <windows.h>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <iostream>
#include "utf_convert.h"
#include <regex>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


#ifdef _UNICODE
using tstring = std::wstring;
using tstringstream = std::wstringstream;
using tifstream = std::wifstream;
#	define _T(x) L ## x
#	define tcout std::wcout
#	define tostream std::wostream
#	define tregex std::wregex
#	define tchar_to_long _wtol
#	define tstrcmp wcscmp
#else
using tstring = std::string;
using tstringstream = std::wstringstream;
using tifstream = std::ifstream;

#	define _T(x) x
#	define tcout std::cout
#	define tostream std::ostream
#	define tregex std::regex
#	define tchar_to_long atol
#	define tstrcmp strcmp
#endif

typedef std::vector< tregex > vecRegex;


