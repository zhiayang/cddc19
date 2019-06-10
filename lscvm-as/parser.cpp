// parser.cpp
// Copyright (c) 2019, zhiayang
// Licensed under the Apache License Version 2.0.

#include "defs.h"

namespace parser
{
	using TT = lexer::TokenType;

	static Instruction parseIdent(State& st)
	{
		warn(st, "found token %s\n", st.eat().str());
		return Instruction();
	}

	static Instruction parseStmt(State& st)
	{
		if(!st.hasTokens())
			error(st, "unexpected end of file");

		st.skipWS();

		auto tok = st.front();
		if(tok != TT::EndOfFile)
		{
			switch(tok.type)
			{
				case TT::Identifier:
					return parseIdent(st);

				default:
					error(st, "unexpected token '%s'", tok.str());
			}
		}

		error(st, "unexpected end of file");
	}







	void parse(State& st)
	{
		while(st.hasTokens() && st.front() != TT::EndOfFile)
		{
			switch(st.front())
			{
				case TT::Comment:
				case TT::Newline:
					st.skipWS();
					continue;

				default:
					st.stmts.push_back(parseStmt(st));
					break;
			}
			st.skipWS();
		}
	}
}
