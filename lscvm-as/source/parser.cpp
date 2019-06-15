// parser.cpp
// Copyright (c) 2019, zhiayang
// Licensed under the Apache License Version 2.0.

#include "defs.h"

namespace parser
{
	using TT = lexer::TokenType;

	static int parseMaxSize(State& st)
	{
		if(st.eat() != TT::LSquare)
			error(st.ploc(), "expected '[' after instruction requiring offset");

		if(st.front() != TT::Number || st.front().text.find(".") != std::string::npos)
			error(st, "expected integer to specify offset size");

		int offsetSize = std::stoi(st.eat().str());
		if(offsetSize <= 0)
			error(st.ploc(), "offset must be a positive integer ('%d' is invalid)", offsetSize);

		if(st.eat() != TT::RSquare)
			error(st.ploc(), "expected ']' after offset size");

		return offsetSize;
	}

	static int parseStackFetch(State& st)
	{
		if(st.eat() != TT::Dollar)
			error(st.ploc(), "expected stack-offset value (eg. $3) for argument to 'dupe' or 'yank'");

		if(st.front() != TT::Number)
			error(st, "expected integer in stack-offset value");

		int ofs = std::stoi(st.eat().str());
		if(ofs < 0)
			error(st.ploc(), "stack-offset must be >= 0");

		return ofs;
	}

	static std::pair<int, bool> parseArg(State& st)
	{
		if(st.front() == TT::Dollar)
		{
			return { parseStackFetch(st), true };
		}
		else if(st.front() == TT::Number)
		{
			return { std::stoi(st.eat().str()), false };
		}
		else if(st.front() == TT::CharLiteral)
		{
			auto s = st.eat().str();
			assert(s.size() == 1);

			return { (int) s[0], false };
		}
		else
		{
			error(st, "unsupported operand");
			return { 0, false };
		}
	}

