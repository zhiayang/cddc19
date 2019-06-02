// lscvm-memoryinator.cpp

#include <stdio.h>

#include <map>
#include <vector>
#include <string>
#include <iostream>

std::map<char, std::string> lookup;

std::string createNumber(int num)
{
	if(num < 10)
	{
		return std::string(1, (char) (num + 'a'));
	}
	else if(num == 10)
	{
		return "cfM";
	}
	else if((num >= 'a' && num <= 'z') || num == '-'
		|| num == '_' || num == '!')
	{
		return lookup[num];
	}
	else
	{
		std::string ret;

		int x = num / 10;
		ret = createNumber(x) + createNumber(10) + "M";

		int y = num % 10;
		ret += createNumber(y) + "A";

		return ret;
	}
}


int main()
{
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
		printf("string: ");
		std::getline(std::cin, input);

		printf("address: ");
		std::string ofs;
		std::getline(std::cin, ofs);

		int offset = 0;
		if(!ofs.empty()) offset = std::stol(ofs);

		std::string output;
		for(size_t i = 0; i < input.size(); i++)
		{
			// K writes to memory. format: K [value] [address]
			output += createNumber(input[i]); // this is the value
			output += createNumber(offset);   // this is the offset
			output += "K";

			offset++;
		}

		printf("\n%s\n", output.c_str());
	}
}








