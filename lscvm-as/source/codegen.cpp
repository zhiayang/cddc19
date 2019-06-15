// codegen.cpp
// Copyright (c) 2019, zhiayang
// Licensed under the Apache License Version 2.0.

#include <unordered_map>

#include "defs.h"

namespace codegen
{
	struct State
	{
		size_t idx = 0;
		int currentOfs = 0;
		std::vector<Stmt> stmts;
		std::unordered_map<std::string, std::pair<int, Location>> namedLabelOffsets;
	};

	static std::string convert(int num, int maximum = 0);

	static int sizearg(const Stmt::anArgument& arg)
	{
		// extra plus 1 is for the "F".
		return convert(arg.value).size() + (arg.isStackOfs ? 1 : 0);
	}

	static int size(const Stmt& stmt)
	{
		if(stmt.type != StmtTy::Invalid && stmt.type != StmtTy::Label && stmt.hasNoArguments)
			return 1;

		switch(stmt.type)
		{
			case StmtTy::Add:
			case StmtTy::Subtract:
			case StmtTy::Divide:
			case StmtTy::Compare:
			case StmtTy::Store:
			case StmtTy::Multiply: {
				int ret = sizearg(stmt.twoarg.arg1);
				if(stmt.twoarg.numargs == 2)
					ret += sizearg(stmt.twoarg.arg2);

				return ret + 1;
			}

			case StmtTy::Load:
			case StmtTy::PrintInt:
			case StmtTy::PrintChar:
				return sizearg(stmt.onearg) + 1;

			case StmtTy::Dupe:
			case StmtTy::Yank:
				return 1 + convert(stmt.dupe.offset).size();

			case StmtTy::Push:
				if(stmt.push.isComplexExpr) return stmt.push.maxsize;
				else                        return convert(stmt.push.value).size();

			case StmtTy::Call:
			case StmtTy::Jmp:
			case StmtTy::JmpIfZero:
				return 1 + stmt.jump.offsetSize;

			case StmtTy::JmpIfCondition:
				return 1 + stmt.jump.offsetSize + sizearg(stmt.jump.op1) + sizearg(stmt.jump.op2) + stmt.jump.gizmo.size();

			case StmtTy::Halt:
			case StmtTy::Drop:
			case StmtTy::Return:
				return 1;

			case StmtTy::Label:
				return 0;

			case StmtTy::Raw:
				return stmt.raw.insert.size();

			case StmtTy::String: {
				int sz = 0;
				for(auto c : stmt.string.s)
					sz += convert(c).size();

				return sz;
			}

			case StmtTy::Repeat: {
				int sz = 0;
				for(const auto& s : stmt.repeat.stmts)
					sz += size(s);

				return sz * stmt.repeat.times;
			}

			default:
				assert(!"yo, wtf?!");
				return 0;
		}
	}

	static int absoluteOfIdx(State* st, int idx)
	{
		assert(idx < st->stmts.size() && idx >= 0);

		int pos = 0;
		for(int i = 0; i < idx; i++)
			pos += size(st->stmts[i]);

		return pos;
	}


	static int find(State* st, const std::string& target, bool local, int dir, int* idx_out = 0)
	{
		if(local)
		{
			// don't bother searching the current instruction
			int idx = st->idx;
			int ofs = 0;

			while(idx != 0 && idx != (st->stmts.size() - 1))
			{
				const auto& s = st->stmts[idx];

				// when going backwards, we need to account for the size of the
				// current instruction -- so subtract the offset first.
				if(dir == -1) ofs -= size(s);

				if(s.type == StmtTy::Label && s.label.isLocalLabel && s.label.name == target)
					return ofs;

				// when going forwards, do it normally
				if(dir == 1) ofs += size(s);

				idx += dir;

				if(idx_out)
					*idx_out = idx;
			}

			error(st->stmts[st->idx].loc, "no local label '%s' (searching %s)", target, dir == 1 ? "forwards" : "backwards");
		}
		else
		{
			if(auto it = st->namedLabelOffsets.find(target); it != st->namedLabelOffsets.end())
				return it->second.first;

			else
				error(st->stmts[st->idx].loc, "unknown jump target '%s'", target);
		}
	}