	static Stmt parseInstruction(State& st)
	{
		auto id = st.eat().str();

		auto ret = Stmt();
		ret.loc = st.ploc();

		if(id == "add")         ret.type = StmtTy::Add;
		else if(id == "halt")   ret.type = StmtTy::Halt;
		else if(id == "call")   ret.type = StmtTy::Call;
		else if(id == "drop")   ret.type = StmtTy::Drop;
		else if(id == "load")   ret.type = StmtTy::Load;
		else if(id == "fetch")  ret.type = StmtTy::Dupe;
		else if(id == "jmp")    ret.type = StmtTy::Jmp;
		else if(id == "yank")   ret.type = StmtTy::Yank;
		else if(id == "printi") ret.type = StmtTy::PrintInt;
		else if(id == "cmp")    ret.type = StmtTy::Compare;
		else if(id == "store")  ret.type = StmtTy::Store;
		else if(id == "mul")    ret.type = StmtTy::Multiply;
		else if(id == "printc") ret.type = StmtTy::PrintChar;
		else if(id == "ret")    ret.type = StmtTy::Return;
		else if(id == "sub")    ret.type = StmtTy::Subtract;
		else if(id == "div")    ret.type = StmtTy::Divide;
		else if(id == "jz")     ret.type = StmtTy::JmpIfZero;
		else if(id == "push")   ret.type = StmtTy::Push;
		else if(id == "jif")    ret.type = StmtTy::JmpIfCondition;
		else                    error(st.ploc(), "unexpected identifier '%s'", id);


		// if there's no argument after this, we're done
		if(auto f = st.front(); f == TT::Newline || f == TT::Semicolon)
		{
			if(id == "jif")
				error(st, "'jif' instruction cannot be standalone");

			ret.hasNoArguments = true;
			return ret;
		}

		ret.hasNoArguments = false;
		switch(ret.type)
		{
			case StmtTy::Add:
			case StmtTy::Compare:
			case StmtTy::Store:
			case StmtTy::Multiply:
			case StmtTy::Subtract:
			case StmtTy::Divide: {

				// we should always have 1 arg if "hasNoArguments" is false
				std::tie(ret.twoarg.arg1.value, ret.twoarg.arg1.isStackOfs) = parseArg(st);

				if(st.front() == TT::Comma)
				{
					st.pop();
					std::tie(ret.twoarg.arg2.value, ret.twoarg.arg2.isStackOfs) = parseArg(st);
					ret.twoarg.numargs = 2;
				}
				else
				{
					ret.twoarg.numargs = 1;
				}

			} break;

			case StmtTy::Load:
			case StmtTy::PrintInt:
			case StmtTy::PrintChar: {

				std::tie(ret.onearg.value, ret.onearg.isStackOfs) = parseArg(st);

			} break;

			case StmtTy::Dupe:
			case StmtTy::Yank: {

				ret.dupe.offset = parseStackFetch(st);

			} break;


			case StmtTy::Push: {

				// expect a number or a character
				ret.push.isComplexExpr = false;

				if(st.front() == TT::LSquare)
					ret.push.maxsize = parseMaxSize(st);

				if(st.front() == TT::Number)
				{
					ret.push.value = std::stoi(st.eat().str());
				}
				else if(st.front() == TT::CharLiteral)
				{
					auto s = st.eat().str();
					assert(s.size() == 1);

					ret.push.value = (int) s[0];
				}
				else if(st.front() == TT::LParen)
				{
					// ok expect some 'complicated' expression.
					// everything must be a constant (or a label), and only 8 operators: +, -, *, /, %, &, |, ^.


					auto getoperand = [&st](int max) -> std::string {

						if(st.front() == TT::Dollar)
						{
							if(max == 0)
								error(st, "push-expressions involving labels must specify a maximum size (eg. push[10])");

							st.eat();
							if(st.front() != TT::Identifier && st.front() != TT::Number)
							{
								error(st, "invalid dollar-expression");
							}
							else if(st.front() == TT::Number && st.lookahead(1) != TT::Identifier)
							{
								error(st, "stack references ($0, $1, etc.) are not constants, and cannot be used in push-expressions");
							}

							if(st.front() == TT::Identifier)
							{
								return "$" + st.eat().str();
							}
							else
							{
								auto s = st.eat().str();
								assert(st.front() == TT::Identifier);
								if(st.front().str() != "b" && st.front().str() != "f")
									error(st, "expected either 'b' or 'f' for local label reference");

								return "$" + s + st.eat().str();
							}
						}
						else if(st.front() == TT::Number)
						{
							return st.eat().str();
						}
						else
						{
							error(st, "expected either label reference ($foo) or constant number in push-expression");
						}
					};

					auto prec = [](const std::string& s) -> int {

						switch(s[0])
						{
							case '*':
							case '/':
							case '%': return 5;

							case '+':
							case '-': return 4;

							case '&': return 3;
							case '^': return 2;
							case '|': return 1;

							default: return -1;
						}
					};


					ret.push.isComplexExpr = true;

					// ok time for some yard shunting
					std::vector<std::string> queue;
					std::vector<std::string> operators;

					int parenDepth = 0;
					while(st.hasTokens())
					{
						if(st.front() == TT::Number || st.front() == TT::Dollar)
						{
							queue.push_back(getoperand(ret.push.maxsize));
						}
						else if(st.front() == TT::LParen)
						{
							parenDepth++;
							operators.push_back("(");

							st.eat();
						}
						else if(auto p = prec(st.front().str()); p > 0)
						{
							while(!operators.empty() && prec(operators.back()) >= p)
							{
								queue.push_back(operators.back());
								operators.pop_back();
							}

							operators.push_back(st.eat().str());
						}
						else if(st.front() == TT::RParen)
						{
							while(true)
							{
								if(operators.empty())
									error(st, "mismatched parentheses");

								if(operators.back() != "(")
								{
									queue.push_back(operators.back());
									operators.pop_back();
								}
								else
								{
									break;
								}
							}

							assert(operators.back() == "(");
							operators.pop_back();

							parenDepth--;

							if(parenDepth == 0)
								break;

							st.eat();
						}
					}

					assert(st.front() == TT::RParen);
					st.eat();

					ret.push.rpn = queue;
				}
				else
				{
					error(st, "expected either number or character literal after 'push'");
				}
			} break;


			case StmtTy::Jmp:
			case StmtTy::JmpIfZero:
			case StmtTy::JmpIfCondition:
			case StmtTy::Call: {

				// get the offset size.
				ret.jump.offsetSize = parseMaxSize(st);

				// if condition, we need to parse it here.
				if(ret.type == StmtTy::JmpIfCondition)
				{
					ret.jump.isJif = true;

					std::tie(ret.jump.op1.value, ret.jump.op1.isStackOfs)= parseArg(st);

					switch(st.eat().type)
					{
						case TT::EqualsTo:          ret.jump.gizmo = "J"; break;
						case TT::NotEquals:         ret.jump.gizmo = "SdZabGb"; break;
						case TT::LAngle:            ret.jump.gizmo = "JbA"; break;
						case TT::RAngle:            ret.jump.gizmo = "JbS"; break;
						case TT::LessThanEquals:    ret.jump.gizmo = "JbSdZabGb"; break;
						case TT::GreaterEquals:     ret.jump.gizmo = "JbAdZabGb"; break;
						default:                    error(st.ploc(), "invalid operator");
					}

					std::tie(ret.jump.op2.value, ret.jump.op2.isStackOfs)= parseArg(st);

					if(st.eat() != TT::Comma)
						error(st.ploc(), "expected ',' after condition in 'jif'");
				}


				// get the target.
				if(st.front() == TT::Identifier)
				{
					ret.jump.target = st.eat().str();
					ret.jump.isLocalLabel = false;
				}
				else if(st.front() == TT::Number)
				{
					auto tgt = st.eat().str();
					if(tgt.find("x") != -1 || tgt.find("-") != -1)
						error(st.ploc(), "invalid target for local label");

					ret.jump.target = tgt;
					ret.jump.isLocalLabel = true;

					if(st.front() != TT::Identifier || (st.front().str() != "f" && st.front().str() != "b"))
						error(st, "expected either 'f' or 'b' for local label target in instruction requiring offset");

					if(st.eat().str() == "f")
						ret.jump.localIsForward = true;

					else
						ret.jump.localIsForward = false;
				}
				else
				{
					error(st, "expected jump target");
				}
			} break;
		}

		return ret;
	}

