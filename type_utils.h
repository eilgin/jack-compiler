#ifndef _TYPE_UTILS_H
#define _TYPE_UTILS_H

#include <string>

using std::string;

/* enumeration types */

/* the following enums are used in JackTokenizer */

// because some keywords are using the same name
// than #define values, we can't use them as is (C-preprocessor sucks)

enum TYPE_TOKEN {
	TOK_KEYWORD,
	TOK_SYMBOL,
	TOK_IDENTIFIER,
	TOK_INT_CONST,
	TOK_STRING_CONST,
	TOK_EMPTY
};

enum TYPE_KEYWORD {
	KW_CLASS,
	KW_METHOD,
	KW_FUNCTION,
	KW_CONSTRUCTOR,
	KW_INT,
	KW_BOOLEAN,
	KW_CHAR,
	KW_VOID,
	KW_VAR,
	KW_STATIC,
	KW_FIELD,
	KW_LET,
	KW_DO,
	KW_IF,
	KW_ELSE,
	KW_WHILE,
	KW_RETURN,
	KW_TRUE,
	KW_FALSE,
	KW_NULL,
	KW_THIS,
	KW_UNKNOWN
};

/* the following enums are used in JackCompiler */

enum KIND {
	K_STATIC,
	K_FIELD,
	K_ARG,
	K_VAR,
	K_NONE
};

enum SEGMENT {
	SEG_CONST,
	SEG_ARG,
	SEG_LOCAL,
	SEG_STATIC,
	SEG_THIS,
	SEG_THAT,
	SEG_POINTER,
	SEG_TEMP
};

enum COMMAND {
	C_ADD,
	C_SUB,
	C_NEG,
	C_EQ,
	C_GT,
	C_LT,
	C_AND,
	C_OR,
	C_NOT
};

/* utils functions */

inline string token_to_string(TYPE_TOKEN tok)
{
	switch ( tok )
	{
	case TOK_KEYWORD:
		return "keyword";
	case TOK_SYMBOL:
		return "symbol";
	case TOK_IDENTIFIER:
		return "identifier";
	case TOK_INT_CONST:
		return "integer";
	case TOK_STRING_CONST:
		return "string";
	case TOK_EMPTY:
		return "empty";
	}
}

inline string keyword_to_string(TYPE_KEYWORD tk)
{
	switch ( tk )
	{
	case KW_CLASS:
		return "class";
	case KW_METHOD:
		return "method";
	case KW_FUNCTION:
		return "function";
	case KW_CONSTRUCTOR:
		return "constructor";
	case KW_INT:
		return "int";
	case KW_BOOLEAN:
		return "boolean";
	case KW_CHAR:
		return "char";
	case KW_VOID:
		return "void";
	case KW_VAR:
		return "var";
	case KW_STATIC:
		return "static";
	case KW_FIELD:
		return "field";
	case KW_LET:
		return "let";
	case KW_DO:
		return "do";
	case KW_IF:
		return "if";
	case KW_ELSE:
		return "else";
	case KW_WHILE:
		return "while";
	case KW_RETURN:
		return "return";
	case KW_TRUE:
		return "true";
	case KW_FALSE:
		return "false";
	case KW_NULL:
		return "null";
	case KW_THIS:
		return "this";
	case KW_UNKNOWN:
		return "unknown";
	}
}

inline string kind_to_string(KIND k)
{
	switch ( k )
	{
	case K_STATIC:
		return "static";
	case K_FIELD:
		return "field";
	case K_ARG:
		return "arg";
	case K_VAR:
		return "var";	
	case K_NONE:
		return "none";
	}
}

inline SEGMENT kind_to_segment(KIND k)
{
	switch ( k )
	{
	case K_STATIC:
		return SEG_STATIC;
	case K_FIELD:
		return SEG_THIS;
	case K_ARG:
		return SEG_ARG;
	case K_VAR:
		return SEG_LOCAL;	
	}
}

inline KIND keyword_to_kind(TYPE_KEYWORD tk)
{
	if (tk == KW_STATIC)
	{
		return K_STATIC;
	}
	else if (tk == KW_FIELD)
	{
		return K_FIELD;
	}
	else if (tk == KW_VAR)
	{
		return K_VAR;
	}
	// this is an ugly hack :(
	else
	{
		return K_ARG;
	}
}


inline string segment_to_string(SEGMENT seg)
{
	switch ( seg )
	{
	case SEG_CONST:
		return "constant";
	case SEG_ARG:
		return "argument";
	case SEG_LOCAL:
		return "local";
	case SEG_STATIC:
		return "static";	
	case SEG_THIS:
		return "this";
	case SEG_THAT:
		return "that";
	case SEG_POINTER:
		return "pointer";
	case SEG_TEMP:
		return "temp";
	}
}

inline string command_to_string(COMMAND cmd)
{
	switch ( cmd )
	{
	case C_ADD:
		return "add";
	case C_SUB:
		return "sub";
	case C_NEG:
		return "neg";
	case C_EQ:
		return "eq";	
	case C_GT:
		return "gt";
	case C_LT:
		return "lt";
	case C_AND:
		return "and";
	case C_OR:
		return "or";
	case C_NOT:
		return "not";
	}
}

inline COMMAND char_to_op(char cmd)
{
	switch ( cmd )
	{
	case '+':
		return C_ADD;
	case '-':
		return C_SUB;
	case '=':
		return C_EQ;	
	case '>':
		return C_GT;
	case '<':
		return C_LT;
	case '&':
		return C_AND;
	case '|':
		return C_OR;
	}
}

inline COMMAND char_to_unaryOp(char cmd)
{
	switch ( cmd )
	{
	case '-':
		return C_NEG;
	case '~':
		return C_NOT;
	}
}

struct SubroutineInfo {
public:
	string type;
	string kind;
	int nArgs;

	SubroutineInfo():type(""), kind(""), nArgs(0) {}
	SubroutineInfo(string t, string k, int i):type(t), kind(k), nArgs(i) {}
};

struct SymbolInfo {
public:
	string type;
	KIND kind;
	int index;

	SymbolInfo():type(""), kind(K_NONE), index(0) {}
	SymbolInfo(string t, KIND k, int i):type(t), kind(k), index(i) {}
};

#endif
