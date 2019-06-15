// main.cpp
// Copyright (c) 2019, zhiayang
// Licensed under the Apache License Version 2.0.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include "defs.h"

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		fprintf(stderr, "usage: lscvm-as [options] <input_file>\n");
		exit(-1);
	}

	std::string filename;
	std::string outputFilename;

	for(int i = 1; i < argc; i++)
	{
		// we have no options yet, lol
		if(strcmp("-o", argv[i]) == 0)
		{
			if(++i == argc)
			{
				fprintf(stderr, "expected filename after '-o'\n");
				exit(-1);
			}

			outputFilename = argv[i];
		}
		else
		{
			filename = argv[i];
			if(filename.empty())
			{
				fprintf(stderr, "usage: lscvm-as [options] <input_file>\n");
				exit(-1);
			}
		}
	}

	auto st = parser::State(frontend::getFileTokens(filename));
	auto stmts = parser::parse(st);
	auto output = codegen::assemble(stmts);

	if(outputFilename.empty())
	{
		printf("\n%s\n", output.c_str());
	}
	else
	{
		auto out = std::ofstream(outputFilename, std::ios::out);
		if(!out.is_open())
		{
			fprintf(stderr, "failed to open output file '%s'\n", outputFilename.c_str());
			exit(-1);
		}

		out << output;
		out.close();
	}
}
















