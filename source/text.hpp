
#ifndef __TEXT_HPP
#define __TEXT_HPP

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <algorithm>

using namespace std;

// Nothing to do with CText. Q&D helpers for string formating.
const char *fmt(const char *format, ...);
std::string sfmt(const char *format, ...);
std::string vectorToString(const vector<std::string> &vect, std::string sep);
vector<std::string> stringToVector(const std::string &text, char sep);
std::string upperCase(std::string text);
std::string lowerCase(std::string text);
std::string ltrim(std::string s);
std::string rtrim(std::string s);
void Asciify( wchar_t *str );
void Asciify2( char *str );

#endif // !defined(__TEXT_HPP)
