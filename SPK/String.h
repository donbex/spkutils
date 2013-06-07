#ifndef __UTILS_STRING_H__
#define __UTILS_STRING_H__

// character converstion macros
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))

#define NullString String("")
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#ifdef QT_DLL
#include <qstring.h>
#endif

#define SPRINTBUFFER 100
#ifdef CY_USESECURE
#define SPRINTF(a,b) sprintf_s ( a, SPRINTBUFFER, b
#else
#define SPRINTF(a,b) sprintf ( a, b 
#endif
typedef unsigned int STRINGLEN;


class String 
{
public:
//	String ( String str ) { m_s = str.c_str(); }
//	String ( const String &str ) { m_s = str.c_str(); }
	String ( const std::string &str ) { m_s = str; }
	String ( const char *str ) { m_s = std::string(str); }
	String ( const long str ) { FromInt ( str ); }
	String ( const float str ) { FromFloat ( str ); }
	String ( const double str ) { FromFloat ( (float)str ); }
	String ( const char str ) { m_s = str; }
#ifdef QT_DLL
	String ( QString &str ) { if ( str.isEmpty() ) m_s = ""; else m_s = str.data(); }
	String ( const QString &str ) { if ( str.isEmpty() ) m_s = ""; else m_s = str.data(); }
#endif
	String () {}

	~String () {}

#ifdef QT_DLL
	String operator= ( QString str ) 
	{ 
		if ( str.isEmpty() ) 
			m_s = ""; 
		else 
			m_s = str.data();
		return (*this); 
	}
//	String &operator= ( const QString &str ) { if ( str.isEmpty() ) m_s = ""; else m_s = str.data(); return (*this); }
#endif
	String &operator= ( const std::string &str ) { m_s = str; return (*this); }
	String &operator= ( const char *str ) { m_s = std::string(str); return (*this); }
	String operator= ( String str ) { if ( str.Empty() ) m_s = ""; else m_s = str.ToString(); return (*this); }
	String &operator= ( const long str ) { FromInt ( str ); return (*this); }
	String &operator= ( const float str ) { FromFloat ( str ); return (*this); }
	String &operator= ( const double str ) { FromFloat ( (float)str ); return (*this); }
	String &operator= ( const char str ) { m_s = str; return (*this); }

#ifdef QT_DLL
	String operator+ ( QString &str ) { return String(m_s + str.data()); }
	String operator+ ( const QString &str ) { return String(m_s + str.data()); }
#endif
	String operator+ ( String str ) { return String(m_s + str.ToString()); }
	String operator+ ( const char *str ) { return String(m_s + std::string(str)); }
	String operator+ ( const std::string &str ) { return String(m_s + str); }
	String operator+ ( const char str ) { return String(m_s + str); }
	String operator+ ( const long  str ) { String s(str); s = m_s + s.ToString(); return s; }
	String operator+ ( const float str ) { String s(str); s = m_s + s.ToString(); return s; }
	String operator+ ( const double str ) { String s(str); s = m_s + s.ToString(); return s; }

	String &operator+= ( String str ) { m_s += str.ToString(); return (*this); }
	String &operator+= ( const char *str ) { m_s += std::string(str); return (*this); }
	String &operator+= ( const std::string &str ) { m_s += str; return (*this); }
	String &operator+= ( const char str ) { m_s += str; return (*this); }
	String &operator+= ( const long  str ) { String s(str); m_s += s.ToString(); return (*this); }
	String &operator+= ( const float str ) { String s(str); m_s += s.ToString(); return (*this); }
	String &operator+= ( const double str ) { String s(str); m_s += s.ToString(); return (*this); }

