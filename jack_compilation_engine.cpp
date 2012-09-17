#include "compilation_engine.h"

using namespace std;

JackCompilationEngine::JackCompilationEngine(JackTokenizer &jtok, boost::filesystem::path p, map<string,SubroutineInfo> ref_methods)
	:CompilationEngine(jtok), m_VMOutput(p), m_externSubroutine_params(0), m_ifCounter(0), m_whileCounter(0)
{
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
	
	
	// get the filename
	m_fileName = p.filename().stem().string();

	// BEFORE we compile this class, we need to retrieve infos about methods of this class
	m_classSubroutine_params = ref_methods;

	// virtuals functions statically resolved in construct/destruct
	JackCompilationEngine::compileClass(); 

	m_VMOutput.close();
}

map<string, SubroutineInfo> JackCompilationEngine::getMethodList()
{
	return m_classSubroutine_params;
}

void JackCompilationEngine::compileClass()
{
	try
	{
		// check 'class'
		m_jtok.advance();
		inspectKeyword(KW_CLASS);

		// check classname
		m_jtok.advance();
		inspectClassName();

		// get the className
		m_className = m_jtok.identifier();

		// check '{'
		m_jtok.advance();
		inspectSymbol('{');

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
		inspectSymbol('}');
	}
	catch (const exception& e)
	{
		cerr << e.what() << endl;
	}
}

void JackCompilationEngine::compileClassVarDec()
{
	// store info for the symbol table
	string idName, idType;
	KIND idKind;

	// check ('static' | 'field')
	m_jtok.advance();
	inspectKeyword( m_classVarDec );

	// get the id. kind
	idKind = keyword_to_kind( m_jtok.keyword() );

	// check type
	m_jtok.advance();
	inspectType();

	// get the type
	if ( m_jtok.tokenType() == TOK_KEYWORD )
	{
		idType = keyword_to_string( m_jtok.keyword() );
	}
	else
	{
		idType = m_jtok.identifier();
	}

	// check varName
	m_jtok.advance();
	inspectVarName();

	// get the name
	idName = m_jtok.identifier();

	// insert a new identifier
	m_symTab.Define(idName, idType, idKind);

	for (;;)
	{
		pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();
		
		if (tok_val.first != TOK_SYMBOL)
		{
			throw WrongToken( TOK_SYMBOL, tok_val.first, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn() );
		}
		
		string val = tok_val.second;

		// if ',' check other varnames
		if (val == ",")
		{
			// check ','
			m_jtok.advance();
			inspectSymbol( ',' );

			// check varname
			m_jtok.advance();
			inspectVarName();

			// get the name
			idName = m_jtok.identifier();

			// insert another identifier
			m_symTab.Define(idName, idType, idKind);
		}
		else if (val == ";")
		{
			// check ';'
			m_jtok.advance();
			inspectSymbol( ';' );

			break;
		}
	}
}

