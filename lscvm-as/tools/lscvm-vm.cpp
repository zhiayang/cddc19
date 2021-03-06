// lscvm.cpp
// Copyright (c) 2019, zhiayang
// Licensed under the Apache License Version 2.0.

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include <array>
#include <string>
#include <vector>

#include <fstream>
#include <sstream>
#include <iostream>

#define MEMORY_SIZE 0x13880

struct state_t
{
	size_t pc;
	size_t cycles;

	std::vector<char> instructions;

	std::vector<uint32_t> stack;
	std::vector<uint32_t> callStack;

	std::array<uint32_t, MEMORY_SIZE> memory;
};

static void hexdump(uint32_t* values, size_t cnt);
static std::vector<char> cleanInput(const std::string& input);
static void run(state_t* st);

bool minify = false;
bool dontRun = false;
bool debugMode = false;

int main(int argc, char** argv)
{
	std::string filename;
	bool read_stdin = false;
	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "--debug") == 0)
			debugMode = true;

		else if(strcmp(argv[i], "--minify") == 0)
			minify = true;

		else if(strcmp(argv[i], "--compile") == 0)
			dontRun = true;

		else if(strcmp(argv[i], "-") == 0)
			read_stdin = true;

		else if(filename.empty())
			filename = argv[i];

		else
			fprintf(stderr, "invalid option '%s'\n", argv[i]), exit(-1);
	}

	std::string input;
	if(read_stdin)
	{
		std::getline(std::cin, input);
	}
	else
	{
		auto file = std::ifstream(argv[1], std::ios::in);
		if(!file) printf("failed to open input file '%s'\n", argv[1]), exit(-1);

		std::stringstream ss;
		ss << file.rdbuf();

		input = ss.str();
	}

	state_t st;

	st.pc = 0;
	st.cycles = 0;
	st.memory.fill(0);
	st.instructions = cleanInput(input);

	if(minify || dontRun)
	{
		auto s = (dontRun ? stdout : stderr);

		fprintf(s, "\ncompiled code: (%zu bytes)\n\n", st.instructions.size());
		for(auto c : st.instructions)
			fputc(c, s);

		fprintf(s, "\n\n");
	}

	if(!dontRun)
		run(&st);

	printf("\n");
	// hexdump(&st.memory[0], MEMORY_SIZE);
}











static void halt(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	fprintf(stderr, "\n");

	vfprintf(stderr, fmt, args);
	fprintf(stderr, "vm error! exiting...\n");
	exit(-1);

	va_end(args);
}

static void run(state_t* st)
{
	auto pop = [st]() -> uint32_t {
		if(st->stack.empty())
			halt("stack underflow\n");

		auto ret = st->stack.back();
		st->stack.pop_back();

		return ret;
	};

	auto push = [st](uint32_t x) {
		st->stack.push_back(x);
	};


	while(st->pc < st->instructions.size())
	{
		auto op = st->instructions[st->pc];

		if(debugMode && !isspace(op))
		{
			printf("op: %c | pc: %zu | cyc: %zu\n", op, st->pc, st->cycles);
		}

		switch(op)
		{
			case 'a': push(0); break;
			case 'b': push(1); break;
			case 'c': push(2); break;
			case 'd': push(3); break;
			case 'e': push(4); break;
			case 'f': push(5); break;
			case 'g': push(6); break;
			case 'h': push(7); break;
			case 'i': push(8); break;
			case 'j': push(9); break;

			// add two operands
			case 'A': {
				auto b = pop();
				auto a = pop();
				push(a + b);
			} break;

			// stop execution
			case 'B': {
				// follow the semantics of the original vm, which is to move the pc to the end.
				st->pc = st->instructions.size();
			} break;

			// call
			case 'C': {
				size_t f = pop();
				if(f >= st->instructions.size())
					halt("C: call to instruction '%u' out of bounds (max %zu)\n", st->instructions.size() - 1);

				st->callStack.push_back(st->pc);

				// -1 cos later we will +1 it.
				// if we call to 0, then we will underflow and overflow correctly, which is defined behaviour.
				st->pc = f - 1;
			} break;

			// pop
			case 'D': {
				pop();
			} break;

			// read from memory
			case 'E': {
				auto addr = pop();
				if(addr >= MEMORY_SIZE)
					halt("E: read from address '%x' out of bounds\n", addr);

				push(st->memory[addr]);
			} break;

			// fetch from stack
			case 'F': {
				auto ofs = pop();
				if(ofs >= st->stack.size())
					halt("F: fetch stack '%d' out of bounds\n", ofs);

				auto x = st->stack[st->stack.size() - 1 - ofs];
				push(x);
			} break;

			// relative jump forward
			case 'G': {
				int32_t jmp = pop();
				if(st->pc + jmp >= st->instructions.size())
					halt("G: jump to instruction '%u' out of bounds (max %zu)\n", st->pc, st->instructions.size() - 1);

				st->pc += jmp;
			} break;

			// same as fetch (F), but remove it also.
			case 'H': {
				auto ofs = pop();
				if(ofs >= st->stack.size())
					halt("H: fetch stack '%d' out of bounds\n", ofs);

				auto x = st->stack[st->stack.size() - 1 - ofs];
				st->stack.erase(st->stack.end() - 1 - ofs);
				push(x);
			} break;

			// print as integer
			case 'I': {
				auto i = pop();
				printf("%d", i); fflush(stdout);
			} break;

			// compare (-1, 0, +1)
			case 'J': {
				auto b = pop();
				auto a = pop();

				if(a < b)  push(-1);
				if(a == b) push(0);
				if(a > b)  push(1);
			} break;

			// write to memory
			case 'K': {
				auto addr = pop();
				auto val = pop();

				if(addr >= MEMORY_SIZE)
					halt("K: write to address '%x' out of bounds\n", addr);

				st->memory[addr] = val;
			} break;

			// multiply
			case 'M': {
				auto b = pop();
				auto a = pop();
				push(a * b);
			} break;

			// print char
			case 'P': {
				auto c = pop();
				printf("%c", c); fflush(stdout);
			} break;

			// return
			case 'R': {
				auto ret = st->callStack.back();
				st->callStack.pop_back();

				st->pc = ret;
			} break;

			// subtract
			case 'S': {
				auto b = pop();
				auto a = pop();
				push(a - b);
			} break;

			// divide
			case 'V': {
				auto b = pop();
				auto a = pop();
				push(a / b);
			} break;

			// relative jump forward if 0 (cond, ofs)
			case 'Z': {
				int32_t ofs = pop();
				auto cond = pop();
				if(cond == 0)
				{
					st->pc += ofs;
					if(st->pc >= st->instructions.size())
						halt("Z: jump to instruction '%u' out of bounds (max %zu)\n", st->pc, st->instructions.size() - 1);
				}
			} break;

			// debug: dump stack
			case '?': {
				printf("\nstack dump:\n");
				hexdump(&st->stack[0], st->stack.size());
				printf("\n");
			} break;

			// debug: dump memory.
			case '!': {
				printf("\nmemory dump:\n");
				hexdump(&st->memory[0], MEMORY_SIZE);
				printf("\n");
			} break;

			case '-':
			case ' ': {
			} break;


			default: {
				halt("invalid instruction '%c'!\n", op);
			}
		}

		st->pc++;
		st->cycles++;

		if(debugMode && !isspace(op))
		{
			printf("stack: [");
			for(auto x : st->stack)
				printf(" %d", x);

			printf(" ]\n");
			printf("callstack:\n");
			for(auto x : st->callStack)
				printf("   %d\n", x);

			printf("\n");
		}
	}
}








