#!/bin/bash

# Configuration Parser Build Script for WSL

echo "🔨 Building configuration parser..."
echo

g++ -Wall -Wextra -Werror -std=c++98 -I. \
    main.cpp \
    config/Lexer.cpp \
    config/Parser.cpp \
    config/Validator.cpp \
    config/LocationConfig.cpp \
    config/ServerConfig.cpp \
    -o config_parser

if [ $? -eq 0 ]; then
    echo
    echo "✅ Compilation successful!"
    echo
    echo "Usage: ./config_parser <config_file>"
    echo "Example: ./config_parser test.conf"
    echo
else
    echo
    echo "❌ Compilation failed!"
    exit 1
fi
