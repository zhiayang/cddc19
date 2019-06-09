// lscvm-memoryinator.cpp

#include <stdio.h>

#include <map>
#include <vector>
#include <string>
#include <iostream>

std::map<char, std::string> lookup;

std::string createNumber(int num)
{
	if(num < 0)
	{
		return "a" + createNumber(-num) + "S";
	}
	else if(num < 10)
	{
		return std::string(1, (char) (num + 'a'));
	}
	else if(num == 10)
	{
		return "cfM";
	}
	else
	{
		switch(num)
		{
			case 'a': return "jjMjAhA";
			case 'b': return "jjMjAiA";
			case 'c': return "jjMjAjA";
			case 'd': return "cfMcfMM";
			case 'e': return "jggAMhS";
			case 'f': return "jiAcdMM";
			case 'g': return "jggAMfS";
			case 'h': return "jeAiM";
			case 'i': return "hdfMM";
			case 'j': return "hdfMMbA";
			case 'A': return "iiMbA";
			case 'B': return "iiMcA";
			case 'C': return "iiMdA";
			case 'D': return "iiMeA";
			case 'E': return "iiMfA";
			case 'F': return "iiMgA";
			case 'G': return "iiMhA";
			case 'H': return "iiMiA";
			case 'I': return "iiMjA";
			case 'J': return "ijMcA";
			case 'K': return "ijMdA";
			case 'M': return "ijMfA";
			case 'P': return "ijMiA";
			case 'R': return "jjMbA";
			case 'S': return "jjMcA";
			case 'V': return "jjMfA";
			case 'Z': return "jjMjA";
			case '!': return "fgAdM";
			case '-': return "fddMM";

			// if we run into code size problems, i'm sure this algo can be optimised.
			default: {
				std::string ret;

				int x = num / 9;
				ret = createNumber(x) + "jM";

				int y = num % 9;
				if(y > 0) ret += createNumber(y) + "A";

				return ret;
			} break;
		}
	}
}


int main()
{
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

		// write the offset once
		output += createNumber(offset);

		for(size_t i = 0; i < input.size(); i++)
		{
			// K writes to memory. format: K [value] [address]
			output += createNumber(input[i]); // this is the value

			output += "bF";   // fetch the offset
			output += "K";

			// add one
			output += "bA";
		}

		printf("\n%s\n", output.c_str());
	}
}













