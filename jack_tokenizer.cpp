#include <sstream>
#include <cctype>
#include <boost/lexical_cast.hpp>
#include "jack_tokenizer.h"

using namespace std;

JackTokenizer::JackTokenizer(string jackcode)
	:m_jackStreamCode(jackcode)
{
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_CLASS, "class") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_CONSTRUCTOR, "constructor") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_FUNCTION, "function") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_METHOD, "method") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_FIELD, "field") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_STATIC, "static") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_VAR, "var") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_INT, "int") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_CHAR, "char") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_BOOLEAN, "boolean") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_VOID, "void") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_TRUE, "true") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_FALSE, "false") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_NULL, "null") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_THIS, "this") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_LET, "let") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_DO, "do") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_IF, "if") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_ELSE, "else") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_WHILE, "while") );
	m_keywords.insert( pair<TYPE_KEYWORD, string>(KW_RETURN, "return") );

	m_symbols.push_back('{'); m_symbols.push_back('}');
	m_symbols.push_back('('); m_symbols.push_back(')');
	m_symbols.push_back('['); m_symbols.push_back(']');
	m_symbols.push_back('.');
	m_symbols.push_back(',');
	m_symbols.push_back(';');
	m_symbols.push_back('+');
	m_symbols.push_back('-');
	m_symbols.push_back('*');
	m_symbols.push_back('/');
	m_symbols.push_back('&');
	m_symbols.push_back('|');
	m_symbols.push_back('<');
	m_symbols.push_back('>');
	m_symbols.push_back('=');
	m_symbols.push_back('~');

	m_curLine = 1;
	m_curCol = 1;
}

int JackTokenizer::getCurrentLine()
{
	return m_curLine;
}

int JackTokenizer::getCurrentColumn()
{
	return m_curCol - m_currentToken.size();
}

bool JackTokenizer::hasMoreTokens()
{
	return m_jackStreamCode.good();
}

void JackTokenizer::advance()
{
	// first, reset the current token to an empty string
	m_currentToken = "";
	
	/* we read the code one character at a time
		storing his value in a string buffer (stringbuf).
		if the char is a separator (space or line feed),
		we skip the function storing the current token as :
		- an empty string if buffer_size = 0;
		- the buffer value.
		if the char is a symbol, it returns that value or
		the concatened buffer.
		if the char is equal to '/' then if it's followed by :
		- another slash, we skipped chars
		until we found a separator char ('\n'); it's a line comment
		- a star, we try to reach the end of that block comment
		and erase it from the stream
		- something else, it's stored as a symbol
	*/
	char c;
	stringbuf buffer;

	while ( hasMoreTokens() )
	{
		// read one char
		c = m_jackStreamCode.get();

		m_curCol ++; // update pointer position

		string s = buffer.str();

		// remove separators
		// i could use isspace() => stdlib
		if (c == ' ' || c == '\t' || c == '\n')
		{
			if ( s.size() > 0 )
			{
				m_currentToken = s;
				m_jackStreamCode.unget();
				m_curCol--;
				return;
			}
			else if (c == '\n')
			{
				m_curLine++; m_curCol = 1; // update pointer position
			}
			continue;
		}
		// remove comments
		if ( c == '/' )
		{
			// read the next char
			char next_c = m_jackStreamCode.get();

			m_curCol++; // update pointer position

			if (next_c == '/')
			{
				// remove 1 line
				char trash;
				while ( hasMoreTokens() )
				{
					trash = m_jackStreamCode.get();

					if (trash == '\n')
					{
						break;
					}
				}
				m_curLine++; m_curCol = 1; // update pointer position

				continue;
			}
			else if (next_c == '*')
			{
				// remove all chars until we found '*/'
				char c1=0;
				char c2=0;
				bool found = false;
				while ( hasMoreTokens() )
				{
					c1 = c2;
					c2 = m_jackStreamCode.get();

					// update pointer position
					if (c2 == '\n')
					{
						m_curLine++; m_curCol = 1;
					}
					else
					{
						m_curCol++;
					}

					if (c1 == '*' && c2 == '/')
					{
						found = true;
						break;
					}
				}
				if (found) continue;
			}
			else
			{
				// put back next_c as the current char
				m_jackStreamCode.unget();

				m_curCol--; // update pointer position
			}
		}

		// there's a special issue with string constant,
		// we need to parse the entire string inside the quotes
		if ( c == '"' )
		{
			char next_c;
			while ( hasMoreTokens() )
			{
				next_c = m_jackStreamCode.get();

				// update pointer position
				if (next_c == '\n')
				{
					cerr << "At line " << getCurrentLine() << ", column " << getCurrentColumn() << " : ";
					cerr << "Escape character is not allowed in a string constant" << endl;
					exit(-1);
				}
				else
				{
					m_curCol++;
				}

				if (next_c == '"')
				{
					break;
				}

				buffer.sputc(next_c);
			}

			m_currentToken = '"' + buffer.str() + '"';

			return;
		}
		
		// check if the character is a symbol
		// i could use strpbrk http://www.cplusplus.com/reference/clibrary/cstring/strpbrk/
		typedef vector<char>::iterator vec_it;
		for (vec_it it = m_symbols.begin(), it_end = m_symbols.end();
			it != it_end;
			++it)
		{
			char symbol = *it;
			if (c == symbol)
			{
				if ( s.size() > 0 )
				{
					m_currentToken = s;
					// get the actual buffer value and
					// return to the previous position in the stream (next time, we'll capture this symbol)
					m_jackStreamCode.unget();

					m_curCol--; // update pointer position

					return;
				}
				else
				{
					m_currentToken = c;
					return;
				}
			}
		}

		// append the current char to the buffer
		buffer.sputc(c);
	}
}