static void hexdump(uint32_t* arr, size_t len)
{
	constexpr int ValuesPerRow = 8;

	auto iszero = [](uint32_t* ptr, size_t len) -> bool {
		for(size_t i = 0; i < len; i++)
			if(ptr[i]) return false;

		return true;
	};


	int all0sCnt = 0;
	for(size_t i = 0; (len - i >= ValuesPerRow) && (i < len); i += ValuesPerRow)
	{
		if(all0sCnt > 0)
		{
			while((len - ValuesPerRow - i >= ValuesPerRow) && (i < len - ValuesPerRow) && iszero(arr + i, ValuesPerRow))
				i += ValuesPerRow;

			printf("    *\n");
		}

		printf("%5zx:  ", i);
		for(size_t k = 0; k < ValuesPerRow; k++)
			printf("  %8x", arr[i + k]);

		printf("    |");

		for(size_t k = 0; k < ValuesPerRow; k++)
		{
			auto c = arr[i + k];
			(c >= 32 && c <= 127) ? putchar(c) : putchar('.');
		}

		printf("|\n");

		if(iszero(arr + i, ValuesPerRow))
			all0sCnt++;

		else
			all0sCnt = 0;
	}


	if(auto rem = len % ValuesPerRow; rem > 0)
	{
		auto tmp = len - (len % ValuesPerRow);

		printf("%5zx:  ", tmp);
		for(size_t i = 0; i < rem; i++)
			printf("  %8x", arr[tmp + i]);

		for(size_t i = 0; i < (ValuesPerRow - rem); i++)
			printf("          ");

		printf("    |");
		for(size_t i = 0; i < rem; i++)
			(arr[tmp + i] >= 32 && arr[tmp + i] <= 127) ? putchar(arr[tmp + i]) : putchar('.');

		printf("|\n");
	}
}








static std::vector<char> cleanInput(const std::string& input)
{
	std::vector<char> ret;
	ret.reserve(input.size());

	for(size_t i = 0; i < input.size(); i++)
	{
		char c = input[i];

		if(isspace(c))
		{
			// if(!minify) ret.push_back(' ');
		}
		else if(c == '-')
		{
			// we use this as filler in relocations. the vm will ignore it.
			ret.push_back('-');
		}
		else if(c == '!' || c == '?')
		{
			// special command to dump memory and stack.
			ret.push_back(c);
		}
		else if(c == ';' || c == '#')
		{
			// skip till end of line.
			while(input[i] != '\n') i++;
		}
		else
		{
			// ok, some basic validation
			if(!isalpha(c) || (c >= 'k' && c <= 'z') || (c == 'L' || c == 'N' || c == 'O' || c == 'Q'
				|| c == 'T' || c == 'U' || c == 'W' || c == 'X' || c == 'Y'))
			{
				fprintf(stderr, "warning: skipping invalid input character '%c'\n", c);
			}
			else
			{
				ret.push_back(c);
			}
		}
	}

	ret.shrink_to_fit();
	return ret;
}