	const char operator[] ( int num ) { return m_s[num]; }

#ifdef QT_DLL
	bool operator== ( QString &str ) 
	{ 
		return (m_s == str.data());
	}
#endif
	bool operator!= ( String str )
	{ 
		if ( str.Length() != m_s.length() ) return true;
		for ( STRINGLEN i = 0; i < str.Length(); i++ )
			if ( LOWER(m_s[i]) != LOWER(str[i]) )
				return true;
		return false; 
	}
	bool operator!= ( const char *str ) 
	{ 
		if ( strlen(str) != m_s.length() ) return true;
		
		for ( STRINGLEN i = 0; i < m_s.length(); i++ )
			if ( LOWER(m_s[i]) != LOWER(*(str + i)) )
				return true;
		return false; 
	}
	bool operator== ( String str ) 
	{ 
		if ( str.Length() != m_s.length() ) return false;
		for ( STRINGLEN i = 0; i < str.Length(); i++ )
			if ( LOWER(m_s[i]) != LOWER(str[i]) )
				return false;
		return true; 
	}
	bool operator== ( const std::string &str ) 
	{ 
		if ( str.length() != m_s.length() ) return false;
		for ( STRINGLEN i = 0; i < str.length(); i++ )
			if ( LOWER(m_s[i]) != LOWER(str[i]) )
				return false;
		return true; 
	}
	bool operator== ( const char *str ) 
	{ 
		if ( strlen(str) != m_s.length() ) return false;
		
		for ( STRINGLEN i = 0; i < m_s.length(); i++ )
			if ( LOWER(m_s[i]) != LOWER(*(str + i)) )
				return false;
		return true; 
	}

	bool Compare ( String &str )
	{
		if ( str.Length() != m_s.length() ) return false;
		for ( STRINGLEN i = 0; i < str.Length(); i++ )
			if ( m_s[i] != str[i] )
				return false;
		return true; 
	}
	bool Compare ( const char *str ) 
	{ 
		if ( strlen(str) != m_s.length() ) return false;
		
		for ( STRINGLEN i = 0; i < m_s.length(); i++ )
			if ( m_s[i] != *(str + i) )
				return false;
		return true; 
	}

	const char *ToChar  () { return m_s.c_str(); }
	const int   ToInt   () { return atoi(m_s.c_str()); }
	const bool  ToBool  () { return (atoi(m_s.c_str())) ? true : false; }
	const long  ToLong  () { return atoi(m_s.c_str()); }
	const float ToFloat () { return (float)atof(m_s.c_str()); }
	const double ToDouble () { return atof(m_s.c_str()); }
	std::string &ToString () { return m_s; }
#ifdef QT_DLL
	QString   ToQString() { return QString(m_s.c_str()); }
#endif

	const char *c_str   () { return m_s.c_str(); }

	bool CharIsNum ( int c )
	{
		if ( (m_s[c] >= '0') && (m_s[c] <= '9') )
			return true;
		return false;
	}

