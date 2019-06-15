// error.cpp
// Copyright (c) 2019, zhiayang
// Licensed under the Apache License Version 2.0.

#include <string.h>

#include "defs.h"

#define COLOUR_RESET            "\033[0m"
#define COLOUR_BLACK            "\033[30m"
#define COLOUR_RED              "\033[31m"
#define COLOUR_GREEN            "\033[32m"
#define COLOUR_YELLOW           "\033[33m"
#define COLOUR_BLUE             "\033[34m"
#define COLOUR_MAGENTA          "\033[35m"
#define COLOUR_CYAN             "\033[36m"
#define COLOUR_WHITE            "\033[37m"
#define COLOUR_BLACK_BOLD       "\033[1m"
#define COLOUR_RED_BOLD         "\033[1m\033[31m"
#define COLOUR_GREEN_BOLD       "\033[1m\033[32m"
#define COLOUR_YELLOW_BOLD      "\033[1m\033[33m"
#define COLOUR_BLUE_BOLD        "\033[1m\033[34m"
#define COLOUR_MAGENTA_BOLD     "\033[1m\033[35m"
#define COLOUR_CYAN_BOLD        "\033[1m\033[36m"
#define COLOUR_WHITE_BOLD       "\033[1m\033[37m"
#define COLOUR_GREY_BOLD        "\033[30;1m"

static std::string repeat(std::string str, size_t n)
{
	if(n == 0)
	{
		str.clear();
		str.shrink_to_fit();
		return str;
	}
	else if(n == 1 || str.empty())
	{
		return str;
	}

	auto period = str.size();
	if(period == 1)
	{
		str.append(n - 1, str.front());
		return str;
	}

	str.reserve(period * n);
	size_t m = 2;
	for(; m < n; m *= 2)
		str += str;

	str.append(str.c_str(), (n - (m / 2)) * period);

	return str;
}

static std::string spaces(size_t n)
{
	return repeat(" ", n);
}

#define TAB_WIDTH           4
#define LEFT_PADDING        TAB_WIDTH
#define UNDERLINE_CHARACTER "^"

struct ESpan
{
	ESpan() { }
	ESpan(const Location& l, const std::string& m) : loc(l), msg(m) { }

	Location loc;
	std::string msg;
	std::string colour;
};

static std::string fetchContextLine(Location loc, size_t* adjust)
{
	if(loc.fileID == 0) return "";

	auto lines = frontend::getFileLines(loc.fileID);
	if(lines.size() > loc.line)
	{
		std::string orig = util::to_string(lines[loc.line]);
		std::stringstream ln;

		// skip all leading whitespace.
		bool ws = true;
		for(auto c : orig)
		{
			if(ws && c == '\t')                 { *adjust += TAB_WIDTH; continue;   }
			else if(ws && c == ' ')             { *adjust += 1; continue;           }
			else if(c == '\n')                  { break;                            }
			else                                { ws = false; ln << c;              }
		}

		return strprintf("%s%s", spaces(LEFT_PADDING), ln.str().c_str());
	}

	return "";
}



std::string getSpannedContext(const Location& loc, const std::vector<ESpan>& spans, size_t* adjust, size_t* num_width, size_t* margin,
	bool underline, bool bottompad, std::string underlineColour)
{
	std::string ret;

	assert(adjust && margin && num_width);
	assert((underline == bottompad || bottompad) && "cannot underline without bottom pad");

	if(!std::is_sorted(spans.begin(), spans.end(), [](auto a, auto b) -> bool { return a.loc.col < b.loc.col; }))
		fprintf(stderr, "spans must be sorted!\n"), exit(-1);

	*num_width = std::to_string(loc.line + 1).length();

	// one spacing line
	*margin = *num_width + 2;
	ret += strprintf("%s |\n", spaces(*num_width));
	ret += strprintf("%d |%s\n", loc.line + 1, fetchContextLine(loc, adjust));

	if(bottompad)
		ret += strprintf("%s |", spaces(*num_width));

	// ok, now loop through each err, and draw the underline.
	if(underline)
	{
		//* cursor represents the 'virtual' position -- excluding the left margin
		size_t cursor = 0;

		// columns actually start at 1 for some reason.
		ret += spaces(LEFT_PADDING - 1);

		for(auto span : spans)
		{
			// pad out.
			auto tmp = strprintf("%s", spaces(1 + span.loc.col - *adjust - cursor)); cursor += tmp.length();
			ret += tmp + strprintf("%s", span.colour.empty() ? underlineColour : span.colour);

			tmp = strprintf("%s", repeat(UNDERLINE_CHARACTER, span.loc.len)); cursor += tmp.length();
			ret += tmp + strprintf("%s", COLOUR_RESET);
		}
	}

	return ret;
}

std::string getSingleContext(const Location& loc, bool underline = true, bool bottompad = true)
{
	if(loc.fileID == 0) return "";

	size_t a = 0;
	size_t b = 0;
	size_t c = 0;
	return getSpannedContext(loc, { ESpan(loc, "") }, &a, &b, &c, underline, bottompad, COLOUR_RED_BOLD);
}



std::string __error_gen_internal(const Location& loc, const std::string& msg, const char* type, bool context)
{
	std::string ret;

	auto colour = COLOUR_RED_BOLD;
	if(strcmp(type, "warning") == 0)
		colour = COLOUR_MAGENTA_BOLD;

	else if(strcmp(type, "note") == 0)
		colour = COLOUR_GREY_BOLD;

	// bool empty = strcmp(type, "") == 0;
	// bool dobold = strcmp(type, "note") != 0;

	//? do we want to truncate the file path?
	//? we're doing it now, might want to change (or use a flag)

	std::string filename = frontend::getFilenameFromPath(loc.fileID == 0 ? "(unknown)"
		: frontend::getFilenameFromID(loc.fileID));

	ret += strprintf("%s%s%s:%s %s\n", COLOUR_RESET, colour, type, COLOUR_RESET, msg);

	if(loc.fileID > 0)
	{
		auto location = strprintf("%s:%d:%d", filename, loc.line + 1, loc.col + 1);
		ret += strprintf("%s%sat:%s %s%s", COLOUR_RESET, COLOUR_GREY_BOLD, spaces(strlen(type) - 2), COLOUR_RESET, location);
	}

	ret += "\n";

	if(context && loc.fileID > 0)
		ret += getSingleContext(loc) + "\n";

	return ret;
}

[[noreturn]] void doTheExit(bool trace)
{
	fprintf(stderr, "\nthere were errors, compilation cannot continue\n");
	exit(-1);
}






