// defs.h
// Copyright (c) 2019, zhiayang
// Licensed under the Apache License Version 2.0

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

#include <vector>
#include <string>
#include <string_view>


#include "tinyformat.h"

namespace util
{
	inline std::string to_string(const std::string_view& sv)
	{
		return std::string(sv.data(), sv.length());
	}
}



struct Location
{
	size_t fileID = 0;
	size_t line = 0;
	size_t col = 0;
	size_t len = 0;

	bool operator == (const Location& other) const
	{
		return this->col == other.col && this->line == other.line && this->len == other.len && this->fileID == other.fileID;
	}

	bool operator != (const Location& other) const
	{
		return !(*this == other);
	}

	std::string toString() const;
	std::string shortString() const;
};



[[noreturn]] void doTheExit(bool trace = true);

std::string __error_gen_internal(const Location& loc, const std::string& msg, const char* type, bool context);

template <typename... Ts>
std::string strprintf(const char* fmt, Ts... ts)
{
	return tinyformat::format(fmt, ts...);
}

template <typename... Ts>
[[noreturn]] inline void _error_and_exit(const char* s, Ts... ts)
{
	tinyformat::format(std::cerr, s, ts...);
	doTheExit();
}

template <typename... Ts>
std::string __error_gen(const Location& loc, const char* msg, const char* type, bool, Ts... ts)
{
	return __error_gen_internal(loc, tinyformat::format(msg, ts...), type, true);
}

#define ERROR_FUNCTION(name, type, attr, doexit)                                                            \
template <typename... Ts> [[attr]] void name (const char* fmt, Ts... ts)                                    \
{ fputs(__error_gen(Location(), fmt, type, doexit, ts...).c_str(), stderr); if(doexit) doTheExit(false); }  \
																											\
template <typename... Ts> [[attr]] void name (const Location& loc, const char* fmt, Ts... ts)               \
{ fputs(__error_gen(loc, fmt, type, doexit, ts...).c_str(), stderr); if(doexit) doTheExit(false); }

ERROR_FUNCTION(error, "error", noreturn, true);
ERROR_FUNCTION(info, "note", maybe_unused, false);
ERROR_FUNCTION(warn, "warning", maybe_unused, false);
ERROR_FUNCTION(exitless_error, "exitless_error", maybe_unused, false);






namespace lexer
{
	enum class TokenType
	{
		Invalid,


		LParen,
		RParen,
		LAngle,
		RAngle,
		LBrace,
		RBrace,
		LSquare,
		RSquare,

		Plus,
		Minus,
		Asterisk,
		Slash,
		Percent,
		Comma,
		Period,
		Dollar,
		SQuote,
		DQuote,
		Colon,
		Equal,
		Question,
		Exclamation,
		Pipe,
		Tilde,
		Caret,
		Pound,
		Semicolon,
		Ampersand,

		// ==, !=, <=, >=
		EqualsTo,
		NotEquals,
		LessThanEquals,
		GreaterEquals,

		Newline,
		Comment,
		EndOfFile,

		StringLiteral,
		CharLiteral,
		Number,

		Identifier
	};

	struct Token
	{
		Location loc;
		TokenType type = TokenType::Invalid;
		std::string_view text;

		operator TokenType() const { return this->type; }
		bool operator == (const std::string& s) { return this->str() == s; }
		std::string str() const { return util::to_string(this->text); }
	};

	TokenType getNextToken(const std::vector<std::string_view>& lines, size_t* line, size_t* offset,
		const std::string_view& whole, Location& pos, Token* out, bool crlf);
}


enum class StmtTy
{
	Invalid,

	Label,
	Raw,
	String,
	Repeat,

	// keywords, so to speak
	Add,        // A
	Halt,       // B
	Call,       // C
	Drop,       // D
	Load,       // E
	Dupe,       // F
	Jmp,        // G
	Yank,       // H
	PrintInt,   // I
	Compare,    // J
	Store,      // K
	Multiply,   // M
	PrintChar,  // P
	Return,     // R
	Subtract,   // S
	Divide,     // V
	JmpIfZero,  // Z

	Push,       // doesn't really correspond to anything
	JmpIfCondition, // "jif" meta-instruction
};

struct Stmt
{
	Location loc;

	StmtTy type = StmtTy::Invalid;
	bool hasNoArguments = false;

	struct anArgument
	{
		int value = 0;
		bool isStackOfs = false;
	};

	// we would put these in a union, but we can't put non-POD types inside unions...
	struct {
		int value = 0;

		bool isComplexExpr = false;
		int maxsize = 0;
		std::vector<std::string> rpn;
	} push;

	struct {
		int offsetSize = 0;
		std::string target;

		bool isLocalLabel = false;
		bool localIsForward = false;

		// for jif
		bool isJif = false;
		anArgument op1;
		anArgument op2;
		std::string gizmo;
	} jump;

	struct {
		std::string name;
		bool isLocalLabel = false;
	} label;

	struct {
		int offset = 0;
	} dupe;

	anArgument onearg;

	struct {
		anArgument arg1;
		anArgument arg2;
		int numargs = 0;
	} twoarg;

	struct {
		std::string insert;
	} raw;

	struct {
		std::string s;
		bool reversed = false;
	} string;

