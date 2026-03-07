# Compiler and flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
INCLUDES = -I.

# Directories
CONFIG_SRC_DIR = tahalla/src/
CONFIG_INCLUDE_DIR = tahalla/include/

# Target executable
NAME = config_parser

# Source files
SRCS = main.cpp \
       $(CONFIG_SRC_DIR)/Lexer.cpp \

# Object files
OBJS = $(SRCS:.cpp=.o)

# Header files (for dependencies)
HEADERS = $(CONFIG_INCLUDE_DIR)/ConfigException.hpp \
          $(CONFIG_INCLUDE_DIR)/Token.hpp

# Colors for output
GREEN = \033[0;32m
RED = \033[0;31m
YELLOW = \033[0;33m
NC = \033[0m # No Color

# Rules
all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(YELLOW)Linking $(NAME)...$(NC)"
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)✓ $(NAME) compiled successfully!$(NC)"

%.o: %.cpp $(HEADERS)
	@echo "$(YELLOW)Compiling $<...$(NC)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "$(RED)Cleaning object files...$(NC)"
	@rm -f $(OBJS)
	@echo "$(GREEN)✓ Cleaned!$(NC)"

fclean: clean
	@echo "$(RED)Removing $(NAME)...$(NC)"
	@rm -f $(NAME)
	@echo "$(GREEN)✓ Fully cleaned!$(NC)"

re: fclean all

test: $(NAME)
	@echo "$(YELLOW)Running test...$(NC)"
	@./$(NAME) test.conf

.PHONY: all clean fclean re test