	static std::string generatearg(const Stmt::anArgument& arg)
	{
		auto ret = convert(arg.value);
		if(arg.isStackOfs) ret += "F";

		return ret;
	}

	static std::string generate(State* st, const Stmt& s)
	{
		switch(s.type)
		{

			case StmtTy::Store:
			case StmtTy::Compare:
			case StmtTy::Add:
			case StmtTy::Multiply:
			case StmtTy::Subtract:
			case StmtTy::Divide: {

				std::string a1;
				std::string a2;

				if(!s.hasNoArguments)
				{
					// always have at least one
					a1 = generatearg(s.twoarg.arg1);

					if(s.twoarg.numargs == 2)
						a2 = generatearg(s.twoarg.arg2);
				}

				if(s.type == StmtTy::Store)         return a1 + a2 + "K";
				else if(s.type == StmtTy::Compare)  return a1 + a2 + "J";
				else if(s.type == StmtTy::Add)      return a1 + a2 + "A";
				else if(s.type == StmtTy::Multiply) return a1 + a2 + "M";
				else if(s.type == StmtTy::Subtract) return a1 + a2 + "S";
				else if(s.type == StmtTy::Divide)   return a1 + a2 + "V";
				else assert(false);
			}


			case StmtTy::Load:
			case StmtTy::PrintInt:
			case StmtTy::PrintChar: {

				std::string num;

				if(!s.hasNoArguments)
				{
					num = generatearg(s.onearg);
				}

				if(s.type == StmtTy::PrintInt)          return num + "I";
				else if(s.type == StmtTy::PrintChar)    return num + "P";
				else if(s.type == StmtTy::Load)         return num + "E";
				else assert(false);
			}


			case StmtTy::Dupe:
			case StmtTy::Yank: {

				std::string ofs;
				if(!s.hasNoArguments)
					ofs = convert(s.dupe.offset);

				if(s.type == StmtTy::Dupe) return ofs + "F";
				else                       return ofs + "H";
			}

			case StmtTy::Call: {

				std::string tgt;

				if(!s.hasNoArguments)
				{
					auto targ = find(st, s.jump.target, /* local: */ false, 0);

					tgt = convert(targ, s.jump.offsetSize);
					if(tgt.size() > s.jump.offsetSize)
					{
						error(s.loc, "call to target '%s' requires offset %d (%s), which cannot fit into %d byte(s)",
							s.jump.target, targ, tgt, s.jump.offsetSize);
					}
				}

				return tgt + "C";
			}

			case StmtTy::Jmp:
			case StmtTy::JmpIfZero:
			case StmtTy::JmpIfCondition: {

				std::string str;

				if(s.type == StmtTy::JmpIfCondition)
				{
					str += generatearg(s.jump.op1);
					str += generatearg(s.jump.op2);
					str += s.jump.gizmo;
				}

				if(!s.hasNoArguments)
				{
					// get the target first.
					int ofs = 0;
					if(s.jump.isLocalLabel)
					{
						// for local=true, find() accounts for everything including the size of the current instruction
						ofs = find(st, s.jump.target, /* local: */ true, s.jump.localIsForward ? 1 : -1);
					}
					else
					{
						// for local=false, we do some calculations ourselves.
						auto tgt = find(st, s.jump.target, /* local: */ false, 0);

						if(tgt < st->currentOfs)
							ofs = tgt - (st->currentOfs + size(s));

						else
							ofs = tgt - (st->currentOfs + size(s));

						// printf("ofs: %d, tgt: %d, curofs: %d, sz: %d\n", ofs, tgt, st->currentOfs, size(s));
					}

					auto strofs = convert(ofs, s.jump.offsetSize);
					if(strofs.size() > s.jump.offsetSize)
					{
						error(s.loc, "jump to target '%s' requires offset %d (%s), which cannot fit into %d byte(s)",
							s.jump.target, ofs, strofs, s.jump.offsetSize);
					}

					str += strofs;
				}

				if(s.type == StmtTy::Jmp) return str + "G";
				else                      return str + "Z";
			}


			case StmtTy::Halt:      return "B";
			case StmtTy::Drop:      return "D";
			case StmtTy::Return:    return "R";

			case StmtTy::Push: {

				if(!s.push.isComplexExpr)
				{
					return convert(s.push.value, s.push.maxsize);
				}
				else
				{
					// $ indicates a label expression.
					auto getop = [&st](const std::string& op) -> int {

						assert(!op.empty());

						if(op[0] == '$')
						{
							if(isdigit(op[1]))
							{
								// local. strip the $ and the f/b
								std::string targ = op.substr(1, op.length() - 2);
								bool isFwd = (op.back() == 'f');

								int idx = 0;
								find(st, targ, /* local: */ true, /* dir: */ isFwd ? 1 : -1, &idx);

								return absoluteOfIdx(st, idx);
							}
							else
							{
								// global. strip the $
								std::string targ = op.substr(1, op.length() - 1);
								return find(st, targ, /* local: */ false, /* dir: */ 0);
							}
						}
						else
						{
							return std::stoi(op);
						}
					};

					std::vector<int> stack;
					auto pop = [&stack]() -> int {
						auto ret = stack.back();
						stack.pop_back();
						return ret;
					};

					for(const auto& s : s.push.rpn)
					{
						int a = 0; int b = 0;

						switch(s[0])
						{
							case '+':   b = pop(), a = pop(), stack.push_back(a + b); break;
							case '-':   b = pop(), a = pop(), stack.push_back(a - b); break;
							case '*':   b = pop(), a = pop(), stack.push_back(a * b); break;
							case '/':   b = pop(), a = pop(), stack.push_back(a / b); break;
							case '%':   b = pop(), a = pop(), stack.push_back(a % b); break;
							case '&':   b = pop(), a = pop(), stack.push_back(a & b); break;
							case '|':   b = pop(), a = pop(), stack.push_back(a | b); break;
							case '^':   b = pop(), a = pop(), stack.push_back(a ^ b); break;
							default:    stack.push_back(getop(s));
						}
					}

					assert(stack.size() == 1);
					int result = stack.front();

					return convert(result, s.push.maxsize);
				}
			}

			case StmtTy::Label:
				return "";

			case StmtTy::Raw:
				return s.raw.insert;

			case StmtTy::String: {
				std::string ret;
				if(s.string.reversed)
				{
					for(auto it = s.string.s.rbegin(); it != s.string.s.rend(); it++)
						ret += convert(*it);
				}
				else
				{
					for(auto c : s.string.s)
						ret += convert(c);
				}

				return ret;
			}

			case StmtTy::Repeat: {
				std::string ret;
				for(int i = 0; i < s.repeat.times; i++)
					for(const auto& s : s.repeat.stmts)
						ret += generate(st, s);

				return ret;
			}

			default:
				assert(!"yo, wtf?!");
				return "";
		}
	}


