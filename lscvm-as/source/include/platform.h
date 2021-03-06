// platform.h
// Copyright (c) 2017, zhiayang
// Licensed under the Apache License Version 2.0.

// platform-specific things

#pragma once

#include "defs.h"

namespace platform
{
	#define ALLOCATE_MEMORY_FUNC						"malloc"
	#define REALLOCATE_MEMORY_FUNC						"realloc"
	#define FREE_MEMORY_FUNC							"free"


	#if defined(_WIN32) && !defined(__MINGW32__)
		#define WIN32_LEAN_AND_MEAN 1

		#ifndef NOMINMAX
			#define NOMINMAX
		#endif

		#include <windows.h>
		using filehandle_t = HANDLE;

		#define CRT_FDOPEN			"_fdopen"
		#define PLATFORM_NEWLINE	"\r\n"

		#define PLATFORM_IS_WINDOWS true
	#else
		#include <unistd.h>
		#include <sys/stat.h>

		using filehandle_t = int;

		#define CRT_FDOPEN			"fdopen"
		#define PLATFORM_NEWLINE	"\n"

		#define PLATFORM_IS_WINDOWS false

		// mingw can be dumb.
		#if defined(__MINGW32__)

			#define WIN32_LEAN_AND_MEAN 1
			#ifndef NOMINMAX
				#define NOMINMAX
			#endif
			#include <windows.h>
			#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
		#endif
	#endif

	extern filehandle_t InvalidFileHandle;

	#define REFCOUNT_SIZE		8

	filehandle_t openFile(const char* name, int mode, int flags);
	// void deleteFile(filehandle_t fd);
	void closeFile(filehandle_t fd);

	size_t readFile(filehandle_t fd, void* buf, size_t count);
	size_t writeFile(filehandle_t fd, void* buf, size_t count);

	size_t getFileSize(const std::string& path);
	bool checkFileExists(const std::string& path);

	std::string_view readEntireFile(const std::string& path);

	std::string getFullPath(const std::string& partial);
	std::string getTemporaryFilename(const std::string& name);

	size_t getTerminalWidth();
}

