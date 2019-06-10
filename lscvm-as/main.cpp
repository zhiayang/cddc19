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
	for(int i = 1; i < argc; i++)
	{
		// we have no options yet, lol
		if(false)
		{
		}
		// else if(strcmp(argv[i], "-") == 0)
		// {
		// 	std::stringstream ss;

		// 	std::string line;
		// 	while(std::getline(std::cin, line))
		// 		ss << line;

		// 	input = ss.str();
		// }
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
	parser::parse(st);
}




