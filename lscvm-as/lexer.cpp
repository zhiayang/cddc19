// lexer.cpp
// Copyright (c) 2014 - 2015, zhiayang
// Licensed under the Apache License Version 2.0.


#include "defs.h"
#include <unordered_map>

using string_view = std::string_view;

constexpr int TAB_WIDTH = 4;

namespace lexer
{
	static void skipWhitespace(string_view& line, Location& pos, size_t* offset)
	{
		size_t skip = 0;
		while(line.length() > skip && (line[skip] == '\t' || line[skip] == ' '))
		{
			(line[skip] == ' ' ? pos.col++ : pos.col += TAB_WIDTH);
			skip++;
		}

		line.remove_prefix(skip);
		(*offset) += skip;
	}

	template <size_t N>
	static bool hasPrefix(const string_view& str, char const (&literal)[N])
	{
		if(str.length() < N - 1) return false;
		for(size_t i = 0; i < N - 1; i++)
			if(str[i] != literal[i]) return false;

		return true;
	}

	template <size_t N>
	static bool compare(const string_view& str, char const (&literal)[N])
	{
		if(str.length() != N - 1) return false;
		for(size_t i = 0; i < N - 1; i++)
			if(str[i] != literal[i]) return false;

		return true;
	}



	static TokenType prevType = TokenType::Invalid;
	static size_t prevID = 0;
	static bool shouldConsiderUnaryLiteral(string_view& stream, Location& pos)
	{
		// unlike flax there's no expressions so this is always true
		bool should = (prevType != TokenType::Invalid && prevID == pos.fileID);

		if(!should) return false;

		// check if the current char is a + or -
		if(stream.length() == 0) return false;
		if(stream[0] != '+' && stream[0] != '-') return false;

		// check if there's only spaces between this and the number itself
		for(size_t i = 1; i < stream.length(); i++)
		{
			if(isdigit(stream[i])) return true;
			else if(stream[i] != ' ') return false;
		}

		return false;
	}





