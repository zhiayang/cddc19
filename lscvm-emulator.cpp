// lscvm-emulator.cpp
// Copyright (c) 2019, zhiayang
// Licensed under the Apache License Version 2.0.

#include <stdio.h>

#include <stack>
#include <vector>
#include <string>
#include <iostream>

int main()
{
	std::string input;

	while(true)
	{
		std::getline(std::cin, input);
		printf("\n");

		std::stack<int> stack;
		for(size_t i = 0; i < input.size(); i++)
		{
			int op = input[i];

			if(op >= 'a' && op <= 'z')
			{
				stack.push(op - 'a');
			}
			else if(op == 'M')
			{
				int a = stack.top(); stack.pop();
				int b = stack.top(); stack.pop();
				stack.push(a * b);
			}
			else if(op == 'A')
			{
				int a = stack.top(); stack.pop();
				int b = stack.top(); stack.pop();
				stack.push(a + b);
			}
			else if(op == 'P')
			{
				printf("%c", stack.top());
				 stack.pop();
			}
			else
			{
				printf("invalid opcode '%c'\n", op);
			}
		}

		printf("\n");
	}
}








