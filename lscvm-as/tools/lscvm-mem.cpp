// lscvm-memoryinator.cpp

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>

static std::unordered_map<int, std::string> cache;
static void populateCache()
{
	for(int i = 0; i < 10; i++)
		cache[i] = 'a' + i;

	cache[10] = "ffA";      cache[11] = "fgA";      cache[12] = "ggA";      cache[13] = "ghA";      cache[14] = "hhA";
	cache[15] = "hiA";      cache[16] = "iiA";      cache[17] = "ijA";      cache[18] = "jjA";      cache[19] = "jjAbA";
	cache[20] = "efM";      cache[21] = "dhM";      cache[22] = "efMcA";    cache[23] = "efMdA";    cache[24] = "diM";
	cache[25] = "ffM";      cache[26] = "ffMbA";    cache[27] = "djM";      cache[28] = "ehM";      cache[29] = "ehMbA";
	cache[30] = "fgM";      cache[31] = "fgMbA";    cache[32] = "eiM";      cache['a'] = "jjMjAhA"; cache['b'] = "hhAhM";
	cache['c'] = "jcAjM";   cache['d'] = "effMM";   cache['e'] = "effMMbA"; cache['f'] = "jiAgM";   cache['g'] = "hhAhMfA";
	cache['h'] = "jeAiM";   cache['i'] = "hdfMM";   cache['j'] = "hdfMMbA"; cache['A'] = "iiMbA";   cache['B'] = "iiMcA";
	cache['C'] = "iiMdA";   cache['D'] = "iiMeA";   cache['E'] = "iiMfA";   cache['F'] = "iiMgA";   cache['G'] = "iiMhA";
	cache['H'] = "iiMiA";   cache['I'] = "iiMjA";   cache['J'] = "ijMcA";   cache['K'] = "ijMdA";   cache['M'] = "ijMfA";
	cache['P'] = "ijMiA";   cache['R'] = "jjMbA";   cache['S'] = "jjMcA";   cache['V'] = "jjMfA";   cache['Z'] = "jjMjA";
	cache['-'] = "fjM";

	// populate all squares from 5*5 to 32*32
	for(int i = 4; i <= 32; i++)
	{
		for(int k = 4; k <= 32; k++)
		{
			auto s = (cache[i] + cache[k] + "M");
			if(auto it = cache.find(i * k); it == cache.end() || s.size() < it->second.size())
				cache[i * k] = s;
		}
	}
}

static std::string convert(int num)
{
	if(cache.empty())
		populateCache();

	std::string ret;

	if(auto it = cache.find(num); it != cache.end())
	{
		ret = it->second;
	}
	else if(num < 0)
	{
		ret = "a" + convert(-num) + "S";
	}
	else if(num < 10)
	{
		ret = std::string(1, (char) (num + 'a'));
	}
	else if(num == 10)
	{
		ret = "cfM";
	}
	else
	{
		// if we got here, then there's nothing exactly cached. so, we try to see if +-9 is cached.
		int best = INT_MAX;
		for(int d = -9; d <= 9; d++)
		{
			if(auto it = cache.find(num + d); it != cache.end())
			{
				auto s = cache[num + d] + (char) ('a' + abs(d)) + std::string(d < 0 ? "A" : "S");
				if(ret.empty() || s.size() < best)
					ret = s, best = s.size();
			}
		}

		if(ret.empty())
		{
			std::string n;

			int x = num / 9;
			if(x == 1)  n = "j";
			else        n = convert(x) + "jM";

			int y = num % 9;
			if(y > 0) n += convert(y) + "A";

			ret = n;
		}
	}

	return ret;
}


int main(int argc, char** argv)
{
	bool reverse = false;
	bool squeeze = false;
	bool stackonly = false;
	bool appendlength = false;
	std::string input;

	for(int i = 1; i < argc; i++)
	{
		if(strcmp("--squeeze", argv[i]) == 0)
		{
			squeeze = true;
		}
		else if(strcmp("--stack", argv[i]) == 0)
		{
			stackonly = true;
		}
		else if(strcmp("--reverse", argv[i]) == 0)
		{
			reverse = true;
		}
		else if(strcmp("--length", argv[i]) == 0)
		{
			appendlength = true;
		}
		else
		{
			fprintf(stderr, "unknown option '%s'\n", argv[i]);
			exit(-1);
		}
	}

	if(stackonly && squeeze)
	{
		fprintf(stderr, "--stack and --squeeze are mutually exclusive\n");
		exit(-1);
	}
	else if(!stackonly && appendlength)
	{
		fprintf(stderr, "--length can only be used with --stack\n");
		exit(-1);
	}


	while(true)
	{
		printf("string: ");
		std::getline(std::cin, input);

		int offset = 0;
		if(!stackonly)
		{
			printf("address: ");
			std::string ofs;
			std::getline(std::cin, ofs);

			if(!ofs.empty())
				offset = std::stol(ofs);
		}

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
				output += convert(thechar);
				offsets.insert(std::make_pair(thechar, chars.size() - offsets.size()));
			}
		}

		if(!stackonly)
		{
			// write the offset once
			output += convert(offset);
		}


		auto action = [&](char c, bool last) {

			if(squeeze)
			{
				// see if we will benefit from fetching.
				auto _fetchOfs = offsets[c];
				assert(_fetchOfs > 0);

				// get how many chars it takes to fetch the thing from stack
				auto fetchOfs = convert(_fetchOfs) + "F";

				auto number = convert(c);

				if(fetchOfs.size() < number.size())
					output += fetchOfs;

				else
					output += number;
			}
			else
			{
				output += convert(c); // this is the value
			}

			if(!stackonly)
			{
				// K writes to memory. format: [value] [address] K
				output += "bF";   // fetch the offset
				output += "K";

				// add one
				if(!last)
					output += "bA";
			}
		};


		if(stackonly ^ reverse)
		{
			for(size_t i = input.size(); i-- > 0;)
				action(input[i], i == 0);
		}
		else
		{
			for(size_t i = 0; i < input.size(); i++)
				action(input[i], i == input.size() - 1);
		}

		if(appendlength)
			output += convert(input.size());

		printf("\n\noutput (%zu chars):\n", output.size());
		printf("\n%s\n\n", output.c_str());
	}
}













