#ifndef _COMP_ENGINE_H
#define _COMP_ENGINE_H

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include "jack_tokenizer.h"
#include "vm_writer.h"
#include "symbol_table.h"

using std::string;
using std::ofstream;
using std::vector;
using std::ostringstream;
using boost::filesystem::path;

class CompilationEngine {
public:
	CompilationEngine(JackTokenizer &jtok)
		:m_jtok(jtok) {}
	virtual ~CompilationEngine() {}

	virtual void compileClass() =0;
	virtual void compileClassVarDec() =0;
	virtual void compileSubroutine() =0;
	virtual void compileParameterList() =0;
	virtual void compileVarDec() =0;
	virtual void compileStatements() =0;
	virtual void compileDo() =0;
	virtual void compileLet() =0;
	virtual void compileWhile() =0;
	virtual void compileReturn() =0;
	virtual void compileIf() =0;
	virtual void compileExpression() =0;
	virtual void compileTerm() =0;
	virtual void compileExpressionList() =0;

protected:
	JackTokenizer &m_jtok;
};

class XMLCompilationEngine : public CompilationEngine {
public:
	XMLCompilationEngine(JackTokenizer &jtok, path p);

	virtual void compileClass();
	virtual void compileClassVarDec();
	virtual void compileSubroutine();
	virtual void compileParameterList();
	virtual void compileVarDec();
	virtual void compileStatements();
	virtual void compileDo();
	virtual void compileLet();
	virtual void compileWhile();
	virtual void compileReturn();
	virtual void compileIf();
	virtual void compileExpression();
	virtual void compileTerm();
	virtual void compileExpressionList();
private:
	string m_indent;
	vector<char> m_op, m_unaryOp;
	vector<TYPE_KEYWORD> m_kwConstant, m_classVarDec, m_type, m_subroutineDec, m_statement;

	ofstream m_out;
	string m_fileName;

	// increment/decrement tabulation needed for XML reading comfort
	void Inc_tab();
	void Dec_tab();
	// outputting functions
	void outputIdentifier(string type);
	void outputClassName();
	void outputVarName();
	void outputSubroutineName();
	void outputKeyword( TYPE_KEYWORD tk );
	void outputKeyword( vector<TYPE_KEYWORD> tk );
	void outputSymbol( char symbol );
	void outputSymbol( vector<char> symbols );
	void outputIntegerConstant();
	void outputStringConstant();
	void outputKeywordConstant();
	void outputOp();
	void outputUnaryOp();
	void outputType();
	void outputSubroutineCall();
};

class JackCompilationEngine : public CompilationEngine {
public:
	JackCompilationEngine( JackTokenizer &jtok, path p, map<string,SubroutineInfo> ref_methods = map<string,SubroutineInfo>() );

	virtual void compileClass();
	virtual void compileClassVarDec();
	virtual void compileSubroutine();
	virtual void compileParameterList();
	virtual void compileVarDec();
	virtual void compileStatements();
	virtual void compileDo();
	virtual void compileLet();
	virtual void compileWhile();
	virtual void compileReturn();
	virtual void compileIf();
	virtual void compileExpression();
	virtual void compileTerm();
	virtual void compileExpressionList();
	void compileSubroutineCall();
	void pushIdentifier(KIND kind, int index);
	void popIdentifier(KIND kind, int index);
	// map< method name, number of LocalVar >
	map<string, SubroutineInfo> getMethodList();

private:
	// vmwriter
	VMWriter m_VMOutput;
	// symbol table
	SymbolTable m_symTab;

	// used to format VM functions
	string m_fileName, m_className;

	// used to know each subroutine context (construct, method, ...)
	string m_currentSubroutine_name, m_currentSubroutine_kind, m_currentSubroutine_type;
	/* we need to determine every subroutine (specially methods) of the class BEFORE
	 * outputting VM inst. so we'll make 2 pass.
	 * The 1st one is to fill this member class
	 */
	map<string, SubroutineInfo> m_classSubroutine_params;
	// count how many arguments used for other subroutines called
	int m_externSubroutine_params;

	// 'if' and 'while' counters
	int m_ifCounter, m_whileCounter;

	// keywords and symbols constants
	vector<char> m_op, m_unaryOp;
	vector<TYPE_KEYWORD> m_kwConstant, m_classVarDec, m_type, m_subroutineDec, m_statement;

	// inspecting functions
	void inspectIdentifier();
	void inspectClassName();
	void inspectVarName();
	void inspectSubroutineName();
	void inspectKeyword( TYPE_KEYWORD tk );
	void inspectKeyword( vector<TYPE_KEYWORD> tk );
	void inspectSymbol( char symbol );
	void inspectSymbol( vector<char> symbols );
	void inspectIntegerConstant();
	void inspectStringConstant();
	void inspectKeywordConstant();
	void inspectOp();
	void inspectUnaryOp();
	void inspectType();
};

/** Handled exception */

class JackError : public std::exception {
public:
	JackError( string message, string filename, int line, int column )
	{
		ostringstream oss;
		oss << "Error in \"" << filename << "\" Ln " << line << ", Col " << column << " : ";
		oss << message;
		this->msg = oss.str();
	}

