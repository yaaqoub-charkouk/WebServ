@echo off
REM Windows compilation script for config_parser
REM Requires g++ (MinGW or similar) in PATH

echo Compiling configuration parser...
echo.

g++ -Wall -Wextra -Werror -std=c++98 -I. ^
    main.cpp ^
    config/Lexer.cpp ^
    config/Parser.cpp ^
    config/Validator.cpp ^
    config/LocationConfig.cpp ^
    config/ServerConfig.cpp ^
    -o config_parser.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] Compilation successful!
    echo Executable: config_parser.exe
    echo.
    echo Usage: config_parser.exe test.conf
) else (
    echo.
    echo [ERROR] Compilation failed!
    exit /b 1
)