	struct {
		int times = 0;
		std::vector<Stmt> stmts;
	} repeat;
};


namespace parser
{

	struct State
	{
		const lexer::Token& lookahead(size_t num) const
		{
			if(this->index + num < this->tokens.size())
				return this->tokens[this->index + num];

			else
				error("lookahead %zu tokens > size %zu", num, this->tokens.size());
		}

		void skip(size_t num)
		{
			if(this->index + num < this->tokens.size())
				this->index += num;

			else
				error("skip %zu tokens > size %zu", num, this->tokens.size());
		}

		void rewind(size_t num)
		{
			if(this->index > num)
				this->index -= num;

			else
				error("rewind %zu tokens > index %zu", num, this->index);
		}

		void rewindTo(size_t ix)
		{
			if(ix >= this->tokens.size())
				error("ix %zu > size %zu", ix, this->tokens.size());

			this->index = ix;
		}

		const lexer::Token& pop()
		{
			if(this->index + 1 < this->tokens.size())
				return this->tokens[this->index++];

			else
				error("pop() on last token");
		}

		const lexer::Token& eat()
		{
			this->skipWS();
			if(this->index + 1 < this->tokens.size())
				return this->tokens[this->index++];

			else
				error("eat() on last token");
		}

		void skipWS()
		{
			while(this->tokens[this->index] == lexer::TokenType::Newline || this->tokens[this->index] == lexer::TokenType::Comment)
				this->index++;
		}

		const lexer::Token& front() const
		{
			return this->tokens[this->index];
		}


		const lexer::Token& frontAfterWS()
		{
			size_t ofs = 0;
			while(this->tokens[this->index + ofs] == lexer::TokenType::Newline
				|| this->tokens[this->index + ofs] == lexer::TokenType::Comment)
			{
				ofs++;
			}
			return this->tokens[this->index + ofs];
		}

		const lexer::Token& prev()
		{
			return this->tokens[this->index - 1];
		}

		const Location& loc() const
		{
			return this->front().loc;
		}

		const Location& ploc() const
		{
			assert(this->index > 0);
			return this->tokens[this->index - 1].loc;
		}

		size_t remaining() const
		{
			return this->tokens.size() - this->index - 1;
		}

		size_t getIndex() const
		{
			return this->index;
		}

		void setIndex(size_t i)
		{
			assert(i < tokens.size());
			this->index = i;
		}

		bool frontIsWS() const
		{
			return this->front() == lexer::TokenType::Comment || this->front() == lexer::TokenType::Newline;
		}

		bool hasTokens() const
		{
			return this->index < this->tokens.size();
		}

		// implicitly coerce to location, so we can
		// error(st, ...)
		operator const Location&() const
		{
			return this->loc();
		}

		State(const std::vector<lexer::Token>& ts) : tokens(ts) { }

		size_t index = 0;
		const std::vector<lexer::Token>& tokens;
	};

	std::vector<Stmt> parse(State& st);
}


namespace codegen
{
	std::string assemble(const std::vector<Stmt>& stmts);
}






namespace frontend
{
	std::string getPathFromFile(const std::string& path);
	std::string getFilenameFromPath(const std::string& path);
	std::string getFullPathOfFile(const std::string& partial);
	std::string removeExtensionFromFilename(const std::string& name);

	std::string getFileContents(const std::string& fullPath);
	const std::string& getFilenameFromID(size_t fileID);
	size_t getFileIDFromFilename(const std::string& name);
	const std::vector<lexer::Token>& getFileTokens(const std::string& fullPath);
	const std::vector<std::string_view>& getFileLines(size_t id);
}















// defer implementation
// credit: gingerBill
// shamelessly stolen from https://github.com/gingerBill/gb


namespace __dontlook
{
	// NOTE(bill): Stupid fucking templates
	template <typename T> struct gbRemoveReference       { typedef T Type; };
	template <typename T> struct gbRemoveReference<T &>  { typedef T Type; };
	template <typename T> struct gbRemoveReference<T &&> { typedef T Type; };

	/// NOTE(bill): "Move" semantics - invented because the C++ committee are idiots (as a collective not as indiviuals (well a least some aren't))
	template <typename T> inline T &&gb_forward(typename gbRemoveReference<T>::Type &t)  { return static_cast<T &&>(t); }
	template <typename T> inline T &&gb_forward(typename gbRemoveReference<T>::Type &&t) { return static_cast<T &&>(t); }
	template <typename T> inline T &&gb_move   (T &&t)                                   { return static_cast<typename gbRemoveReference<T>::Type &&>(t); }
	template <typename F>
	struct gbprivDefer {
		F f;
		gbprivDefer(F &&f) : f(gb_forward<F>(f)) {}
		~gbprivDefer() { f(); }
	};
	template <typename F> gbprivDefer<F> gb__defer_func(F &&f) { return gbprivDefer<F>(gb_forward<F>(f)); }
}

#define GB_DEFER_1(x, y) x##y
#define GB_DEFER_2(x, y) GB_DEFER_1(x, y)
#define GB_DEFER_3(x)    GB_DEFER_2(x, __COUNTER__)
#define defer(code) auto GB_DEFER_3(_defer_) = __dontlook::gb__defer_func([&]()->void{code;})

