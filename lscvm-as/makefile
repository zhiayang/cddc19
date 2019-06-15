# makefile
# Copyright (c) 2019, zhiayang
# Licensed under the Apache License Version 2.0.

WARNINGS		:= -Wno-unused-parameter -Wno-sign-conversion -Wno-padded -Wno-conversion -Wno-shadow -Wno-missing-noreturn -Wno-unused-macros -Wno-switch-enum -Wno-deprecated -Wno-format-nonliteral -Wno-trigraphs -Wno-unused-const-variable -Wno-deprecated-declarations


CLANGWARNINGS	:= -Wno-undefined-func-template -Wno-comma -Wno-nullability-completeness -Wno-redundant-move -Wno-nested-anon-types -Wno-gnu-anonymous-struct -Wno-reserved-id-macro -Wno-extra-semi -Wno-gnu-zero-variadic-macro-arguments -Wno-shift-sign-overflow -Wno-exit-time-destructors -Wno-global-constructors -Wno-c++98-compat-pedantic -Wno-documentation-unknown-command -Wno-weak-vtables -Wno-c++98-compat

CC				?= "clang"
CXX				?= "clang++"

CXXFLAGS		+= -std=c++17 -O0 -g -c -Wall -frtti -fexceptions -fno-omit-frame-pointer -Wno-old-style-cast
CFLAGS			+= -std=c11 -O0 -g -c -Wall -fno-omit-frame-pointer -Wno-overlength-strings


.PHONY: as vm mem ascii

as: build/lscvm-as
vm: build/lscvm-vm
mem: build/lscvm-mem
ascii: build/lscvm-ascii

.DEFAULT_GOAL = all
all: as vm mem ascii

build/lscvm-as: $(shell find source -name "*.cpp")
	# lscvm-as
	@$(CXX) $(CXXFLAGS) $(WARNINGS) -Isource/include -o $@ $<

build/lscvm-vm: tools/lscvm-vm.cpp
	# lscvm-vm
	@$(CXX) $(CXXFLAGS) $(WARNINGS) -o $@ $<

build/lscvm-mem: tools/lscvm-mem.cpp
	# lscvm-mem
	@$(CXX) $(CXXFLAGS) $(WARNINGS) -o $@ $<

build/lscvm-ascii: tools/lscvm-ascii.cpp
	# lscvm-ascii
	@$(CXX) $(CXXFLAGS) $(WARNINGS) -o $@ $<



# haha
clena: clean
clean:
	@rm -f build/lscvm-as
	@rm -f build/lscvm-vm
	@rm -f build/lscvm-mem
	@rm -f build/lscvm-ascii