	TokenType getNextToken(const std::vector<string_view>& lines, size_t* line, size_t* offset, const string_view& whole,
		Location& pos, Token* out, bool crlf)
	{
		bool flag = true;

		if(*line == lines.size())
		{
			out->loc = pos;
			out->type = TokenType::EndOfFile;
			return TokenType::EndOfFile;
		}

		string_view stream = lines[*line].substr(*offset);
		if(stream.empty())
		{
			out->loc = pos;
			out->type = TokenType::EndOfFile;
			return TokenType::EndOfFile;
		}



		size_t read = 0;
		size_t unicodeLength = 0;

		// first eat all whitespace
		skipWhitespace(stream, pos, offset);

		Token& tok = *out;
		tok.loc = pos;
		tok.type = TokenType::Invalid;


		// check compound symbols first.
		if(hasPrefix(stream, "//"))
		{
			tok.type = TokenType::Comment;
			// stream = stream.substr(0, 0);
			(*line)++;
			pos.line++;
			pos.col = 0;


			(*offset) = 0;

			// don't assign lines[line] = stream, since over here we've changed 'line' to be the next one.
			flag = false;
			tok.text = "";
		}
		else if(hasPrefix(stream, "/*"))
		{
			int currentNest = 1;

			// support nested, so basically we have to loop until we find either a /* or a */
			stream.remove_prefix(2);
			(*offset) += 2;
			pos.col += 2;


			Location opening = pos;
			Location curpos = pos;

			size_t k = 0;
			while(currentNest > 0)
			{
				// we can do this, because we know the closing token (*/) is 2 chars long
				// so if we have 1 char left, gg.
				if(k + 1 == stream.size() || stream[k] == '\n')
				{
					if(*line + 1 == lines.size())
						error(opening, "expected closing */ (reached EOF), for block comment started here:");

					// else, get the next line.
					// also note: if we're in this loop, we're inside a block comment.
					// since the ending token cannot be split across lines, we know that this last char
					// must also be part of the comment. hence, just skip over it.

					k = 0;
					curpos.line++;
					curpos.col = 0;
					(*offset) = 0;
					(*line)++;

					stream = lines[*line];
					continue;
				}


				if(stream[k] == '/' && stream[k + 1] == '*')
					currentNest++, k++, curpos.col++, opening = curpos;

				else if(stream[k] == '*' && stream[k + 1] == '/')
					currentNest--, k++, curpos.col++;

				k++;
				curpos.col++;
			}


			if(currentNest != 0)
				error(opening, "expected closing */ (reached EOF), for block comment started here:");

			pos = curpos;

			// don't actually store the text, because it's pointless and memory-wasting
			// tok.text = "/* I used to be a comment like you, until I took a memory-leak to the knee. */";
			tok.type = TokenType::Comment;
			tok.text = "";
			read = k;
		}
		else if(hasPrefix(stream, "*/"))
		{
			error(tok.loc, "unexpected '*/'");
		}
		else if((!stream.empty() && ((stream[0] >= 1 && (int) stream[0] <= 255 && isdigit(stream[0])) || shouldConsiderUnaryLiteral(stream, pos)))
			/* handle cases like '+ 3' or '- 14' (ie. space between sign and number) */
			&& ((isdigit(stream[0]) ? true : false) || (stream.size() > 1 && isdigit(stream[1]))))
		{
			// copy it.
			auto tmp = stream;

			if(tmp.find('-') == 0 || tmp.find('+') == 0)
				tmp.remove_prefix(1);

			int base = 10;
			if(tmp.find("0x") == 0 || tmp.find("0X") == 0)
				base = 16, tmp.remove_prefix(2);

			else if(tmp.find("0b") == 0 || tmp.find("0B") == 0)
				base = 2, tmp.remove_prefix(2);

			// find that shit
			auto end = std::find_if_not(tmp.begin(), tmp.end(), [base](const char& c) -> bool {
				if(base == 10)	return isdigit(c);
				if(base == 16)	return isdigit(c) || (toupper(c) >= 'A' && toupper(c) <= 'F');
				else			return (c == '0' || c == '1');
			});

			tmp.remove_prefix((end - tmp.begin()));

			size_t didRead = stream.size() - tmp.size();
			tok.text = stream.substr(0, didRead);

			tok.type = TokenType::Number;
			tok.loc.len = didRead;

			read = didRead;
		}
		else if(!stream.empty() && (stream[0] == '_' || isalpha(stream[0]) > 0))
		{
			// get as many letters as possible first
			size_t identLength = 1;
			while(identLength < stream.size() && (isalnum(stream[identLength]) || stream[identLength] == '_'))
				identLength++;

			read = identLength;
			tok.text = stream.substr(0, identLength);

			tok.type = TokenType::Identifier;
		}
		else if(!stream.empty() && stream[0] == '"')
		{
			// string literal
			// because we want to avoid using std::string (ie. copying) in the lexer (Token), we must send the string over verbatim.

			// store the starting position
			size_t start = (size_t) (stream.data() - whole.data() + 1);

			// opening "
			pos.col++;

			size_t i = 1;
			size_t didRead = 0;
			for(; stream[i] != '"'; i++)
			{
				if(stream[i] == '\\')
				{
					if(i + 1 == stream.size())
					{
						error(pos, "unexpected end of input");
					}
					else if(stream[i + 1] == '"')
					{
						// add the quote and the backslash, and skip it.
						didRead += 2;
						pos.col += 2;
						i++;
					}
					// breaking string over two lines
					else if(stream[i + 1] == '\n')
					{
						// skip it, then move to the next line
						pos.line++;
						pos.col = 0;
						(*line)++;

						if(*line == lines.size())
							error(pos, "unexpected end of input");

						i = 0;

						// just a fudge factor gotten from empirical evidence
						// 3 extra holds for multiple lines, so all is well.

						didRead += 3;
						stream = lines[*line];
						(*offset) = 0;
					}
					else if(stream[i + 1] == '\\')
					{
						i++;
						didRead += 2;
						pos.col += 2;
					}
					else
					{
						// just put the backslash in.
						// and don't skip the next one.
						didRead++;
						pos.col++;
					}

					continue;
				}

				didRead++;
				pos.col++;

				if(i == stream.size() - 1 || stream[i] == '\n')
				{
					error(pos, "expected closing '\"' (%zu/%zu/%zu/%c/%s/%zu)", i, stream.size(), didRead,
						stream[i], util::to_string(stream), *offset);
				}
			}

			// closing "
			pos.col++;


			tok.type = TokenType::StringLiteral;
			tok.text = whole.substr(start, didRead);

			tok.loc.len = 2 + didRead;

			stream = stream.substr(i + 1);
			(*offset) += i + 1;

			read = 0;
			flag = false;
		}
		else if(crlf && hasPrefix(stream, "\r\n"))
		{
			read = 2;
			flag = false;

			tok.type = TokenType::Newline;
			tok.text = "\n";
		}
		else if(!stream.empty())
		{
			if(isascii(stream[0]))
			{
				// check the first char
				switch(stream[0])
				{
					// for single-char things
					case '\n':  tok.type = TokenType::Newline;      break;
					case '{':   tok.type = TokenType::LBrace;       break;
					case '}':   tok.type = TokenType::RBrace;       break;
					case '(':   tok.type = TokenType::LParen;       break;
					case ')':   tok.type = TokenType::RParen;       break;
					case '[':   tok.type = TokenType::LSquare;      break;
					case ']':   tok.type = TokenType::RSquare;      break;
					case '<':   tok.type = TokenType::LAngle;       break;
					case '>':   tok.type = TokenType::RAngle;       break;
					case '+':   tok.type = TokenType::Plus;         break;
					case '-':   tok.type = TokenType::Minus;        break;
					case '*':   tok.type = TokenType::Asterisk;     break;
					case '/':   tok.type = TokenType::Slash;        break;
					case '\'':  tok.type = TokenType::SQuote;       break;
					case '.':   tok.type = TokenType::Period;       break;
					case ',':   tok.type = TokenType::Comma;        break;
					case ':':   tok.type = TokenType::Colon;        break;
					case '=':   tok.type = TokenType::Equal;        break;
					case '?':   tok.type = TokenType::Question;     break;
					case '!':   tok.type = TokenType::Exclamation;  break;
					case ';':   tok.type = TokenType::Semicolon;    break;
					case '&':   tok.type = TokenType::Ampersand;    break;
					case '%':   tok.type = TokenType::Percent;      break;
					case '#':   tok.type = TokenType::Pound;        break;
					case '~':   tok.type = TokenType::Tilde;        break;
					case '^':   tok.type = TokenType::Caret;        break;
					case '$':   tok.type = TokenType::Dollar;       break;

					default:
						error(tok.loc, "unknown token '%c'", stream[0]);
				}

				tok.text = stream.substr(0, 1);
				// tok.loc.col += 1;
				read = 1;
			}
			else
			{
				error(tok.loc, "unknown token '%s'", util::to_string(stream.substr(0, 10)));
			}
		}

		stream.remove_prefix(read);

		if(flag)
			(*offset) += read;

		if(tok.type != TokenType::Newline)
		{
			if(read > 0)
			{
				// note(debatable): put the actual "position" in the front of the token
				pos.col += (unicodeLength > 0 ? unicodeLength : read);

				// special handling -- things like ƒ, ≤ etc. are one character wide, but can be several *bytes* long.
				pos.len = (unicodeLength > 0 ? unicodeLength : read);
				tok.loc.len = (unicodeLength > 0 ? unicodeLength : read);
			}
		}
		else
		{
			pos.col = 0;
			pos.line++;

			(*line)++;
			(*offset) = 0;
		}

		// debuglog("token %s: %d // %d\n", util::to_string(tok.text), tok.loc.col, pos.col);

		prevType = tok.type;
		prevID = tok.loc.fileID;

		return prevType;
	}
}
