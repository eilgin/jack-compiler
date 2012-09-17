#ifndef _VM_WRITER_H
#define _VM_WRITER_H

#include <fstream>
#include <string>
#include <boost/filesystem.hpp>
#include "type_utils.h"

using std::ofstream;
using std::string;
using boost::filesystem::path;


class VMWriter {
public:
	VMWriter(path p);

	void writePush(SEGMENT seg, int index);
	void writePop(SEGMENT seg, int index);
	void writeArithmetic(COMMAND cmd);
	void writeLabel(string label, int counter);
	void writeGoto(string label, int counter);
	void writeIf(string label, int counter);
	void writeCall(string name, int nArgs);
	void writeFunction(string name, int nArgs);
	void writeReturn();
	void close();

private:
	ofstream m_out;
};

#endif
