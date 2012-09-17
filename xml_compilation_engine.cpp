#include <sstream>
#include <vector>
#include <utility>
#include "compilation_engine.h"

using namespace std;

#define TAB 2


/** Utils functions */

void XMLCompilationEngine::Inc_tab()
{
	m_indent.append( TAB, ' ');
}

void XMLCompilationEngine::Dec_tab()
{
	m_indent.erase( m_indent.size() - TAB );
}


/**********************************************
 * CONSTRUCTOR
 * 
 **********************************************/
XMLCompilationEngine::XMLCompilationEngine(JackTokenizer &jtok, boost::filesystem::path p)
	:CompilationEngine(jtok)
{
	// get the filename
	m_fileName = p.filename().stem().string();
	
	// initialize the ofstream
	p.replace_extension( ".xml" );
	m_out.open( p.c_str() );
	

	// initialize internal vars
	m_op.push_back( '+' );
	m_op.push_back( '-' );
	m_op.push_back( '*' );
	m_op.push_back( '/' );
	m_op.push_back( '&' );
	m_op.push_back( '|' );
	m_op.push_back( '<' );
	m_op.push_back( '>' );
	m_op.push_back( '=' );

	m_unaryOp.push_back( '-' );
	m_unaryOp.push_back( '~' );

	m_kwConstant.push_back( KW_TRUE );
	m_kwConstant.push_back( KW_FALSE );
	m_kwConstant.push_back( KW_NULL );
	m_kwConstant.push_back( KW_THIS ); 

	m_classVarDec.push_back(KW_STATIC);
	m_classVarDec.push_back(KW_FIELD);

	m_type.push_back(KW_INT);
	m_type.push_back(KW_CHAR);
	m_type.push_back(KW_BOOLEAN);

	m_subroutineDec.push_back(KW_CONSTRUCTOR);
	m_subroutineDec.push_back(KW_FUNCTION);
	m_subroutineDec.push_back(KW_METHOD);

	m_statement.push_back(KW_LET);
	m_statement.push_back(KW_IF);
	m_statement.push_back(KW_WHILE);
	m_statement.push_back(KW_DO);
	m_statement.push_back(KW_RETURN);

	// compile class
	XMLCompilationEngine::compileClass();

	m_out.close();
}

/******************************************
 * 
 * output functions
 * Check type correctness
 *
 ******************************************/


/* output identifier */
void XMLCompilationEngine::outputIdentifier(string type)
{
	if ( m_jtok.tokenType() == TOK_IDENTIFIER)
	{
		m_out << m_indent << "<identifier> " << m_jtok.identifier() << " </identifier>" << endl;
	}
	else if ( m_jtok.tokenType() != TOK_IDENTIFIER)
	{
		throw WrongToken( TOK_IDENTIFIER, m_jtok.tokenType(), m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn() );
	}
	else throw IdentifierNotFound( type, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn() );
}

void XMLCompilationEngine::outputClassName()
{
	outputIdentifier( "className" );
}

void XMLCompilationEngine::outputVarName()
{
	outputIdentifier( "varName" );
}

void XMLCompilationEngine::outputSubroutineName()
{
	outputIdentifier( "subRoutineName" );
}

/* output keyword */

