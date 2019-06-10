// frontend.cpp
// Copyright (c) 2019, zhiayang
// Licensed under the Apache License Version 2.0.

#include <map>

#include "defs.h"
#include "platform.h"

namespace frontend
{
	struct FileInnards
	{
		std::string fileContents;
		std::vector<lexer::Token> tokens;
		std::vector<std::string_view> lines;

		bool didLex = false;
	};

	static std::map<std::string, FileInnards> fileList;
	static FileInnards& readFileIfNecessary(const std::string& fullPath)
	{
		// break early if we can
		{
			auto it = fileList.find(fullPath);
			if(it != fileList.end() && it->second.didLex)
				return it->second;
		}

		std::string_view fileContents;
		fileContents = platform::readEntireFile(fullPath);

		// split into lines
		bool crlf = false;
		std::vector<std::string_view> rawlines;

		{
			std::string_view view = fileContents;

			bool first = true;
			while(true)
			{
				size_t ln = 0;

				if(first || crlf)
				{
					ln = view.find("\r\n");
					if(ln != std::string_view::npos && first)
						crlf = true;
				}

				if((!first && !crlf) || (first && !crlf && ln == std::string_view::npos))
					ln = view.find('\n');

				first = false;

				if(ln != std::string_view::npos)
				{
					rawlines.push_back(std::string_view(view.data(), ln + (crlf ? 2 : 1)));
					view.remove_prefix(ln + (crlf ? 2 : 1));
				}
				else
				{
					break;
				}
			}

			// account for the case when there's no trailing newline, and we still have some stuff stuck in the view.
			if(!view.empty())
			{
				rawlines.push_back(std::string_view(view.data(), view.length()));
			}
		}

		Location pos;
		FileInnards& innards = fileList[fullPath];
		{
			pos.fileID = getFileIDFromFilename(fullPath);

			innards.fileContents = std::move(fileContents);
			innards.lines = std::move(rawlines);
		}


		{
			size_t curLine = 0;
			size_t curOffset = 0;

			bool flag = true;
			size_t i = 0;

			do {
				lexer::Token out {};
				auto type = lexer::getNextToken(innards.lines, &curLine, &curOffset,
					std::string_view(innards.fileContents), pos, &out, crlf);

				flag = (type != lexer::TokenType::EndOfFile);
				if(type == lexer::TokenType::Invalid)
					error(pos, "invalid token");

				innards.tokens.push_back(out);
				i++;

			} while(flag);

			innards.tokens.back().loc.len = 0;
		}

		innards.didLex = true;
		return innards;
	}













	const std::vector<lexer::Token>& getFileTokens(const std::string& fullPath)
	{
		return readFileIfNecessary(fullPath).tokens;
	}

	std::string getFileContents(const std::string& fullPath)
	{
		return util::to_string(readFileIfNecessary(fullPath).fileContents);
	}


	static std::vector<std::string> fileNames { "null" };
	static std::map<std::string, size_t> existingNames;

	const std::string& getFilenameFromID(size_t fileID)
	{
		assert(fileID > 0 && fileID < fileNames.size());
		return fileNames[fileID];
	}

	size_t getFileIDFromFilename(const std::string& name)
	{
		if(existingNames.find(name) != existingNames.end())
		{
			return existingNames[name];
		}
		else
		{
			fileNames.push_back(name);
			existingNames[name] = fileNames.size() - 1;

			return fileNames.size() - 1;
		}
	}

	const std::vector<std::string_view>& getFileLines(size_t id)
	{
		std::string fp = getFilenameFromID(id);
		return readFileIfNecessary(fp).lines;
	}



	std::string getPathFromFile(const std::string& path)
	{
		std::string ret;

		size_t sep = path.find_last_of("\\/");
		if(sep != std::string::npos)
			ret = path.substr(0, sep);

		return ret;
	}

	std::string getFilenameFromPath(const std::string& path)
	{
		std::string ret;

		size_t sep = path.find_last_of("\\/");
		if(sep != std::string::npos)
			ret = path.substr(sep + 1);

		return ret;
	}


	std::string getFullPathOfFile(const std::string& partial)
	{
		std::string full = platform::getFullPath(partial);
		if(full.empty())
			error("nonexistent file %s", partial.c_str());

		return full;
	}

	std::string removeExtensionFromFilename(const std::string& name)
	{
		auto i = name.find_last_of(".");
		return name.substr(0, i);
	}
}