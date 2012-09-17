#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

#include <string>
#include <map>
#include "type_utils.h"


using std::string;
using std::map;

class SymbolTable {
public:
	SymbolTable()
		:static_cur_index(0),
		field_cur_index(0),
		arg_cur_index(0),
		var_cur_index(0)
	{}

	void startSubroutine();
	void Define(string name, string type, KIND kind);
	int VarCount(KIND kind);
	KIND KindOf(string name);
	string TypeOf(string name);
	int IndexOf(string name);
private:
	// identifier name is used as a key
	map<string, SymbolInfo> m_class_scope;
	map<string, SymbolInfo> m_subroutine_scope;
	// for each kind, we store how many indexes are used
	int static_cur_index;
	int field_cur_index;
	int arg_cur_index;
	int var_cur_index;

	SymbolInfo FindSymbolInfo(string name);
};

#endif