pair<TYPE_TOKEN, string> JackTokenizer::peek()
{
	// save the current token
	string curToken = m_currentToken;
	
	advance();

	int rewind = m_currentToken.size();

	for (int i=0; i < rewind; i++)
	{
		m_jackStreamCode.unget();
		m_curCol--;
	}

	pair<TYPE_TOKEN, string> token_value(tokenType(), m_currentToken);

	// restore the current token as if nothing happened
	m_currentToken = curToken;

	return token_value;
}

TYPE_TOKEN JackTokenizer::tokenType()
{
	if (m_currentToken.size() == 0)
		return TOK_EMPTY;
	
	typedef map<TYPE_KEYWORD, string>::iterator keys_it;
	typedef vector<char>::iterator sym_it;
	
	// check if the current token is:
	// a keyword
	for (keys_it it = m_keywords.begin(), it_end = m_keywords.end();
		it != it_end;
		++it)
	{
		if (m_currentToken == it->second)
			return TOK_KEYWORD;
	}

	// a symbol
	for (sym_it it = m_symbols.begin(), it_end = m_symbols.end();
		it != it_end;
		++it)
	{
		string str;
		char c = *it;
		str = c;
		if (m_currentToken == str)
			return TOK_SYMBOL;
	}

	// an integer constant
	if (isdigit( m_currentToken[0] ))
		return TOK_INT_CONST;

	// a string constant
	if (m_currentToken[0] == '"')
		return TOK_STRING_CONST;

	// it must be an identifier
	return TOK_IDENTIFIER;
}

TYPE_KEYWORD JackTokenizer::keyword()
{
	typedef map<TYPE_KEYWORD, string>::iterator keys_it;
	for (keys_it it = m_keywords.begin(), it_end = m_keywords.end();
		it != it_end;
		++it)
	{
		if (m_currentToken == it->second)
			return it->first;
	}
	throw TokenMismatch( "keyword" );
}

char JackTokenizer::symbol()
{
	if (m_currentToken.size() == 1)
		return m_currentToken[0];
	else throw TokenMismatch( "symbol" );
}

string JackTokenizer::identifier()
{
	if (m_currentToken.size() > 0)
		return m_currentToken;
	else throw TokenMismatch( "identifier" );
}

int JackTokenizer::intVal()
{
	int num = 0;
	try
	{
		num = boost::lexical_cast<int>( m_currentToken );
	}
	catch (const boost::bad_lexical_cast &e)
	{
		cout << e.what() << endl;
	}
	return num;
}

string JackTokenizer::stringVal()
{
	if (m_currentToken.size() > 2 && m_currentToken[0] == '"')
		return m_currentToken.substr(1, m_currentToken.size()-2);
	else throw TokenMismatch( "string" );
}

