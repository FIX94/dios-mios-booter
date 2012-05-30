#include "text.hpp"

using namespace std;

// Simplified use of sprintf
const char *fmt(const char *format, ...)
{
	enum {
		MAX_MSG_SIZE	= 512,
		MAX_USES		= 8
	};

	static int currentStr = 0;
	currentStr = (currentStr + 1) % MAX_USES;

	va_list va;
	va_start(va, format);
	static char buffer[MAX_USES][MAX_MSG_SIZE];
	vsnprintf(buffer[currentStr], MAX_MSG_SIZE, format, va);
	buffer[currentStr][MAX_MSG_SIZE - 1] = '\0';
	va_end(va);

	return buffer[currentStr];
}

string sfmt(const char *format, ...)
{
	va_list va;
	va_start(va, format);
	u32 length = vsnprintf(0, 0, format, va) + 1;
	va_end(va);
	char *tmp = new char[length + 1];
	va_start(va, format);
	vsnprintf(tmp, length, format, va);
	va_end(va);
	string s = tmp;
	delete[] tmp;
	return s;
}

string vectorToString(const vector<string> &vect, string sep)
{
	string s;
	for (u32 i = 0; i < vect.size(); ++i)
	{
		if (i > 0)
			s.append(sep);
		s.append(vect[i]);
	}
	return s;
}

vector<string> stringToVector(const string &text, char sep)
{
	vector<string> v;
	if (text.empty()) return v;
	u32 count = 1;
	for (u32 i = 0; i < text.size(); ++i)
		if (text[i] == sep)
			++count;
	v.reserve(count);
	string::size_type off = 0;
	string::size_type i = 0;
	do
	{
		i = text.find_first_of(sep, off);
		if (i != string::npos)
		{
			string ws(text.substr(off, i - off));
			v.push_back(ws);
			off = i + 1;
		}
		else
			v.push_back(text.substr(off));
	} while (i != string::npos);
	return v;
}

string upperCase(string text)
{
	char c;
	for (string::size_type i = 0; i < text.size(); ++i)
	{
		c = text[i];
		if (c >= 'a' && c <= 'z')
			text[i] = c & 0xDF;
	}
	return text;
}


string lowerCase(string text)
{
	char c;
	for (string::size_type i = 0; i < text.size(); ++i)
	{
		c = text[i];
		if (c >= 'A' && c <= 'Z')
			text[i] = c | 0x20;
	}
	return text;
}

// trim from start
string ltrim(string s)
{
	s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
	return s;
}

// trim from end
string rtrim(string s)
{
	s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

void Asciify( wchar_t *str )
{
	const wchar_t *ptr = str;
	wchar_t *ctr = str;
	
	while(*ptr != '\0')
    {
		switch(*ptr)
		{
			case 0x14c:
				*ctr = 0x4f;
				break;
		}
		*ctr = *ptr;
		++ptr;
		++ctr;	
	}
	*ctr = '\0';
}

void Asciify2( char *str )
{
	u8 i=0;
	for( i=0; i < strlen(str); ++i )
	{
		if( str[i] < 0x20 || str[i] > 0x7F )
			str[i] = '_';
		else {
			switch( str[i] )
			{
				case '*':
				case '\"':
				case '|':
				case '<':
				case '>':
				case '?':
					str[i] = '_';
				break;
			}
		}
	}
}