void JackCompilationEngine::compileSubroutine()
{
	// check ('constructor' | 'function' | 'method')
	m_jtok.advance();
	inspectKeyword( m_subroutineDec );

	// we specify to the symTable to reset local ids
	m_symTab.startSubroutine();

	// context will be different based on that value
	m_currentSubroutine_kind = keyword_to_string( m_jtok.keyword() );

	// check (void | type)
	pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();

	// get the type
	string idType;

	if ( tok_val.second == "void" )
	{
		m_jtok.advance();
		inspectKeyword( KW_VOID );
		idType = keyword_to_string( m_jtok.keyword() );
	}
	else
	{
		m_jtok.advance();
		inspectType();

		if ( m_jtok.tokenType() == TOK_KEYWORD )
		{
			idType = keyword_to_string( m_jtok.keyword() );
		}
		else
		{
			idType = m_jtok.identifier();
		}
	}

	// needed to check the return statement correctness
	m_currentSubroutine_type = idType;

	// a construct func must have a type identical to the className
	if (m_currentSubroutine_kind == "constructor")
	{
		if (idType != m_className)
		{
			throw InvalidType(idType, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
		}
	}

	// check subroutineName
	m_jtok.advance();
	inspectSubroutineName();

	// get the subName
	m_currentSubroutine_name = m_jtok.identifier();

	// check '('
	m_jtok.advance();
	inspectSymbol( '(' );

	// check parameterlist
	// the list could be empty
	compileParameterList();

	// check ')'
	m_jtok.advance();
	inspectSymbol( ')' );

	// store infos (TYPE, KIND, number of ARGUMENTS) from this new subroutine 
	SubroutineInfo subInfo(m_currentSubroutine_type, m_currentSubroutine_kind, m_symTab.VarCount(K_ARG));
	m_classSubroutine_params.insert( pair<string, SubroutineInfo>(m_currentSubroutine_name, subInfo) );
	
	// check subroutineBody

	// check '{'
	m_jtok.advance();
	inspectSymbol( '{' );

	for (;;)
	{
		pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();
		
		if ( tok_val.first != TOK_KEYWORD )
		{
			throw WrongToken( TOK_KEYWORD, tok_val.first, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
		}

		string val = tok_val.second;

		// check varDec*
		if (val == "var")
		{
			compileVarDec();
		}
		else
		{
			break;
		}
	}

	// at this point, we can write VM function declaration
	m_VMOutput.writeFunction(string(m_className + "." + m_currentSubroutine_name), m_symTab.VarCount(K_VAR));
	
	// put 'this' to the stack if this is a method
	if (m_currentSubroutine_kind == "method")
	{
		m_VMOutput.writePush(SEG_ARG, 0);
		m_VMOutput.writePop(SEG_POINTER, 0);
	}
	// allocate one cell in case of a construct declaration
	else if (m_currentSubroutine_kind == "constructor")
	{
		m_VMOutput.writePush(SEG_CONST, m_symTab.VarCount(K_FIELD));
		m_VMOutput.writeCall("Memory.alloc", 1);
		m_VMOutput.writePop(SEG_POINTER, 0);
	}

	// check statement*
	compileStatements();

	// check '}'
	m_jtok.advance();
	inspectSymbol( '}' );
}

void JackCompilationEngine::compileParameterList()
{
	// store info for the symbol table
	string idName, idType;
	
	pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();

	// check if this is an empty list
	if ( tok_val.first == TOK_SYMBOL)
	{
		// paramList = ()
		return;
	}

	// check type
	m_jtok.advance();
	inspectType();

	// get the type
	if ( m_jtok.tokenType() == TOK_KEYWORD )
	{
		idType = keyword_to_string( m_jtok.keyword() );
	}
	else
	{
		idType = m_jtok.identifier();
	}

	// check varName
	m_jtok.advance();
	inspectVarName();

	// get the name
	idName = m_jtok.identifier();

	// insert a new argument id
	m_symTab.Define(idName, idType, K_ARG);

	for (;;)
	{
		pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();
		
		if ( tok_val.first != TOK_SYMBOL )
		{
			throw WrongToken( TOK_SYMBOL, tok_val.first, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
		}
		
		string val = tok_val.second;

		// if ',' check other varnames
		if (val == ",")
		{
			// check ','
			m_jtok.advance();
			inspectSymbol( ',' );

			// check type
			m_jtok.advance();
			inspectType();

			// get the type
			if ( m_jtok.tokenType() == TOK_KEYWORD )
			{
				idType = keyword_to_string( m_jtok.keyword() );
			}
			else
			{
				idType = m_jtok.identifier();
			}
				
			// check varname
			m_jtok.advance();
			inspectVarName();

			// get the name
			idName = m_jtok.identifier();

			// insert another argument id
			m_symTab.Define(idName, idType, K_ARG);
		}
		else if (val == ")")
		{
			break;
		}
	}
}

void JackCompilationEngine::compileVarDec()
{
	// store info for the symbol table
	string idName, idType;
	
	// check 'var'
	m_jtok.advance();
	inspectKeyword( KW_VAR );

	// check type
	m_jtok.advance();
	inspectType();

	// get the type
	if ( m_jtok.tokenType() == TOK_KEYWORD )
	{
		idType = keyword_to_string( m_jtok.keyword() );
	}
	else
	{
		idType = m_jtok.identifier();
	}

	// check varName
	m_jtok.advance();
	inspectVarName();

	// get the name
	idName = m_jtok.identifier();

	// insert a new local id
	m_symTab.Define(idName, idType, K_VAR);

	// if ',' check other varnames
	for (;;)
	{
		pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();
		
		if ( tok_val.first != TOK_SYMBOL )
		{
			throw WrongToken( TOK_SYMBOL, tok_val.first, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
		}

		string val = tok_val.second;

		// if ',' check other varnames
		if (val == ",")
		{
			// check ','
			m_jtok.advance();
			inspectSymbol( ',' );

			// check varname
			m_jtok.advance();
			inspectVarName();

			// get the name
			idName = m_jtok.identifier();

			// insert another local id
			m_symTab.Define(idName, idType, K_VAR);
		}
		else if (val == ";")
		{
			// check ';'
			m_jtok.advance();
			inspectSymbol( ';' );

			break;
		}
	}
}

void JackCompilationEngine::compileStatements()
{
	// check if there's at least one statement
	if ( m_jtok.peek().first != TOK_KEYWORD )
	{
		return;
	}
	

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
}

void JackCompilationEngine::compileDo()
{
	// check 'do'
	m_jtok.advance();
	inspectKeyword( KW_DO );

	// compile subroutinecall
	m_jtok.advance();
	compileSubroutineCall();

	
	m_VMOutput.writePop(SEG_TEMP, 0);

	// check ';'
	m_jtok.advance();
	inspectSymbol(';');
}

void JackCompilationEngine::compileLet()
{
	// check 'let'
	m_jtok.advance();
	inspectKeyword( KW_LET );

	// check varname
	m_jtok.advance();
	inspectVarName();

	string str = m_jtok.identifier();
	KIND var_kind = m_symTab.KindOf( str );
	int var_index = m_symTab.IndexOf( str );

	// varname can be an array, check varname[expr]
	pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();
	
	if (tok_val.first != TOK_SYMBOL)
	{
		throw WrongToken( TOK_SYMBOL, tok_val.first, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
	}

	string val = tok_val.second;

	if (val == "[")
	{
		m_jtok.advance();
		inspectSymbol( '[' );

		// check expression
		compileExpression();

		// write VM 
		pushIdentifier(var_kind, var_index);

		// compute pointer address
		m_VMOutput.writeArithmetic( C_ADD );
		
		// check ']'
		m_jtok.advance();
		inspectSymbol( ']' );

		// check '='
		m_jtok.advance();
		inspectSymbol( '=' );

		// check expression
		compileExpression();

		// we save the rvalue in a temp cell
		m_VMOutput.writePop(SEG_TEMP, 0);
		// we save the lvalue as a pointer address
		m_VMOutput.writePop(SEG_POINTER, 1);
		// we assign the rvalue in the address pointed by 'that'
		m_VMOutput.writePush(SEG_TEMP, 0);
		m_VMOutput.writePop(SEG_THAT, 0);

		// check ';'
		m_jtok.advance();
		inspectSymbol(';');
	}
	else if (val == "=")
	{
		m_jtok.advance();
		inspectSymbol( '=' );

		// check expression
		compileExpression();

		// write VM 
		popIdentifier(var_kind, var_index);

		// check ';'
		m_jtok.advance();
		inspectSymbol(';');
	}
}

void JackCompilationEngine::compileWhile()
{
	// see compileIf() comments for 'local_if_counter'
	int local_while_counter = m_whileCounter++;
	
	// check 'while'
	m_jtok.advance();
	inspectKeyword( KW_WHILE );

	// check '('
	m_jtok.advance();
	inspectSymbol('(');

	// always check the expression at the beginning of every loop
	m_VMOutput.writeLabel( "WHILE_EXP", local_while_counter );

	// check expression
	compileExpression();

	// we invert the result of the expression
	// because 'while' statement loop until if-goto is !true
	m_VMOutput.writeArithmetic( C_NOT );
	// if-goto
	m_VMOutput.writeIf( "WHILE_END", local_while_counter );
	
	
	// check ')'
	m_jtok.advance();
	inspectSymbol(')');

	// check '{'
	m_jtok.advance();
	inspectSymbol('{');

	// check statements
	compileStatements();

	// return to 'WHILE_EXP'
	m_VMOutput.writeGoto( "WHILE_EXP", local_while_counter );
	// or exit the loop
	m_VMOutput.writeLabel( "WHILE_END", local_while_counter );

	// check '}'
	m_jtok.advance();
	inspectSymbol('}');
}

void JackCompilationEngine::compileReturn()
{
	// check 'return'
	m_jtok.advance();
	inspectKeyword( KW_RETURN );

	// check if there's one expression
	pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();
	
	// a constructor must return 'this'
	if (m_currentSubroutine_kind == "constructor")
	{
		if (tok_val.second != "this")
		{
			throw JackError("A constructor must return 'this'", m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
		}
	}
	
	if (tok_val.second == ";")
	{
		if (m_currentSubroutine_type != "void")
		{
			throw JackError("A void function must return no value", m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
		}
		
		// check ';'
		m_jtok.advance();
		inspectSymbol(';');

		// VM command 'return' always pop 1 value from the stack
		// so we push a dummy value 
		m_VMOutput.writePush(SEG_CONST, 0);
		m_VMOutput.writeReturn();
	}
	else
	{
		if (m_currentSubroutine_type == "void")
		{
			throw JackError("A non-void function must return a value", m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
		}

		// TODO : check constructor, method, non-void functions and so on...

		compileExpression();

		// check ';'
		m_jtok.advance();
		inspectSymbol(';');

		m_VMOutput.writeReturn();
	}
}

void JackCompilationEngine::compileIf()
{
	// we must ensure that this counter is updated
	// at the beginning (at least before 'statements')
	int local_if_counter = m_ifCounter++;

	// check 'if'
	m_jtok.advance();
	inspectKeyword( KW_IF );

	// check '('
	m_jtok.advance();
	inspectSymbol('(');

	// check expression
	compileExpression();

	// if-goto
	m_VMOutput.writeIf( "IF_TRUE", local_if_counter );
	m_VMOutput.writeGoto( "IF_FALSE", local_if_counter );

	// check ')'
	m_jtok.advance();
	inspectSymbol(')');

	// check '{'
	m_jtok.advance();
	inspectSymbol('{');

	// label TRUE 'if'
	m_VMOutput.writeLabel( "IF_TRUE", local_if_counter );

	// check statements
	compileStatements();

	// check '}'
	m_jtok.advance();
	inspectSymbol('}');

	// we could have 0 or 1 'else'
	pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();

	if (tok_val.second == "else")
	{
		// check 'else'
		m_jtok.advance();
		inspectKeyword( KW_ELSE );

		// 'if' and 'else' path flow are exclusive (XOR)
		m_VMOutput.writeGoto( "IF_END", local_if_counter );
		// label FALSE 'if'
		m_VMOutput.writeLabel( "IF_FALSE", local_if_counter );
		
		// check '{'
		m_jtok.advance();
		inspectSymbol('{');

		// check statements
		compileStatements();

		// label FALSE 'else'
		m_VMOutput.writeLabel( "IF_END", local_if_counter );

		// check '}'
		m_jtok.advance();
		inspectSymbol('}');
	}
	else
	{
		// in case we just have an 'if' statement
		m_VMOutput.writeLabel( "IF_FALSE", local_if_counter );
	}
}

void JackCompilationEngine::compileSubroutineCall()
{
	string subr_name, id_name;

	pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();

	if (tok_val.first != TOK_SYMBOL)
	{
		throw WrongToken( TOK_SYMBOL, tok_val.first, m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
	}

	string val = tok_val.second;

	// direct call
	if (val == "(")
	{
		// check subroutineName
		inspectSubroutineName();
		
		subr_name = m_jtok.identifier();

		// check '('
		m_jtok.advance();
		inspectSymbol('(');	

		// if the identifier is a varName, we need to push his value to the heap before
		// evaluating the expression in parenthesis
		if ( m_symTab.TypeOf( id_name ) != "" )
		{
			// push identifier before any other value
			KIND kind = m_symTab.KindOf( id_name );
			int idx = m_symTab.IndexOf( id_name );
			pushIdentifier( kind, idx );
		}
		else
		{
			m_VMOutput.writePush( SEG_POINTER, 0 );
		}

		// check expressionList
		compileExpressionList();
		

		// At this point, the subroutine MUST be defined in the current class
		map<string, SubroutineInfo>::iterator subr_it, subr_it_end;

		subr_it = m_classSubroutine_params.find( subr_name );
		subr_it_end = m_classSubroutine_params.end();

		if (subr_it != subr_it_end)
		{
			string subr_kind = subr_it->second.kind;
			int subr_nArgs = subr_it->second.nArgs;

			// 'this' is the first argument
			if (subr_kind == "method")
			{
				subr_nArgs += 1;
			}
		
			// if the identifier is a varName, we write his type instead of his value
			if ( m_symTab.TypeOf( id_name ) != "" )
			{	
				string type = m_symTab.TypeOf( id_name );
				m_VMOutput.writeCall(string( type + "." + subr_name ), subr_nArgs);
			}
			// if it's a direct call, don't forget to push 'this'
			else
			{
				m_VMOutput.writeCall(string( m_className + "." + subr_name ), subr_nArgs);
			}
		}

		// check ')'
		m_jtok.advance();
		inspectSymbol(')');
	}
	// call from class (represented by a className or a varName)
	else if (val == ".")
	{
		// check whether it's a classname or a varname
		inspectIdentifier();

		id_name = m_jtok.identifier();

		// check '.'
		m_jtok.advance();
		inspectSymbol('.');

		// check subRoutineName
		m_jtok.advance();
		inspectSubroutineName();

		subr_name = m_jtok.identifier();

		// check '('
		m_jtok.advance();
		inspectSymbol('(');

		// if the identifier is a varName, we need to push his value to the heap before
		// evaluating the expression in parenthesis
		if ( m_symTab.TypeOf( id_name ) != "" )
		{
			// push identifier before any other value
			KIND kind = m_symTab.KindOf( id_name );
			int idx = m_symTab.IndexOf( id_name );
			pushIdentifier( kind, idx );
		}

		// check expressionList
		compileExpressionList();


		// if the identifier is a varName, we write his type instead of his value
		if ( m_symTab.TypeOf( id_name ) != "" )
		{
			string type = m_symTab.TypeOf( id_name );
			m_VMOutput.writeCall(string( type + "." + subr_name ), m_externSubroutine_params + 1);
		}
		// if it's a class, just write the vm call
		else
		{
			m_VMOutput.writeCall(string( id_name + "." + subr_name ), m_externSubroutine_params);
		}

		// check ')'
		m_jtok.advance();
		inspectSymbol(')');
	}

	// reset args counter for non-class subroutines
	m_externSubroutine_params = 0;
}

void JackCompilationEngine::compileExpression()
{
	// TODO : vérifier leur type et lever une exception de type InvalidType s'ils sont différents du type entier 'int'

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
			inspectOp();

			// check term
			compileTerm();

			// write VM operations in accordance to the RPN
			// i.e. term1 term2 op
			if (val == "*")
			{
				m_VMOutput.writeCall("Math.multiply", 2);
			}
			else if (val == "/")
			{
				m_VMOutput.writeCall("Math.divide", 2);
			}
			else
			{
				m_VMOutput.writeArithmetic( char_to_op( val[0] ));
			}
		}
		else
		{
			break;
		}	
	}
}

void JackCompilationEngine::compileTerm()
{
	m_jtok.advance();

	TYPE_TOKEN tt = m_jtok.tokenType();
	
	// check integer
	if ( tt == TOK_INT_CONST )
	{
		inspectIntegerConstant();

		m_VMOutput.writePush(SEG_CONST, m_jtok.intVal());
	}
	// check string
	else if ( tt == TOK_STRING_CONST )
	{
		//inspectStringConstant();
		// alloc a pointer with string.size() elements
		string str = m_jtok.stringVal();
		int size = str.size();

		m_VMOutput.writePush(SEG_CONST, size);
		m_VMOutput.writeCall("String.new", 1);
		// push elements char by char
		for (int i = 0; i < size; i++)
		{
			m_VMOutput.writePush(SEG_CONST, str[i]);
			m_VMOutput.writeCall("String.appendChar", 2);
		}
	}
	// check keyword constant
	else if ( tt == TOK_KEYWORD )
	{
		inspectKeywordConstant();
		
		switch ( m_jtok.keyword() )
		{
		case KW_THIS:
			m_VMOutput.writePush(SEG_POINTER, 0);
			break;
		case KW_TRUE:
			{
				m_VMOutput.writePush(SEG_CONST, 0);
				m_VMOutput.writeArithmetic(C_NOT);
				break;
			}
		case KW_NULL:
		case KW_FALSE:
			m_VMOutput.writePush(SEG_CONST, 0);
			break;
		}
	}
	else if ( tt == TOK_SYMBOL )
	{
		char sym = m_jtok.symbol();

		// check '(' expr ')'
		if ( sym == '(' )
		{
			inspectSymbol( '(' );

			// check expression
			compileExpression();

			// check ')'
			m_jtok.advance();
			inspectSymbol( ')' );
		}
		// check unaryOp
		else if ( sym == '-' || sym == '~' )
		{
			inspectUnaryOp();

			// check term
			compileTerm();

			m_VMOutput.writeArithmetic( char_to_unaryOp( sym ));
		}
	}
	else if ( tt == TOK_IDENTIFIER )
	{
		pair<TYPE_TOKEN, string> tok_val = m_jtok.peek();

		// check varname[expr]
		if (tok_val.second == "[")
		{
			inspectVarName();

			string str = m_jtok.identifier();
			KIND var_kind = m_symTab.KindOf( str );
			int var_index = m_symTab.IndexOf( str );

			// check '['
			m_jtok.advance();
			inspectSymbol( '[' );

			// check expression
			compileExpression();

			// check ']'
			m_jtok.advance();
			inspectSymbol( ']' );

			// write VM 
			pushIdentifier(var_kind, var_index);

			// compute pointer address
			m_VMOutput.writeArithmetic( C_ADD );
			m_VMOutput.writePop(SEG_POINTER, 1);
			m_VMOutput.writePush(SEG_THAT, 0);
		}
		// check subroutine call
		else if (tok_val.second == "(" || tok_val.second == ".")
		{
			compileSubroutineCall();
		}
		// check varname
		else
		{
			inspectVarName();

			string str = m_jtok.identifier();
			KIND var_kind = m_symTab.KindOf( str );
			int var_index = m_symTab.IndexOf( str );
			
			// write VM 
			pushIdentifier(var_kind, var_index);
		}
	}
}

void JackCompilationEngine::compileExpressionList()
{
	// check if there's at least one expr
	if ( m_jtok.peek().second != ")" )
	{
		// check expression
		compileExpression();

		m_externSubroutine_params++;

		// check if there's other expr
		for (;;)
		{
			if ( m_jtok.peek().second == "," )
			{
				// check ','
				m_jtok.advance();
				inspectSymbol( ',' );

				// check expression
				compileExpression();

				m_externSubroutine_params++;
			}
			else
			{
				break;
			}
		}
	}
}

/******************************************
 * 
 * inspect functions
 * Check type correctness
 *
 ******************************************/


/* inspect identifier */
void JackCompilationEngine::inspectIdentifier()
{
	TYPE_TOKEN type = m_jtok.tokenType();
	int line = m_jtok.getCurrentLine();
	int column = m_jtok.getCurrentColumn();

	if ( type != TOK_IDENTIFIER)
	{
		throw WrongToken( TOK_IDENTIFIER, type, m_fileName, line, column );
	}
}

void JackCompilationEngine::inspectClassName()
{
	inspectIdentifier();
}

void JackCompilationEngine::inspectVarName()
{
	inspectIdentifier();
}

void JackCompilationEngine::inspectSubroutineName()
{
	inspectIdentifier();
}

/* inspect keyword */

void JackCompilationEngine::inspectKeyword( TYPE_KEYWORD tk )
{
	TYPE_TOKEN type = m_jtok.tokenType();
	int line = m_jtok.getCurrentLine();
	int column = m_jtok.getCurrentColumn();

	if ( type == TOK_KEYWORD && m_jtok.keyword() != tk)
	{
		throw KeywordNotFound( tk, m_fileName, line, column);
	}
	else if ( type != TOK_KEYWORD)
	{
		throw WrongToken( TOK_KEYWORD, type, m_fileName, line, column);
	}
}

/* vector version */

void JackCompilationEngine::inspectKeyword( vector<TYPE_KEYWORD> tk )
{
	TYPE_TOKEN type = m_jtok.tokenType();
	int line = m_jtok.getCurrentLine();
	int column = m_jtok.getCurrentColumn();

	if ( type == TOK_KEYWORD )
	{
		bool found = false;
		TYPE_KEYWORD key = m_jtok.keyword();

		for ( vector<TYPE_KEYWORD>::iterator it = tk.begin(), it_end = tk.end();
			  it != it_end;
			  ++it)
		{
			if ( key == *it)
			{
				found = true;
				break;
			}
		}

		if ( !found )
		{
			throw KeywordNotFound( tk, m_fileName, line, column);
		}
	}
	else if ( type != TOK_KEYWORD)
	{
		throw WrongToken( TOK_KEYWORD, type, m_fileName, line, column);
	}
}

/* inspect symbol */

void JackCompilationEngine::inspectSymbol( char symbol )
{
	TYPE_TOKEN type = m_jtok.tokenType();
	int line = m_jtok.getCurrentLine();
	int column = m_jtok.getCurrentColumn();

	if ( type == TOK_SYMBOL && m_jtok.symbol() != symbol)
	{
		throw SymbolNotFound( symbol, m_fileName, line, column);
	}
	else if ( type != TOK_SYMBOL)
	{
		throw WrongToken( TOK_SYMBOL, type, m_fileName, line, column);
	}
}

/* vector version */

void JackCompilationEngine::inspectSymbol( vector<char> symbols )
{
	TYPE_TOKEN type = m_jtok.tokenType();
	int line = m_jtok.getCurrentLine();
	int column = m_jtok.getCurrentColumn();

	if ( type == TOK_SYMBOL )
	{
		bool found = false;
		char sym = m_jtok.symbol();
		
		for ( vector<char>::iterator it = symbols.begin(), it_end = symbols.end();
			  it != it_end;
			  ++it)
		{
			if ( *it == sym)
			{
				found = true;
				break;
			}
		}

		if ( !found )
		{
			throw SymbolNotFound( symbols, m_fileName, line, column);
		}
	}
	else if ( type != TOK_SYMBOL)
	{
		throw WrongToken( TOK_SYMBOL, type, m_fileName, line, column);
	}
}

/* inspect constants */

void JackCompilationEngine::inspectIntegerConstant()
{
	TYPE_TOKEN type = m_jtok.tokenType();
	int line = m_jtok.getCurrentLine();
	int column = m_jtok.getCurrentColumn();

	if ( type == TOK_INT_CONST)
	{
		int val = m_jtok.intVal();

		if ( val < 0 || val > 32767 )
		{
			 throw IntegerOutOfRange(m_fileName, line, column);
		}
	}
	else if ( type != TOK_INT_CONST)
	{
		throw WrongToken( TOK_INT_CONST, type, m_fileName, line, column);
	}
}

void JackCompilationEngine::inspectStringConstant()
{
	TYPE_TOKEN type = m_jtok.tokenType();
	int line = m_jtok.getCurrentLine();
	int column = m_jtok.getCurrentColumn();

	if ( type != TOK_STRING_CONST)
	{
		throw WrongToken( TOK_STRING_CONST, type, m_fileName, line, column);
	}
}

void JackCompilationEngine::inspectKeywordConstant()
{
	inspectKeyword( m_kwConstant );
}

void JackCompilationEngine::inspectOp()
{
	inspectSymbol( m_op );
}

void JackCompilationEngine::inspectUnaryOp()
{
	inspectSymbol( m_unaryOp );
}


void JackCompilationEngine::inspectType()
{
	TYPE_TOKEN type = m_jtok.tokenType();
	int line = m_jtok.getCurrentLine();
	int column = m_jtok.getCurrentColumn();
	
	if (type != TOK_KEYWORD && type != TOK_IDENTIFIER)
	{
		throw WrongType( type, m_fileName, line, column);
	}
}

/** Output utils functions */

void JackCompilationEngine::pushIdentifier(KIND kind, int index)
{
	// write VM 
	if (kind == K_ARG)
	{
		// if the identifier is inside a method
		// we need to add +1 to the index (arg0 => 'this')
		if (m_currentSubroutine_kind == "method")
		{
			m_VMOutput.writePush( SEG_ARG, index + 1 );
		}
		else
		{
			m_VMOutput.writePush( SEG_ARG, index );
		}
	}
	else if (kind == K_FIELD)
	{
		// check if it's inside a constructor or method
		if (m_currentSubroutine_kind == "method" || m_currentSubroutine_kind == "constructor")
		{
			m_VMOutput.writePush( kind_to_segment( kind ), index );
		}
		else
		{
			throw JackError("A field identifier must be used inside a constructor or a segment",
				m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
		}
	}
	else
	{
		m_VMOutput.writePush( kind_to_segment( kind ), index  );
	}
}

void JackCompilationEngine::popIdentifier(KIND kind, int index)
{
	// write VM 
	if (kind == K_ARG)
	{
		// if the identifier is inside a method
		// we need to add +1 to the index (arg0 => 'this')
		if (m_currentSubroutine_kind == "method")
		{
			m_VMOutput.writePop( SEG_ARG, index + 1 );
		}
		else
		{
			m_VMOutput.writePop( SEG_ARG, index );
		}
	}
	else if (kind == K_FIELD)
	{
		// check if it's inside a constructor or method
		if (m_currentSubroutine_kind == "method" || m_currentSubroutine_kind == "constructor")
		{
			m_VMOutput.writePop( kind_to_segment( kind ), index );
		}
		else
		{
			throw JackError("A field identifier must be used inside a constructor or a segment",
				m_fileName, m_jtok.getCurrentLine(), m_jtok.getCurrentColumn());
		}
	}
	else
	{
		m_VMOutput.writePop( kind_to_segment( kind ), index  );
	}
}
