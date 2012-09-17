#ifndef _JACK_ANALYZER_H
#define _JACK_ANALYZER_H

#include <string>
#include <boost/filesystem.hpp>
#include "jack_tokenizer.h"
#include "compilation_engine.h"

class JackAnalyzer {
public:
	JackAnalyzer(boost::filesystem::path p, std::string jackcode) :m_jtok(jackcode), m_compEngine(m_jtok, p) {}

private:
	JackTokenizer m_jtok;
	XMLCompilationEngine m_compEngine;
};

#endif
