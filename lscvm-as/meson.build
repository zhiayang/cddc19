project('lscvm-as', ['c', 'cpp'])

the_compiler = meson.get_compiler('c')

if get_option('buildtype') == 'debug'
	libKind = 'Debug'
else
	libKind = 'Release'
endif

if the_compiler.get_id() == 'msvc'
	add_project_arguments('/std:c++17', language: 'cpp')
	add_project_arguments('/permissive-', language: 'cpp')
	add_project_arguments('-D_STDC_LIMIT_MACROS', language: 'cpp')
	add_project_arguments('-D_SCL_SECURE_NO_WARNINGS', language: 'cpp')
 	add_project_arguments('-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING', language: 'cpp')

	add_project_link_arguments('/machine:X64', language: 'cpp')
	add_project_link_arguments('/incremental', language: 'cpp')
	add_project_link_arguments('/debug:fastlink', language: ['c', 'cpp'])
	add_project_link_arguments('/nodefaultlib:libcmt.lib', language: 'cpp')
	add_project_link_arguments('/nodefaultlib:libcmtd.lib', language: 'cpp')
else
	# add_project_arguments('--std=c++17', language: 'cpp')
	# add_project_arguments('--std=c11', language: 'c')

	error('You fool, use the makefile for non-windows builds')
endif


source_files = files([
	'source/main.cpp',

	'source/error.cpp',
	'source/lexer.cpp',
	'source/parser.cpp',
	'source/codegen.cpp',
	'source/platform.cpp',
	'source/frontend.cpp'
])

include_dirs = include_directories([ 'source/include' ])

executable('lscvm-as', source_files,
	include_directories: include_dirs
)

executable('lscvm-vm', files([ 'tools/lscvm-vm.cpp' ]))
executable('lscvm-mem', files([ 'tools/lscvm-mem.cpp' ]))
executable('lscvm-ascii', files([ 'tools/lscvm-ascii.cpp' ]))



