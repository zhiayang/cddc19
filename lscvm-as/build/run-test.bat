@echo off

SETLOCAL

IF /I "%1"=="release" (
	SET buildDir="build\meson-rel"
)

IF /I "%1"=="debug" (
	SET buildDir="build\meson-dbg"
)


ninja -C %buildDir% && %buildDir%\lscvm-as.exe build\%2 %3 %4 %5 %6 %7 %8 %9

ENDLOCAL
