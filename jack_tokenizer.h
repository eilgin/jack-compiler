#ifndef _JACK_TOKENIZER_H
#define _JACK_TOKENIZER_H

#include <string>
#include <sstream>
#include <vector>
#include <map>

#include "type_utils.h"

using std::string;
using std::istringstream;
using std::vector;
using std::map;
using std::pair;

class JackTokenizer {
public:
	/**
	Opens the input file and gets ready
	to parse it
	*/
	JackTokenizer(string jackcode);
	/**
	Are there more tokens in the input ?
	*/
	bool hasMoreTokens();
	/**
	Reads the next token from the input
	and makes it the current token.
	Should be called only if hasMoreTokens()
	is true.
	Initially there is no current token
	*/
	void advance();
	/**
	Reads the next token from the input
	WITHOUT making it the current token.
	Should be called only if hasMoreTokens()
	is true.
	*/
	pair<TYPE_TOKEN, string> peek();
	/**
	Returns the type of the current
	token
	*/
	TYPE_TOKEN tokenType();
	/**
	Only call this when token type == KEYWORD
	*/
	TYPE_KEYWORD keyword();
	/**
	Only call this when token type == SYMBOL
	*/
	char symbol();
	/**
	Only call this when token type == IDENTIFIER
	*/
	string identifier();
	/**
	Only call this when token type == INT_CONST
	*/
	int intVal();
	/**
	Only call this when token type == STRING_CONST
	*/
	string stringVal();

	/* helper functions */
	/**
	get current line in the source text
	*/
	int getCurrentLine();
	/**
	get current column in the source text
	*/
	int getCurrentColumn();

private:
	istringstream m_jackStreamCode;
	string m_currentToken;

	// keywords (defined in the constructor)
	map<TYPE_KEYWORD, string> m_keywords;
	vector<char> m_symbols;

	// storing current pointer position
	int m_curLine, m_curCol;
};

class TokenMismatch : public std::exception {
public:
	TokenMismatch( string validToken )
	{
		std::ostringstream oss;
		oss << "The current token is not a valid \"" << validToken << "\"";
		this->msg = oss.str();
	}

	virtual ~TokenMismatch() throw() {}

	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
private:
	string msg;
};

#endif