void XMLCompilationEngine::outputKeyword( TYPE_KEYWORD tk )
{
	if ( m_jtok.tokenType() == TOK_KEYWORD)
	{
		if ( m_jtok.keyword() == tk )
		{
			m_out << m_indent << "<keyword> " << keyword_to_string( tk ) << " </keyword>" << endl;
		}
		else 
		{
			throw KeywordNotFound( tk, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
		}
	}
	else if ( m_jtok.tokenType() != TOK_KEYWORD)
	{
		throw WrongToken( TOK_KEYWORD, m_jtok.tokenType(), m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
	}
}

/* vector version */

void XMLCompilationEngine::outputKeyword( vector<TYPE_KEYWORD> tk )
{
	if ( m_jtok.tokenType() == TOK_KEYWORD )
	{
		bool found = false;
		TYPE_KEYWORD key = m_jtok.keyword();

		for ( vector<TYPE_KEYWORD>::iterator it = tk.begin(), it_end = tk.end();
			  it != it_end;
			  ++it)
		{
			if ( key == *it)
			{
				m_out << m_indent << "<keyword> " << keyword_to_string( key ) << " </keyword>" << endl;
				found = true;
				break;
			}
		}

		if ( !found )
		{
			throw KeywordNotFound( tk, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
		}
	}
	else if ( m_jtok.tokenType() != TOK_KEYWORD)
	{
		throw WrongToken( TOK_KEYWORD, m_jtok.tokenType(), m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
	}
}

/* output symbol */

void XMLCompilationEngine::outputSymbol( char symbol )
{
	TYPE_TOKEN type = m_jtok.tokenType();

	if ( m_jtok.tokenType() == TOK_SYMBOL && m_jtok.symbol() == symbol)
	{
		// be careful to output <,> and &
		// for XML markup.
		string str_sym;
		if (symbol == '<') str_sym = "&lt;";
		else if (symbol == '>') str_sym = "&gt;";
		else if (symbol == '&') str_sym = "&amp;";
		else str_sym = symbol;
		
		m_out << m_indent << "<symbol> " << str_sym << " </symbol>" << endl;
	}
	else if ( m_jtok.tokenType() != TOK_SYMBOL)
	{
		throw WrongToken( TOK_SYMBOL, m_jtok.tokenType(), m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
	}
	else throw SymbolNotFound( symbol, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
}

/* vector version */

void XMLCompilationEngine::outputSymbol( vector<char> symbols )
{
	if ( m_jtok.tokenType() == TOK_SYMBOL )
	{
		bool found = false;
		char sym = m_jtok.symbol();
		
		for ( vector<char>::iterator it = symbols.begin(), it_end = symbols.end();
			  it != it_end;
			  ++it)
		{
			if ( *it == sym)
			{
				// be careful to output <,> and &
				// for XML markup.
				string str_sym;
				if (sym == '<') str_sym = "&lt;";
				else if (sym == '>') str_sym = "&gt;";
				else if (sym == '&') str_sym = "&amp;";
				else str_sym = sym;
		
				m_out << m_indent << "<symbol> " << str_sym << " </symbol>" << endl;
				found = true;
				break;
			}
		}

		if ( !found )
		{
			throw SymbolNotFound( symbols, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
		}
	}
	else if ( m_jtok.tokenType() != TOK_SYMBOL)
	{
		throw WrongToken( TOK_SYMBOL, m_jtok.tokenType(), m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
	}
}

/* output constants */

void XMLCompilationEngine::outputIntegerConstant()
{
	if ( m_jtok.tokenType() == TOK_INT_CONST)
	{
		int val = m_jtok.intVal();

		if ( val < 0 || val > 32767 )
			 throw IntegerOutOfRange(m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());

		m_out << m_indent << "<integerConstant> " << m_jtok.intVal() << " </integerConstant>" << endl;
	}
	else if ( m_jtok.tokenType() != TOK_INT_CONST)
	{
		throw WrongToken( TOK_INT_CONST, m_jtok.tokenType(), m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
	}
}

void XMLCompilationEngine::outputStringConstant()
{
	if ( m_jtok.tokenType() == TOK_STRING_CONST)
	{
		m_out << m_indent << "<stringConstant> " << m_jtok.stringVal() << " </stringConstant>" << endl;
	}
	else if ( m_jtok.tokenType() != TOK_STRING_CONST)
	{
		throw WrongToken( TOK_STRING_CONST, m_jtok.tokenType(), m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
	}
	else throw MissingStringConstant( m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
}

void XMLCompilationEngine::outputKeywordConstant()
{
	outputKeyword( m_kwConstant );
}

void XMLCompilationEngine::outputOp()
{
	outputSymbol( m_op );
}

void XMLCompilationEngine::outputUnaryOp()
{
	outputSymbol( m_unaryOp );
}


void XMLCompilationEngine::outputType()
{
	if (m_jtok.tokenType() == TOK_KEYWORD)
	{
		outputKeyword(m_type);
	}
	else if (m_jtok.tokenType() == TOK_IDENTIFIER)
	{
		outputClassName();
	}
	else throw WrongType( m_jtok.tokenType(), m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
}

void XMLCompilationEngine:: outputSubroutineCall()
{
	pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();

	if (tok_val.first == TOK_SYMBOL)
	{
		string val = tok_val.second;
		// direct call
		if (val == "(")
		{
			// check subroutineName
			outputSubroutineName();
			
			// check '('
			m_jtok.advance();
			outputSymbol('(');

			// check expressionList
			compileExpressionList();

			// check ')'
			m_jtok.advance();
			outputSymbol(')');
		}
		// call from class
		else if (val == ".")
		{
			// TODO : we should check wether it's a classname or a varname
			outputIdentifier(  m_jtok.identifier() );

			// check '.'
			m_jtok.advance();
			outputSymbol('.');

			// check subRoutineName
			m_jtok.advance();
			outputSubroutineName();

			// check '('
			m_jtok.advance();
			outputSymbol('(');

			// check expressionList
			compileExpressionList();

			// check ')'
			m_jtok.advance();
			outputSymbol(')');
		}
	}
	else throw WrongToken( TOK_SYMBOL, tok_val.first, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
}


/******************************
 * 
 * Compile functions
 * Check the grammar structure
 *
 ******************************/


void XMLCompilationEngine::compileClass()
{
	m_out << "<class>" << endl;
	
	Inc_tab();

	try
	{
		// check 'class'
		m_jtok.advance();
		outputKeyword(KW_CLASS);

		// check classname
		m_jtok.advance();
		outputClassName();

		// check '{'
		m_jtok.advance();
		outputSymbol('{');

		for (;;)
		{
			pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();

			string val = tok_val.second;
			if ( val == "static" || val == "field" )
			{
				// check classvardec (0..*)
				compileClassVarDec();
			}
			else if ( val == "constructor" || val == "function" || val == "method" )
			{
				// check subroutineDec (0..*)
				compileSubroutine();
			}
			else
			{
				break;
			}
		}

		// check '}'
		m_jtok.advance();
		outputSymbol('}');
	}
	catch (const exception& e)
	{
		cerr << e.what() << endl;
	}

	m_out << "</class>" << endl;
}

void XMLCompilationEngine::compileClassVarDec()
{
	m_out << m_indent << "<classVarDec>" << endl;

	Inc_tab();

	// check ('static' | 'field')
	m_jtok.advance();
	outputKeyword( m_classVarDec );

	// check type
	m_jtok.advance();
	outputType();

	// check varName
	m_jtok.advance();
	outputVarName();

	for (;;)
	{
		pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();
		
		if ( tok_val.first == TOK_SYMBOL )
		{
			string val = tok_val.second;

			// if ',' check other varnames
			if (val == ",")
			{
				// check ','
				m_jtok.advance();
				outputSymbol( ',' );

				// check varname
				m_jtok.advance();
				outputVarName();
			}
			else if (val == ";")
			{
				// check ';'
				m_jtok.advance();
				outputSymbol( ';' );

				break;
			}
		}
		else throw WrongToken( TOK_SYMBOL, tok_val.first, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
	}

	Dec_tab();

	m_out << m_indent << "</classVarDec>" << endl;
}

void XMLCompilationEngine::compileSubroutine()
{
	m_out << m_indent << "<subroutineDec>" << endl;
	
	Inc_tab();
	
	// check ('constructor' | 'function' | 'method')
	m_jtok.advance();
	outputKeyword( m_subroutineDec );

	// check (void | type)
	pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();

	if ( tok_val.second == "void" )
	{
		m_jtok.advance();
		outputKeyword( KW_VOID );
	}
	else
	{
		m_jtok.advance();
		outputType();
	}

	// check subroutineName
	m_jtok.advance();
	outputSubroutineName();

	// check '('
	m_jtok.advance();
	outputSymbol( '(' );

	// check parameterlist
	// the list could be empty
	compileParameterList();

	// check ')'
	m_jtok.advance();
	outputSymbol( ')' );

	// check subroutineBody
	m_out << m_indent << "<subroutineBody>" << endl;

	Inc_tab();

	// check '{'
	m_jtok.advance();
	outputSymbol( '{' );

	for (;;)
	{
		pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();
		
		if ( tok_val.first == TOK_KEYWORD )
		{
			string val = tok_val.second;

			// check varDec*
			if (val == "var")
			{
				compileVarDec();
			}
			else
			{
				compileStatements();
				break;
			}
		}
		else throw WrongToken( TOK_KEYWORD, tok_val.first, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
	}

	// check '}'
	m_jtok.advance();
	outputSymbol( '}' );

	Dec_tab();

	m_out << m_indent << "</subroutineBody>" << endl;

	Dec_tab();

	m_out << m_indent << "</subroutineDec>" << endl;
}

void XMLCompilationEngine::compileParameterList()
{
	m_out << m_indent << "<parameterList>" << endl;
	
	Inc_tab();
	
	// check if this is an empty list
	pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();

	if ( tok_val.first == TOK_SYMBOL)
	{
		Dec_tab();
		m_out << m_indent << "</parameterList>" << endl;
		return;
	}

	// check type
	m_jtok.advance();
	outputType();

	// check varName
	m_jtok.advance();
	outputVarName();

	for (;;)
	{
		pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();
		
		if ( tok_val.first == TOK_SYMBOL )
		{
			string val = tok_val.second;

			// if ',' check other varnames
			if (val == ",")
			{
				// check ','
				m_jtok.advance();
				outputSymbol( ',' );

				// check type
				m_jtok.advance();
				outputType();
				
				// check varname
				m_jtok.advance();
				outputVarName();
			}
			else if (val == ")")
			{
				break;
			}
		}
		else throw WrongToken( TOK_SYMBOL, tok_val.first, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
	}

	Dec_tab();

	m_out << m_indent << "</parameterList>" << endl;
}

void XMLCompilationEngine::compileVarDec()
{
	m_out << m_indent << "<varDec>" << endl;
	
	Inc_tab();

	// check 'var'
	m_jtok.advance();
	outputKeyword( KW_VAR );

	// check type
	m_jtok.advance();
	outputType();

	// check varName
	m_jtok.advance();
	outputVarName();

	// if ',' check other varnames
	for (;;)
	{
		pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();
		
		if ( tok_val.first == TOK_SYMBOL )
		{
			string val = tok_val.second;

			// if ',' check other varnames
			if (val == ",")
			{
				// check ','
				m_jtok.advance();
				outputSymbol( ',' );

				// check varname
				m_jtok.advance();
				outputVarName();
			}
			else if (val == ";")
			{
				// check ';'
				m_jtok.advance();
				outputSymbol( ';' );

				break;
			}
		}
		else throw WrongToken( TOK_SYMBOL, tok_val.first, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
	}

	Dec_tab();

	m_out << m_indent << "</varDec>" << endl;
}

void XMLCompilationEngine::compileStatements()
{
	// check if there's at least one statement
	if ( m_jtok.peek().first != TOK_KEYWORD )
	{
		return;
	}
	
	m_out << m_indent << "<statements>" << endl;
	
	Inc_tab();

	for (;;)
	{
		// check if there's not more statements available
		pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();
		if ( tok_val.first != TOK_KEYWORD )
		{
			break;
		}

		string val = tok_val.second;
		if (val == "let")
		{
			compileLet();
		}
		else if (val == "if")
		{
			compileIf();
		}
		else if (val == "while")
		{
			compileWhile();
		}
		else if (val == "do")
		{
			compileDo();
		}
		else if (val == "return")
		{
			compileReturn();
		}
		else throw KeywordNotFound( m_statement, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn() );
	}

	Dec_tab();

	m_out << m_indent << "</statements>" << endl;
}
void XMLCompilationEngine::compileDo()
{
	m_out << m_indent << "<doStatement>" << endl;
	
	Inc_tab();

	// check 'do'
	m_jtok.advance();
	outputKeyword( KW_DO );

	//  check subroutinecall
	m_jtok.advance();
	outputSubroutineCall();

	// check ';'
	m_jtok.advance();
	outputSymbol(';');

	Dec_tab();

	m_out << m_indent << "</doStatement>" << endl;
}

void XMLCompilationEngine::compileLet()
{
	m_out << m_indent << "<letStatement>" << endl;
	
	Inc_tab();

	// check 'let'
	m_jtok.advance();
	outputKeyword( KW_LET );

	// check varname
	m_jtok.advance();
	outputVarName();

	// varname can be an array, check varname[expr]
	pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();
	
	if (tok_val.first == TOK_SYMBOL)
	{
		string val = tok_val.second;

		if (val == "[")
		{
			m_jtok.advance();
			outputSymbol( '[' );

			// check expression
			compileExpression();

			// check ']'
			m_jtok.advance();
			outputSymbol( ']' );

			// check '='
			m_jtok.advance();
			outputSymbol( '=' );

			// check expression
			compileExpression();

			// check ';'
			m_jtok.advance();
			outputSymbol(';');
		}
		else if (val == "=")
		{
			m_jtok.advance();
			outputSymbol( '=' );

			// check expression
			compileExpression();

			// check ';'
			m_jtok.advance();
			outputSymbol(';');
		}
	}
	else throw WrongToken( TOK_SYMBOL, tok_val.first, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());


	Dec_tab();

	m_out << m_indent << "</letStatement>" << endl;
}
void XMLCompilationEngine::compileWhile()
{
	m_out << m_indent << "<whileStatement>" << endl;
	
	Inc_tab();

	// check 'while'
	m_jtok.advance();
	outputKeyword( KW_WHILE );

	// check '('
	m_jtok.advance();
	outputSymbol('(');

	// check expression
	compileExpression();

	// check ')'
	m_jtok.advance();
	outputSymbol(')');

	// check '{'
	m_jtok.advance();
	outputSymbol('{');

	// check statements
	compileStatements();

	// check '}'
	m_jtok.advance();
	outputSymbol('}');

	Dec_tab();

	m_out << m_indent << "</whileStatement>" << endl;
}
void XMLCompilationEngine::compileReturn()
{
	m_out << m_indent << "<returnStatement>" << endl;
	
	Inc_tab();

	// check 'return'
	m_jtok.advance();
	outputKeyword( KW_RETURN );

	// check if there's one expression
	pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();
	
	if (tok_val.second == ";")
	{
		// check ';'
		m_jtok.advance();
		outputSymbol(';');
	}
	else
	{
		compileExpression();

		// check ';'
		m_jtok.advance();
		outputSymbol(';');
	}

	Dec_tab();

	m_out << m_indent << "</returnStatement>" << endl;
}
void XMLCompilationEngine::compileIf()
{
	m_out << m_indent << "<ifStatement>" << endl;
	
	Inc_tab();

	// check 'if'
	m_jtok.advance();
	outputKeyword( KW_IF );

	// check '('
	m_jtok.advance();
	outputSymbol('(');

	// check expression
	compileExpression();

	// check ')'
	m_jtok.advance();
	outputSymbol(')');

	// check '{'
	m_jtok.advance();
	outputSymbol('{');

	// check statements
	compileStatements();

	// check '}'
	m_jtok.advance();
	outputSymbol('}');

	// we could have 0 or 1 'else'
	pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();

	if (tok_val.second == "else")
	{
		// check 'else'
		m_jtok.advance();
		outputKeyword( KW_ELSE );

		// check '{'
		m_jtok.advance();
		outputSymbol('{');

		// check statements
		compileStatements();

		// check '}'
		m_jtok.advance();
		outputSymbol('}');
	}

	Dec_tab();

	m_out << m_indent << "</ifStatement>" << endl;
}

void XMLCompilationEngine::compileExpression()
{
	m_out << m_indent << "<expression>" << endl;
	
	Inc_tab();
	
	// check term
	compileTerm();

	// check (op term)*
	for (;;)
	{
		pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();

		string val = tok_val.second;
		if ( val == "+" || val == "-" || val == "*" || val == "/" || val == "&" || val == "|" ||
			val == "<" || val == ">" || val == "=" )
		{
			// check op
			m_jtok.advance();
			outputOp();

			// check term
			compileTerm();
		}
		else
		{
			break;
		}	
	}

	Dec_tab();

	m_out << m_indent << "</expression>" << endl;
}

void XMLCompilationEngine::compileTerm()
{
	m_out << m_indent << "<term>" << endl;
	
	Inc_tab();

	m_jtok.advance();

	TYPE_TOKEN tt = m_jtok.tokenType();
	
	// check integer
	if ( tt == TOK_INT_CONST )
	{
		outputIntegerConstant();
	}
	// check string
	else if ( tt == TOK_STRING_CONST )
	{
		outputStringConstant();
	}
	// check keyword constant
	else if ( tt == TOK_KEYWORD )
	{
		outputKeywordConstant();
	}
	else if ( tt == TOK_SYMBOL )
	{
		char sym = m_jtok.symbol();

		// check '(' expr ')'
		if ( sym == '(' )
		{
			outputSymbol( '(' );

			// check expression
			compileExpression();

			// check ')'
			m_jtok.advance();
			outputSymbol( ')' );
		}
		// check unaryOp
		else if ( sym == '-' || sym == '~' )
		{
			outputUnaryOp();

			// check term
			compileTerm();
		}
	}
	else if ( tt == TOK_IDENTIFIER )
	{
		pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();

		// check varname[expr]
		if (tok_val.second == "[")
		{
			outputVarName();

			// check '['
			m_jtok.advance();
			outputSymbol( '[' );

			// check expression
			compileExpression();

			// check ']'
			m_jtok.advance();
			outputSymbol( ']' );
		}
		// check subroutine call
		else if (tok_val.second == "(" || tok_val.second == ".")
		{
			outputSubroutineCall();
		}
		// check varname
		else
		{
			outputVarName();
		}
	}

	Dec_tab();

	m_out << m_indent << "</term>" << endl;
}

void XMLCompilationEngine::compileExpressionList()
{
	m_out << m_indent << "<expressionList>" << endl;
	
	Inc_tab();

	// check if there's at least one expr
	if ( m_jtok.peek().second != ")" )
	{
		// check expression
		compileExpression();

		// check if there's other expr
		for (;;)
		{
			if ( m_jtok.peek().second == "," )
			{
				// check ','
				m_jtok.advance();
				outputSymbol( ',' );

				// check expression
				compileExpression();
			}
			else
			{
				break;
			}
		}
	}

	Dec_tab();

	m_out << m_indent << "</expressionList>" << endl;
}