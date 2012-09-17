#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <boost/filesystem.hpp>
#include "jack_analyzer.h"
#include "jack_compiler.h"

using namespace std;
using namespace boost::filesystem;

/* this function use a path class, read the content
and map the data to his path
*/
pair<path, string> assoc_file_to_content(path p)
{
	ifstream in(p.string());

	// read data as a block
	stringbuf sbuf;
	in >> &sbuf;

	// create a string from buffer data
	string buf( sbuf.str() );

	return pair<path, string>(p, buf);
}

int main(int argc, char **argv)
{
	// verify that there's only 1 argument
	if (argc != 2)
	{
		cout << "usage: " << argv[0] << " (filename | directory)" << endl;
		exit(1);
	}

	string in_ext_type = ".jack";
	path p (argv[1]);
	map<path, string> input_files;

	/*
	first, we test the existence of the pathname. then,
	whether it's a file or a directory, we associate
	the path to the content of each JACK file found
	in order to facilitate their manipulation after
	*/
	try
	{
		if (exists(p))
		{
			if (is_regular_file(p) && p.extension() == in_ext_type)
			{			
				input_files.insert( assoc_file_to_content(p) );
			}
			else if (is_directory(p))
			{
				// we do the same as for one file except we search every files
				// with "in_ext_type" in one directory
				vector<path> vec;
				copy(directory_iterator(p), directory_iterator(), back_inserter(vec));

				typedef vector<path>::iterator path_iter;
				for (path_iter it(vec.begin()), it_end(vec.end()); it != it_end; ++it)
				{
					path p = *it;

					if (p.extension() == in_ext_type)
					{
						input_files.insert( assoc_file_to_content(p) );
					}
					
				}
			}
			else
			{
				cout << p << " exists, but is neither a regular file nor a directory " << endl;
			}
		}
		else
		{
			cout << p << " does not exist" << endl;
		}
	}
	catch(const filesystem_error& e)
	{
		cout << e.what() << endl;
	}

	for (map<path, string>::iterator it = input_files.begin(), it_end = input_files.end();
		it != it_end; ++it)
	{
		path p = it->first;
		string pData = it->second;

#ifdef XML_OUTPUT
		JackAnalyzer janalyse(p, pData);
#else
		JackCompiler jcompiler(p, pData);
#endif
	}

	return 0;
}