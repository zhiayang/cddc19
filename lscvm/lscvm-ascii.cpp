// lscvm-deasciiinator.cpp

#include <stdio.h>
#include <string.h>

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


int main(int argc, char** argv)
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
	bool reverse = false;

	if(argc == 2 && strcmp(argv[1], "--reverse") == 0)
		reverse = true;

	while(true)
	{
		printf("string: ");
		std::getline(std::cin, input);

		std::string output;

		// the reason we consolidate all the 'P's at the end is so we can use this to set strings up on the stack,
		// without printing -- we just don't copy the 'P' part.

		if(reverse)
		{
			for(size_t i = 0; i < input.size(); i++)
				output += createNumber(input[i]);
		}
		else
		{
			for(size_t i = input.size(); i-- > 0;)
				output += createNumber(input[i]);
		}

		for(size_t i = 0; i < input.size(); i++)
			output += "P";

		printf("\n%s\n", output.c_str());
	}
}







