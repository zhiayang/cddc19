// lscvm-deasciiinator.cpp

#include <stdio.h>
#include <string.h>

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

		printf("\n%s\n", output.c_str());
	}
}