	static Stmt parseLabel(State& st)
	{
		auto ret = Stmt();
		ret.loc = st.loc();
		ret.type = StmtTy::Label;

		ret.label.name = st.front().str();

		if(st.front() == TT::Number)
		{
		    if(ret.label.name.find("x") != -1 || ret.label.name.find("-") != -1)
				error(st.ploc(), "invalid local label name '%s'", ret.label.name);

			ret.label.isLocalLabel = true;
		}

		st.pop();
		if(st.eat() != TT::Colon)
			error(st.ploc(), "expected colon after label");

		return ret;
	}

	static Stmt parseStmt(State& st);
	static Stmt parseDirective(State& st)
	{
		st.eat();
		if(st.front() != TT::Identifier)
			error(st, "expected identifer in #directive");

		auto ret = Stmt();
		auto dir = st.eat().str();

		if(dir == "raw")
		{
			ret.type = StmtTy::Raw;
			if(st.front() != TT::StringLiteral)
				error(st, "expected string literal after #raw directive");

			ret.raw.insert = st.eat().str();
		}
		else if(dir == "string" || dir == "string_rev")
		{
			ret.type = StmtTy::String;
			if(st.front() != TT::StringLiteral)
				error(st, "expected string literal after #string directive");

			ret.string.s = st.eat().str();
			ret.string.reversed = (dir == "string_rev");
		}
		else if(dir == "repeat")
		{
			ret.type = StmtTy::Repeat;
			if(st.front() != TT::Number)
				error(st, "expected integer number of times to repeat");

			ret.repeat.times = std::stoi(st.eat().str());
			if(ret.repeat.times < 0)
				error(st, "number of repetitions should be a positive integer");

			// parse a braced block.
			if(st.eat() != TT::LBrace)
				error(st, "expected '{' to specify instructions to repeat");

			while(st.hasTokens() && st.frontAfterWS() != TT::RBrace)
				ret.repeat.stmts.push_back(parseStmt(st));

			if(st.eat() != TT::RBrace)
				error(st, "expected terminating '}'");
		}
		else
		{
			error(st.ploc(), "unknown directive '%s'", dir);
		}

		return ret;
	}

	static Stmt parseStmt(State& st)
	{
		if(!st.hasTokens())
			error(st, "unexpected end of file");

		st.skipWS();

		auto tok = st.front();
		if(tok != TT::EndOfFile)
		{
			if(tok == TT::Identifier)
			{
				if(st.lookahead(1) == TT::Colon)
					return parseLabel(st);

				else
					return parseInstruction(st);
			}
			else if(tok == TT::Number && st.lookahead(1) == TT::Colon)
			{
				return parseLabel(st);
			}
			else if(tok == TT::Pound)
			{
				return parseDirective(st);
			}
			else
			{
				error(st, "unexpected token '%s'", tok.str());
			}
		}

		error(st, "unexpected end of file");
	}


	std::vector<Stmt> parse(State& st)
	{
		std::vector<Stmt> stmts;
		while(st.hasTokens() && st.front() != TT::EndOfFile)
		{
			if(st.front() == TT::Newline || st.front() == TT::Comment)
			{
				// do nothing.
			}
			else
			{
				auto s = parseStmt(st);
				stmts.push_back(s);

				// labels can be on the same line
				if(s.type != StmtTy::Label)
				{
					if(st.front() != TT::Newline && st.front() != TT::Semicolon && st.front() != TT::Comment)
						error(st, "expected newline or semicolon to terminate statements");

					st.pop();
				}
			}

			st.skipWS();
		}

		return stmts;
	}
}





















