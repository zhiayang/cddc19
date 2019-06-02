// lscvm-deasciiinator.cpp
// Copyright (c) 2019, zhiayang
// Licensed under the Apache License Version 2.0.

#include <stdio.h>

#include <map>
#include <stack>
#include <vector>
#include <string>
#include <iostream>

int main()
{
	std::map<char, std::string> lookup;

	lookup['a'] = "jjMjAhA";
	lookup['b'] = "jjMjAiA";
	lookup['c'] = "jjMjAjA";
	lookup['d'] = "cfMcfMM";
	lookup['e'] = "cfMcfMMbA";
	lookup['f'] = "jiAcdMM";
	lookup['g'] = "jiAcdMMbA";
	lookup['h'] = "jeAiM";
	lookup['i'] = "hdfMM";
	lookup['j'] = "hdfMMbA";
	lookup['k'] = "hdfMMcA";
	lookup['l'] = "ggdMM";
	lookup['m'] = "ggdMMbA";
	lookup['n'] = "fgAfMcM";
	lookup['o'] = "fgAfMcMbA";
	lookup['p'] = "fgAfMcMcA";
	lookup['q'] = "fgAfMcMdA";
	lookup['r'] = "fgAfMcMeA";
	lookup['s'] = "fgAfMcMfA";
	lookup['t'] = "fgAfMcMgA";
	lookup['u'] = "fgAfMcMhA";
	lookup['v'] = "fgAfMcMiA";
	lookup['w'] = "fgAfMcMjA";
	lookup['x'] = "gcfcMMM";
	lookup['y'] = "fgAfgAM";
	lookup['z'] = "fgAfgAMbA";
	lookup['_'] = "gfdMMfA";
	lookup['!'] = "fgAdM";
	lookup['-'] = "fddMM";


	std::string input;

	while(true)
	{
		std::getline(std::cin, input);

		std::string output;
		for(size_t i = input.size(); i-- > 0; )
			output += lookup[input[i]];

		for(size_t i = 0; i < input.size(); i++)
			output += "P";

		printf("\n%s\n", output.c_str());
	}
}








