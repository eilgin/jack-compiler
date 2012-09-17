#include <stdexcept>
#include <sstream>

#include "symbol_table.h"

using namespace std;

void SymbolTable::startSubroutine()
{
	arg_cur_index = 0;
	var_cur_index = 0;

	m_subroutine_scope.clear();
}

void SymbolTable::Define(string name, string type, KIND kind)
{
	switch ( kind )
	{
	case K_STATIC:
		{
			pair<string, SymbolInfo> static_id( name, SymbolInfo(type, kind, static_cur_index) );
			m_class_scope.insert(static_id);
			static_cur_index++;
			break;
		}
	case K_FIELD:
		{
			pair<string, SymbolInfo> field_id( name, SymbolInfo(type, kind, field_cur_index) );
			m_class_scope.insert(field_id);
			field_cur_index++;
			break;
		}
	case K_ARG:
		{
			pair<string, SymbolInfo> arg_id( name, SymbolInfo(type, kind, arg_cur_index) );
			m_subroutine_scope.insert(arg_id);
			arg_cur_index++;
			break;
		}
	case K_VAR:
		{
			pair<string, SymbolInfo> var_id( name, SymbolInfo(type, kind, var_cur_index) );
			m_subroutine_scope.insert(var_id);
			var_cur_index++;
			break;
		}
	}
}

int SymbolTable::VarCount(KIND kind)
{
	int val;

	if (kind == K_STATIC) val = static_cur_index;
	else if (kind == K_FIELD) val = field_cur_index;
	else if (kind == K_ARG) val = arg_cur_index;
	else if (kind == K_VAR) val = var_cur_index;

	return val;
}

SymbolInfo SymbolTable::FindSymbolInfo(string name)
{
	SymbolInfo si;

	typedef map<string, SymbolInfo> map_symbol;
	map_symbol::iterator symInfo_class = m_class_scope.find( name );
	map_symbol::iterator symInfo_subroutine = m_subroutine_scope.find( name );

	if ( symInfo_class != m_class_scope.end() )
	{
		si = symInfo_class->second;
	}
	else if ( symInfo_subroutine != m_subroutine_scope.end() )
	{
		si = symInfo_subroutine->second;
	}
	else
	{
		// we must not invoke exceptions !
		/*ostringstream oss;
		oss << "Couldn't found the symbol info with the name \"" << name << "\"";
		throw runtime_error( oss.str() );*/
	}

	return si;
}

KIND SymbolTable::KindOf(string name)
{
	return FindSymbolInfo( name ).kind;
}

string SymbolTable::TypeOf(string name)
{
	return FindSymbolInfo( name ).type;
}

int SymbolTable::IndexOf(string name)
{
	return FindSymbolInfo( name ).index;
}
