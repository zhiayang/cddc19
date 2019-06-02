@echo off

:again
set isquit=""

(call )

cls
lualatex --interaction=nonstopmode --halt-on-error --shell-escape writeup

echo.
set /p isquit= "q to quit / enter to recompile: "

IF /I "%isquit%"=="q" (
	echo "i am q"
	goto doquit
)

goto again

:doquit
set isquit=""