	virtual ~JackError() throw() {}

	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
private:
	string msg;
};

class IdentifierNotFound : public std::exception {
public:
	IdentifierNotFound( string type, string filename, int line, int column )
	{
		ostringstream oss;
		oss << "Error in \"" << filename << "\" Ln " << line << ", Col " << column << " : ";
		oss << "\"" << type << "\" not found";
		this->msg = oss.str();
	}

	virtual ~IdentifierNotFound() throw() {}

	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
private:
	string msg;
};

class KeywordNotFound : public std::exception {
public:
	KeywordNotFound( TYPE_KEYWORD keyword, string filename, int line, int column )
	{
		ostringstream oss;
		oss << "Error in \"" << filename << "\" Ln " << line << ", Col " << column << " : ";
		oss << "\"" << keyword_to_string( keyword ) << "\" not found";
		this->msg = oss.str();
	}

	KeywordNotFound( vector<TYPE_KEYWORD> keywords, string filename, int line, int column )
	{
		ostringstream oss;
		string sep = ", ";

		oss << "Error in \"" << filename << "\" Ln " << line << ", Col " << column << " : ";
		
		for ( vector<TYPE_KEYWORD>::iterator it = keywords.begin(), it_end = keywords.end();
			it != it_end;
			++it)
		{
			oss << "'" << keyword_to_string( *it ) << "'" << sep;
		}
		// remove the last separator
		long pos = oss.tellp();
		oss.seekp( pos - sep.size() );

		oss << " not found";
		this->msg = oss.str();
	}

	virtual ~KeywordNotFound() throw() {}

	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
private:
	string msg;
};

class SymbolNotFound : public std::exception {
public:
	SymbolNotFound( char symbol, string filename, int line, int column )
	{
		ostringstream oss;
		oss << "Error in \"" << filename << "\" Ln " << line << ", Col " << column << " : ";
		oss << "\"" << symbol << "\" not found";
		this->msg = oss.str();
	}

	SymbolNotFound( vector<char> symbols, string filename, int line, int column )
	{
		ostringstream oss;
		string sep = ", ";

		oss << "Error in \"" << filename << "\" Ln " << line << ", Col " << column << " : ";
		
		for ( vector<char>::iterator it = symbols.begin(), it_end = symbols.end();
			it != it_end;
			++it)
		{
			oss << "'" << *it << "'" << sep;
		}
		// remove the last separator
		long pos = oss.tellp();
		oss.seekp( pos - sep.size() );

		oss << " not found";
		this->msg = oss.str();
	}

	virtual ~SymbolNotFound() throw() {}

	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
private:
	string msg;
};

class WrongType : public std::exception {
public:
	WrongType( TYPE_TOKEN tt, string filename, int line, int column )
	{
		ostringstream oss;
		oss << "Error in \"" << filename << "\" Ln " << line << ", Col " << column << " : ";
		oss << "\"" << token_to_string(tt) << "\" is not a valid type";
		this->msg = oss.str();
	}

	virtual ~WrongType() throw() {}

	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
private:
	string msg;
};

class InvalidType : public std::exception {
public:
	InvalidType( string type, string filename, int line, int column )
	{
		ostringstream oss;
		oss << "Error in \"" << filename << "\" Ln " << line << ", Col " << column << " : ";
		oss << "\"" << type << "\" is illegal here";
		this->msg = oss.str();
	}

	virtual ~InvalidType() throw() {}

	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
private:
	string msg;
};

class WrongToken : public std::exception {
public:
	WrongToken( TYPE_TOKEN expected, TYPE_TOKEN found, string filename, int line, int column )
	{
		ostringstream oss;
		oss << "Error in \"" << filename << "\" Ln " << line << ", Col " << column << " : ";
		oss << "Expected a \"" << token_to_string(expected) << "\" token";
		oss << ", found a \"" << token_to_string(found) << "\" one";
		this->msg = oss.str();
	}

	virtual ~WrongToken() throw() {}

	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
private:
	string msg;
};

class MissingStringConstant : public std::exception {
public:
	MissingStringConstant( string filename, int line, int column )
	{
		ostringstream oss;
		oss << "Error in \"" << filename << "\" Ln " << line << ", Col " << column << " : ";
		oss << "Missing string constant";
		this->msg = oss.str();
	}

	virtual ~MissingStringConstant() throw() {}

	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
private:
	string msg;
};

/* May not be actually thrown because some programs
 * exploit the binary representation of -1
 */
class IntegerOutOfRange : public std::exception {
public:
	IntegerOutOfRange( string filename, int line, int column )
	{
		ostringstream oss;
		oss << "Error in \"" << filename << "\" Ln " << line << ", Col " << column << " : ";
		oss << "Integer value is out of range [0..32767]";
		this->msg = oss.str();
	}

	virtual ~IntegerOutOfRange() throw() {}

	virtual const char* what() const throw()
	{
		return this->msg.c_str();
	}
private:
	string msg;
};

#endif
