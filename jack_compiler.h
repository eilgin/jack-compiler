#ifndef _JACK_COMPILER_H
#define _JACK_COMPILER_H

#include <string>
#include <map>
#include <boost/filesystem.hpp>
#include "jack_tokenizer.h"
#include "compilation_engine.h"

class JackCompiler {
public:
	JackCompiler(boost::filesystem::path p, std::string jackcode) : m_temp_jtok(jackcode), m_temp_engine(m_temp_jtok, p)
	{
		map<string, SubroutineInfo> methodList = m_temp_engine.getMethodList();

		// final Pass
		JackTokenizer m_final_jtok(jackcode);
		JackCompilationEngine final_pass(m_final_jtok, p, methodList);
	}

private:
	JackTokenizer m_temp_jtok;
	JackCompilationEngine m_temp_engine;
};

#endif
