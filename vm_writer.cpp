

#include "vm_writer.h"

using namespace std;

VMWriter::VMWriter(path p)
{
	// initialize the VM Writer module
	p.replace_extension( ".vm" );

	m_out.open( p.c_str() );
}

void VMWriter::writePush(SEGMENT seg, int index)
{
	m_out << "push " << segment_to_string(seg) << " " << index << endl;
}

void VMWriter::writePop(SEGMENT seg, int index)
{
	m_out << "pop " << segment_to_string(seg) << " " << index << endl;
}

void VMWriter::writeArithmetic(COMMAND cmd)
{
	m_out << command_to_string(cmd) << endl;
}

void VMWriter::writeLabel(string label, int counter)
{
	m_out << "label " << label << counter << endl;
}

void VMWriter::writeGoto(string label, int counter)
{
	m_out << "goto " << label << counter << endl;
}

void VMWriter::writeIf(string label, int counter)
{
	m_out << "if-goto " << label << counter << endl;
}

void VMWriter::writeCall(string name, int nArgs)
{
	m_out << "call " << name << " " << nArgs << endl;
}

void VMWriter::writeFunction(string name, int nArgs)
{
	m_out << "function " << name << " " << nArgs << endl;
}

void VMWriter::writeReturn()
{
	m_out << "return" << endl;
}

void VMWriter::close()
{
	m_out.close();
}