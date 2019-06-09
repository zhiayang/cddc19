// lscvm-memoryinator.cpp

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <map>
#include <vector>
#include <string>
#include <iostream>

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
			case 11: return "fgA";
			case 12: return "ggA";
			case 13: return "ghA";
			case 14: return "hhA";
			case 15: return "hiA";
			case 16: return "iiA";
			case 17: return "ijA";
			case 18: return "jjA";
			case 19: return "jjAbA";
			case 20: return "efM";
			case 21: return "dhM";
			case 22: return "efMcA";
			case 23: return "efMdA";
			case 24: return "diM";
			case 25: return "ffM";
			case 26: return "ffMbA";
			case 27: return "djM";
			case 28: return "ehM";
			case 29: return "ehMbA";
			case 30: return "fgM";
			case 31: return "fgMbA";
			case 32: return "eiM";

			case 'a': return "jjMjAhA";
			case 'b': return "hhAhM";
			case 'c': return "jcAjM";
			case 'd': return "effMM";
			case 'e': return "effMMbA";
			case 'f': return "jiAgM";
			case 'g': return "hhAhMfA";
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


int main(int argc, char** argv)
{
	bool squeeze = false;
	std::string input;

	if(argc > 1)
	{
		if(strcmp("--squeeze", argv[1]) == 0)
		{
			squeeze = true;
		}
		else
		{
			fprintf(stderr, "unknown option '%s'\n", argv[1]);
			exit(-1);
		}
	}




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

		std::map<char, size_t> offsets;

		if(squeeze)
		{
			// dump all the characters out.
			std::vector<char> chars = {
				'Z', 'V', 'S', 'R', 'K', 'J', 'I', 'H', 'G', 'E', 'D', 'C', 'B',
				'M', 'A', 'j', 'i', 'h', 'g', 'f', 'e', 'd', 'c', 'b', 'a', 'F', 'P'
			};

			for(auto thechar : chars)
			{
				output += createNumber(thechar);
				offsets.insert(std::make_pair(thechar, chars.size() - offsets.size()));
			}
		}



		// write the offset once
		output += createNumber(offset);

		for(size_t i = 0; i < input.size(); i++)
		{
			if(squeeze)
			{
				// see if we will benefit from fetching.
				auto _fetchOfs = offsets[input[i]];
				assert(_fetchOfs > 0);

				// get how many chars it takes to fetch the thing from stack
				auto fetchOfs = createNumber(_fetchOfs) + "F";

				auto number = createNumber(input[i]);

				if(fetchOfs.size() < number.size())
					output += fetchOfs;

				else
					output += number;
			}
			else
			{
				output += createNumber(input[i]); // this is the value
			}

			// K writes to memory. format: [value] [address] K
			output += "bF";   // fetch the offset
			output += "K";

			// add one
			if(i + 1 < input.size())
				output += "bA";
		}

		printf("\n\noutput (%zu chars):\n", output.size());
		printf("\n%s\n\n", output.c_str());
	}
}













