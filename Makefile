# Compiler and flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
INCLUDES = -I.

# Directories
CONFIG_SRC_DIR    = tahalla/src
CONFIG_INCLUDE_DIR = tahalla/include
SERVER_SRC_DIR    = yaaqoub/server
HTTP_SRC_DIR      = adnane/HttpResponse
CGI_SRC_DIR       = adnane/cgi
SRCS_DIR          = srcs

# ── config_parser target (original, unchanged) ───────────────────────────────

CONFIG_PARSER_NAME = config_parser

CONFIG_PARSER_SRCS = main.cpp \
       $(CONFIG_SRC_DIR)/Lexer.cpp \
       $(CONFIG_SRC_DIR)/Parser.cpp \
       $(CONFIG_SRC_DIR)/ParserBlocks.cpp \
       $(CONFIG_SRC_DIR)/ParserServerDirectives.cpp \
       $(CONFIG_SRC_DIR)/ParserLocationDirectives.cpp \
       $(CONFIG_SRC_DIR)/ParserUtils.cpp \
       $(CONFIG_SRC_DIR)/Validator.cpp \
       $(CONFIG_SRC_DIR)/LocationConfig.cpp \
       $(CONFIG_SRC_DIR)/ServerConfig.cpp

CONFIG_PARSER_OBJS = $(CONFIG_PARSER_SRCS:.cpp=.o)

# ── webserv target ────────────────────────────────────────────────────────────

WEBSERV_NAME = webserv

WEBSERV_SRCS = webserv_main.cpp \
       $(CONFIG_SRC_DIR)/Lexer.cpp \
       $(CONFIG_SRC_DIR)/Parser.cpp \
       $(CONFIG_SRC_DIR)/ParserBlocks.cpp \
       $(CONFIG_SRC_DIR)/ParserServerDirectives.cpp \
       $(CONFIG_SRC_DIR)/ParserLocationDirectives.cpp \
       $(CONFIG_SRC_DIR)/ParserUtils.cpp \
       $(CONFIG_SRC_DIR)/Validator.cpp \
       $(CONFIG_SRC_DIR)/LocationConfig.cpp \
       $(CONFIG_SRC_DIR)/ServerConfig.cpp \
       $(SERVER_SRC_DIR)/server.cpp \
       $(SERVER_SRC_DIR)/server_client_connection.cpp \
       $(SERVER_SRC_DIR)/server_api.cpp \
       $(HTTP_SRC_DIR)/HttpResponse.cpp \
       $(HTTP_SRC_DIR)/FileUtils.cpp \
       $(CGI_SRC_DIR)/Cgi.cpp \
       $(SRCS_DIR)/HttpRequest.cpp \
       $(SRCS_DIR)/RequestHandler.cpp

WEBSERV_OBJS = $(WEBSERV_SRCS:.cpp=.o)

# Header files (for dependencies)
HEADERS = $(CONFIG_INCLUDE_DIR)/ConfigException.hpp \
          $(CONFIG_INCLUDE_DIR)/Token.hpp \
          $(CONFIG_INCLUDE_DIR)/LocationConfig.hpp \
          $(CONFIG_INCLUDE_DIR)/ServerConfig.hpp \
          $(CONFIG_INCLUDE_DIR)/Lexer.hpp \
          $(CONFIG_INCLUDE_DIR)/Parser.hpp \
          $(CONFIG_INCLUDE_DIR)/Validator.hpp \
          $(SERVER_SRC_DIR)/server.hpp \
          $(HTTP_SRC_DIR)/HttpResponse.hpp \
          $(HTTP_SRC_DIR)/FileUtils.hpp \
          $(CGI_SRC_DIR)/Cgi.hpp \
          $(SRCS_DIR)/HttpRequest.hpp \
          $(SRCS_DIR)/RequestHandler.hpp

# Colors for output
GREEN  = \033[0;32m
RED    = \033[0;31m
YELLOW = \033[0;33m
NC     = \033[0m

# ── Rules ─────────────────────────────────────────────────────────────────────

all: $(WEBSERV_NAME) $(CONFIG_PARSER_NAME)

$(CONFIG_PARSER_NAME): $(CONFIG_PARSER_OBJS)
	@echo "$(YELLOW)Linking $(CONFIG_PARSER_NAME)...$(NC)"
	@$(CXX) $(CXXFLAGS) $(CONFIG_PARSER_OBJS) -o $(CONFIG_PARSER_NAME)
	@echo "$(GREEN)✓ $(CONFIG_PARSER_NAME) compiled successfully!$(NC)"

$(WEBSERV_NAME): $(WEBSERV_OBJS)
	@echo "$(YELLOW)Linking $(WEBSERV_NAME)...$(NC)"
	@$(CXX) $(CXXFLAGS) $(WEBSERV_OBJS) -o $(WEBSERV_NAME)
	@echo "$(GREEN)✓ $(WEBSERV_NAME) compiled successfully!$(NC)"

%.o: %.cpp $(HEADERS)
	@echo "$(YELLOW)Compiling $<...$(NC)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "$(RED)Cleaning object files...$(NC)"
	@rm -f $(CONFIG_PARSER_OBJS) $(WEBSERV_OBJS)
	@echo "$(GREEN)✓ Cleaned!$(NC)"

fclean: clean
	@echo "$(RED)Removing executables...$(NC)"
	@rm -f $(CONFIG_PARSER_NAME) $(WEBSERV_NAME)
	@echo "$(GREEN)✓ Fully cleaned!$(NC)"

re: fclean all

test: $(CONFIG_PARSER_NAME)
	@echo "$(YELLOW)Running config parser test...$(NC)"
	@./$(CONFIG_PARSER_NAME) test.conf

.PHONY: all clean fclean re test