	static String Number ( const long num ) { char c[SPRINTBUFFER]; SPRINTF(c,"%ld"), num ); return std::string(c); }
	String &FromChar   ( const char  *str ) { m_s = std::string(str); return (*this); }
	String &FromInt    ( const long   str ) { char c[SPRINTBUFFER]; SPRINTF(c,"%ld"), str ); m_s = std::string(c); return (*this); }
	String &FromFloat  ( const float  str, int dp = -1 ) 
	{ 
		char c[SPRINTBUFFER]; 
		if ( dp != -1 )
		{
			switch ( dp )
			{
				case 0:
					SPRINTF(c,"%.0f"), str ); 
					break;
				case 1:
					SPRINTF(c,"%.1f"), str ); 
					break;
				case 2:
					SPRINTF(c,"%.2f"), str ); 
					break;
				case 3:
					SPRINTF(c,"%.3f"), str ); 
					break;
				case 4:
					SPRINTF(c,"%.4f"), str ); 
					break;
				case 5:
					SPRINTF(c,"%.5f"), str ); 
					break;
				case 6:
					SPRINTF(c,"%.6f"), str ); 
					break;

				default:
					SPRINTF(c,"%.7f"), str ); 
			}
		}
		else
			SPRINTF(c,"%f"), str ); 

		m_s = std::string(c); 
		return (*this); 
	}

	const size_t Length() { return m_s.length(); }
	const bool Empty () { return m_s.empty();  }

	String &ToLower () 
	{
		for ( STRINGLEN i = 0; i < m_s.length(); i++ )
			m_s[i] = LOWER(m_s[i]);
		return (*this);
	}
	String lower () 
	{
		std::string s = m_s;
		for ( STRINGLEN i = 0; i < s.length(); i++ )
			s[i] = LOWER(s[i]);
		return String(s);
	}
	String upper () 
	{
		std::string s = m_s;
		for ( STRINGLEN i = 0; i < s.length(); i++ )
			s[i] = UPPER(s[i]);
		return String(s);
	}
	String &ToLower ( const char *str )
	{
		m_s = "";
		char *c = (char *)str;
		while ( *c != '\0' )
		{
			m_s += LOWER(*c);
			++c;
		}
		return (*this);
	}

	String &ToUpper () 
	{
		for ( STRINGLEN i = 0; i < m_s.length(); i++ )
			m_s[i] = UPPER(m_s[i]);
		return (*this);
	}
	String &ToUpper ( const char *str )
	{
		m_s = "";
		char *c = (char *)str;
		while ( *c != '\0' )
		{
			m_s += UPPER(*c);
			++c;
		}
		return (*this);
	}


	int NumToken ( char *token )
	{
		// finds the number of tokens in a string
		int found = 0;

		std::string tmpstr = m_s;

		// ignore the initial spaces
		std::string::size_type pos = m_s.find_first_not_of(token, 0);

		// remove end spaces
		size_t i = tmpstr.size() - 1;
		while ( tmpstr[i] == ' ' )
			i--;
		tmpstr.erase ( i + 1, tmpstr.size() - i );

		// count tokens
		while ( pos < std::string::npos )
		{
			pos = tmpstr.find (token,pos + 1);
			while (tmpstr[pos] == ' ')
				pos++;
			found++;
		}

		// return number of tokens
		return found;
	}
	int NumToken ( char token )
	{
		// finds the number of tokens in a string
		int found = 0;

		std::string tmpstr = m_s;

		// ignore the initial spaces
		std::string::size_type pos = m_s.find_first_not_of(token, 0);

		// remove end spaces
		size_t i = tmpstr.size() - 1;
		while ( tmpstr[i] == ' ' )
			i--;
		tmpstr.erase ( i + 1, tmpstr.size() - i );

		// count tokens
		while ( pos < std::string::npos )
		{
			pos = tmpstr.find (token,pos + 1);
//			while (tmpstr[pos] == ' ')
//				pos++;
			found++;
		}

		// return number of tokens
		return found;
	}

	std::string::size_type GetNextPos ( char *token, std::string::size_type curPos )
	{
		bool found = false;
		std::string::size_type startPos = 0;
		size_t max = strlen(token);
		while ( !found )
		{
			found = true;
			startPos = m_s.find_first_of ( token[0], curPos);
			if ( startPos == std::string::npos )
			{
				startPos = m_s.length();
				break;
			}

			std::string::size_type pos = startPos;
			size_t i = 1;
			while ( i < max )
			{
				++pos;
				if ( m_s[pos] != token[i] )
				{
					found = false;
					break;
				}
				++i;
			}
			curPos = pos;
		}
		return startPos;
	}

	String GetToken ( char *token, int from, int to = -1 )
	{
		std::string newstr;

		int whichtok = 1;
		std::string::size_type lastPos = 0;
		std::string::size_type pos     = GetNextPos ( token, 0 );

		while (std::string::npos != pos || std::string::npos != lastPos)
		{
			if ( to == -1 )
			{
				if ( whichtok >= from )
				{
					if ( newstr != "" ) newstr = newstr + token;
					newstr = newstr + m_s.substr(lastPos, pos - lastPos);
				}
			}
			else
			{
				if ( (whichtok >= from) && (whichtok <= to) )
				{
					if ( newstr != "" ) newstr = newstr + token;
					newstr = newstr + m_s.substr(lastPos, pos - lastPos);
				}
				if ( whichtok > to )
					break;
			}
			// Found a token, add it to the vector.
			whichtok++;

			if ( pos >= m_s.length() )
				break;

			// skip past token
			size_t i = 0;
			size_t max = strlen(token);
			lastPos = pos;
			while ( (i < max) && (token[i] == m_s[lastPos]) )
			{
				++i;
				++lastPos;
			}

			// get the next token
			pos = GetNextPos ( token, lastPos );
		}
		return String(newstr);
	}

	String GetToken ( int num, char token )
	{
		std::string newstr;

		int whichtok = 1;
		// get first token
		std::string::size_type lastPos = m_s.find_first_not_of(token, 0);
		std::string::size_type pos     = m_s.find_first_of(token, lastPos);


		while (std::string::npos != pos || std::string::npos != lastPos)
		{
			if ( whichtok == num )
			{
				newstr = m_s.substr(lastPos, pos - lastPos);
				break;
			}
			// Found a token, add it to the vector.
			whichtok++;
			// Skip delimiters.  Note the "not_of"
			lastPos = m_s.find_first_not_of(token, pos);
			// Find next "non-delimiter"
			pos = m_s.find_first_of(token, lastPos);

		}
		return String(newstr);
	}
	String GetToken ( int from, int to, char token )
	{
		std::string newstr;

		int whichtok = 1;
		// get first token
		std::string::size_type lastPos = m_s.find_first_not_of(token, 0);
		std::string::size_type pos     = m_s.find_first_of(token, lastPos);


		while (std::string::npos != pos || std::string::npos != lastPos)
		{
			if ( to == -1 )
			{
				if ( whichtok >= from )
				{
					if ( newstr != "" ) newstr = newstr + token;
					newstr = newstr + m_s.substr(lastPos, pos - lastPos);
				}
			}
			else
			{
				if ( (whichtok >= from) && (whichtok <= to) )
				{
					if ( newstr != "" ) newstr = newstr + token;
					newstr = newstr + m_s.substr(lastPos, pos - lastPos);
				}
				if ( whichtok > to )
					break;
			}
			// Found a token, add it to the vector.
			whichtok++;
			// Skip delimiters.  Note the "not_of"
			lastPos = m_s.find_first_not_of(token, pos);
			// Find next "non-delimiter"
			pos = m_s.find_first_of(token, lastPos);

		}
		return String(newstr);
	}

	int FindToken ( String &str, char token )
	{
		// get first token
		std::string::size_type lastPos = m_s.find_first_not_of(token, 0);
		std::string::size_type pos     = m_s.find_first_of(token, lastPos);

		int tok = 1;
		while (std::string::npos != pos || std::string::npos != lastPos)
		{
			std::string newstr = m_s.substr(lastPos, pos - lastPos);
			if ( str == newstr )
				return tok;
			++tok;
		}
		return 0;
	}

	String DelToken ( int num, char token )
	{
		std::string newstr = "";

		int whichtok = 1;
		// get first token
		std::string::size_type lastPos = m_s.find_first_not_of(token, 0);
		std::string::size_type pos     = m_s.find_first_of(token, lastPos);

		while (std::string::npos != pos || std::string::npos != lastPos)
		{
			if ( whichtok != num )
			{
				if ( newstr == "" )
					newstr = m_s.substr(lastPos, pos - lastPos);
				else
					newstr += token + m_s.substr(lastPos, pos - lastPos);
			}

			// Found a token, add it to the vector.
			whichtok++;
			// Skip delimiters.  Note the "not_of"
			lastPos = m_s.find_first_not_of(token, pos);
			// Find next "non-delimiter"
			pos = m_s.find_first_of(token, lastPos);

		}
		return String(newstr);
	}
	String DelToken ( String &find, char token, bool single = true )
	{
		std::string newstr = "";

		// get first token
		std::string::size_type lastPos = m_s.find_first_not_of(token, 0);
		std::string::size_type pos     = m_s.find_first_of(token, lastPos);

		while (std::string::npos != pos || std::string::npos != lastPos)
		{
			std::string s = m_s.substr(lastPos, pos - lastPos);
			if ( find != s.c_str() )
			{
				if ( newstr == "" )
					newstr = s;
				else
					newstr += token + s;
			}
			else if ( single )
			{
				lastPos = m_s.find_first_not_of(token, pos);
				if ( newstr == "" )
					newstr = m_s.substr(lastPos, m_s.length() - lastPos);
				else
					newstr += token + m_s.substr(lastPos, m_s.length() - lastPos);
				return String(newstr);
			}

			// Skip delimiters.  Note the "not_of"
			lastPos = m_s.find_first_not_of(token, pos);
			// Find next "non-delimiter"
			pos = m_s.find_first_of(token, lastPos);

		}
		return String(newstr);
	}

	String &Addtoken ( char token, String &t )
	{
		if ( !m_s.empty() )
			m_s += token;
		m_s += t.ToString();

		return (*this);
	}
	String &AddToken ( char token, String &t )
	{
		if ( !m_s.empty() )
			m_s += token;
		m_s += t.ToString();

		return (*this);
	}
	String &AddToken ( char token, char *t )
	{
		if ( !m_s.empty() )
			m_s += token;
		m_s += t;

		return (*this);
	}
	/*
	bool IsToken (const std::string &str, const std::string &tokens, char token )
	{
		int jmax = NumToken ( tokens, token );

		// check against each token
		for ( int j = 1; j <= jmax; j++ )
		{
			if ( str == GetToken ( tokens, j, token ) )
				return true;
		}
		return false;
	}*/

	int FindPos ( String str, int start = 0 )
	{
		STRINGLEN pos = 0;
		for ( STRINGLEN i = start; i < m_s.length(); i++ )
		{
			if ( m_s[i] == str[pos] )
			{
				++pos;
				if ( pos >= str.Length() )
					return i - (str.Length() - 1);
			}
			else
				pos = 0;
		}

		return -1;
	}

	bool IsAnyIn ( String str ) { return IsAnyIn ( str.c_str() ); }
	bool IsAnyIn ( char *str )
	{
		char *c = str;
		while ( *c != 0 )
		{
			if ( IsIn ( *c ) )
				return true;
			c++;
		}
		return false;
	}

	bool IsIn ( const char *str ) { return IsIn ( String(str) ); }
	bool IsIn ( String str )
	{
		STRINGLEN check = 0;
		for ( STRINGLEN i = 0; i < m_s.length(); i++ )
		{
			if ( LOWER(m_s[i]) == LOWER(str[check]) )
				++check;
			else
				check = 0;

			if ( check >= str.Length() )
				return true;
		}
		return false;
	}
	bool IsIn ( char c )
	{
		bool f = false;
		for ( STRINGLEN i = 0; i < m_s.length(); i++ )
		{
			if ( m_s[i] == c )
			{
				f = true;
				break;
			}
		}
		return f;
	}
	bool IsInExact ( const char *str ) { return IsInExact ( String(str) ); }
	bool IsInExact ( String str )
	{
		STRINGLEN check = 0;
		for ( STRINGLEN i = 0; i < m_s.length(); i++ )
		{
			if ( m_s[i] == str[check] )
				++check;
			else
				check = 0;

			if ( check >= str.Length() )
				return true;
		}
		return false;
	}

	String *SplitToken ( char token, int *max )
	{
		*max = NumToken ( token );

		String *newstr = new String[*max];

		int whichtok = 0;
		// get first token
		std::string::size_type lastPos = m_s.find_first_not_of(token, 0);
		std::string::size_type pos     = m_s.find_first_of(token, lastPos);


		while (std::string::npos != pos || std::string::npos != lastPos)
		{
			// set token
			newstr[whichtok] = m_s.substr(lastPos, pos - lastPos);

			// move to next token
			whichtok++;
			if ( whichtok >= *max )
				break;

			// Skip delimiters.  Note the "not_of"
			lastPos = m_s.find_first_not_of(token, pos);
			// Find next "non-delimiter"
			pos = m_s.find_first_of(token, lastPos);
		}

		return newstr;
	}

	String Remove ( char c )
	{
		std::string newstr;
		for ( STRINGLEN i = 0; i < m_s.length(); i++ )
		{
			if ( m_s[i] != c )
				newstr += m_s[i];
		}
		return newstr;
	}

	void RemoveChar ( char c )
	{
		std::string old = m_s;
		m_s = "";
		for ( STRINGLEN i = 0; i < old.length(); i++ )
		{
			if ( old[i] != c )
				m_s += old[i];
		}
	}

	void RemoveChar ( char *c )
	{
		std::string old = m_s;
		m_s = "";
		for ( STRINGLEN i = 0; i < old.length(); i++ )
		{
			bool found = false;
			char *nc = &c[0];
			while ( *nc != '\0' )
			{
				if ( old[i] == *nc )
				{
					found = true;
					break;
				}
				++nc;
			}
			if ( !found )
				m_s += old[i];
		}
	}

	String Left ( long num )
	{
		if ( num < 0 )
			num = m_s.length() + num;
		if ( num > (long)m_s.length() )
			num = m_s.length();

		char *str = new char[num + 1];
		STRINGLEN i;
		for ( i = 0; i < (STRINGLEN)num; i++ )
			str[i] = m_s[i];
		str[i] = '\0';
		return String(str);
	}

	void Truncate ( int num )
	{
		if ( num < 0 )
			m_s[m_s.length() + num] = '\0';
		else
			m_s[num] = '\0';
	}
	String Right ( size_t num )
	{
		if ( num < 0 )
			num = m_s.length() + num;
		if ( num > m_s.length() )
			num = m_s.length();
		size_t start = m_s.length() - num;
		STRINGLEN i;
		char *str = new char[num + 1];
		for ( i = (STRINGLEN)start; i < m_s.length(); i++ )
			str[i - start] = m_s[i];
		str[i - start] = '\0';
		return String(str);
	}
	String Mid ( int start, int len )
	{
		STRINGLEN end = start + len;
		if ( end >= m_s.length() )
		{
			end = (STRINGLEN)m_s.length();
			len = end - start;
		}

		char *str = new char[len + 1];
		STRINGLEN i;
		for ( i = start; i < end; i++ )
			str[i - start] = m_s[i - 1];
		str[i - start] = '\0';
		return String(str);
	}

	String &RemoveFirstSpace ()
	{
		size_t i = 0;
		while ( m_s[i] == ' ' )
			i++;
		m_s.erase ( 0, i );
		return (*this);
	}

	String &RemoveEndSpace ()
	{
		size_t i = m_s.size() - 1;
		while ( m_s[i] == ' ' )
			i--;
		m_s.erase ( i + 1, m_s.size() - i );
		return (*this);
	}
	String &RemoveSpaces ()
	{
		while ( m_s[m_s.length() - 1] == ' ' )
			m_s.erase ( m_s.length() - 1, 1 );
		while ( m_s[0] == ' ' )
			m_s.erase ( 0, 1 );
		return (*this);
	}

	String &Erase ( int start, int num ) { m_s.erase ( start, num ); return (*this); }

	String FindReplace ( const char *find, const char *replace ) { return FindReplace ( String(find), String(replace) ); }
	String FindReplace ( String find, const char *replace ) { return FindReplace ( find, String(replace) ); }
	String FindReplace ( const char *find, String replace ) { return FindReplace ( String(find), replace ); }
	String FindReplace ( String find, String replace )
	{
		std::string::size_type pos = m_s.find ( find.ToString(), 0 );
		while ( pos < std::string::npos )
		{
			m_s.replace ( pos, find.Length(), replace.ToString() );
			pos = m_s.find ( find.ToString(), pos );
		}

		return m_s;
	}

	String GetEndOfLine ( FILE *id, int *line, bool upper )
	{
		m_s = "";
		char c = fgetc ( id );
		if ( c == -1 )
			return "";

		while ( (c != 13) && (!feof(id)) && (c != '\n') )
		{
			m_s += c;
			c = fgetc ( id );
		}

		if ( line )
			++(*line);

		if ( upper )
			ToUpper ();

		return (*this);
	}

	int ConvertEquationSection ( String str )
	{
		std::string s = str.ToString();
		char *c = (char *)s.c_str();

		int curNum = 0;
		char *num = new char[s.length() + 1];
		char sign = 0;

		int numpos = 0;
		while ( true )
		{
			if ( (*c >= '0') && (*c <= '9') )
			{
				num[numpos] = *c;
				++numpos;
			}
			else
			{
				num[numpos] = 0;
				numpos = 0;
				int n = atoi(num);

				switch ( sign )
				{
					case 0:
						curNum = n;
						break;
					case '-':
						curNum = curNum - n;
						break;
					case '+':
						curNum = curNum + n;
						break;
					case '*':
						curNum = curNum * n;
						break;
					case '/':
						curNum = curNum / n;
						break;
					case '%':
						curNum = curNum % n;
						break;
					case '^':
						{
							int c = curNum;
							for ( int i = 0; i < n; i++ )
								curNum *= c;
						}
						break;
				}
			
				sign = *c;
			}
			if ( *c == 0 )
				break;
			c++;
		}
		delete num;

		return curNum;
	}
	int ConvertEquationSection () { return ConvertEquationSection ( m_s ); }
	int ConvertEquation ()
	{
		String n = m_s;
		n.RemoveChar ( ' ' );
		size_t endpos = n.GetNextPos ( ")", 0 ), pos;	
		while ( endpos < n.Length() )
		{
			pos = endpos;

			char *find = (char *)n.c_str();
			while ( pos )
			{
				--pos;
				char c = find[pos];
				if ( c == '(' )
					break;
			}
			std::string sub = n.ToString().substr ( pos + 1, endpos - (pos + 1) );
			String sNum = String::Number(String(sub).ConvertEquationSection());
			if ( (pos > 0) && (find[pos - 1] >= '0') && (find[pos - 1] <= '9') )
				sNum = String("*") + sNum;
			n = n.FindReplace ( n.ToString().substr ( pos, (endpos - pos) + 1 ), sNum );
			endpos = n.GetNextPos ( ")", 0 );
		}

		return ConvertEquationSection ( n );
	}

private:
	std::string m_s;
};

#endif //__UTILS_STRING_H__