	std::string assemble(const std::vector<Stmt>& stmts)
	{
		State st;
		st.stmts = stmts;

		{
			int currentofs = 0;

			for(const auto& s : stmts)
			{
				if(s.type == StmtTy::Label && !s.label.isLocalLabel)
				{
					if(auto it = st.namedLabelOffsets.find(s.label.name); it != st.namedLabelOffsets.end())
					{
						exitless_error(s.loc, "label '%s' was already defined", s.label.name);
						info(it->second.second, "previous definition was here:");
						doTheExit();
					}
					else
					{
						st.namedLabelOffsets[s.label.name] = { currentofs, s.loc };
					}
				}

				currentofs += size(s);
			}
		}


		std::stringstream ss;

		// ok, now we have a map of all the labels
		// just loop through the thing.
		for(const auto& s : stmts)
		{
			ss << generate(&st, s);

			st.idx++;
			st.currentOfs += size(s);
		}


		return ss.str();
	}













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

	static std::string convert(int num, int maximum)
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
		else
		{
			// if we got here, then there's nothing exactly cached. so, we try to see if +-9 is cached.
			// int best = INT_MAX;
			// for(int d = -9; d <= 9; d++)
			// {
			// 	if(auto it = cache.find(num + d); it != cache.end())
			// 	{
			// 		auto s = cache[num + d] + (char) ('a' + abs(d)) + std::string(d < 0 ? "A" : "S");
			// 		if(ret.empty() || s.size() < best)
			// 			ret = s, best = s.size();
			// 	}
			// }

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


		if(maximum > 0 && ret.size() < maximum)
		{
			return ret + std::string(maximum - ret.size(), '-');
		}
		else
		{
			return ret;
		}
	}
}


























