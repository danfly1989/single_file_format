NAME_MINISHELL = minishell

LIBFT_DIR = libft
LIBFT = $(LIBFT_DIR)/libft.a

SRC_MINISHELL = \
	src/minishell.c

OBJ_MINISHELL = $(SRC_MINISHELL:.c=.o)

CC = cc
CFLAGS = -g3 -O0 -Wall -Wextra -Werror -Iincludes -I$(LIBFT_DIR)
LDFLAGS = -L$(LIBFT_DIR)
LDLIBS = -lft -lreadline -lhistory

.PHONY: all clean fclean re debug check-deps

all: check-deps $(NAME_MINISHELL)

check-deps:
	@echo "Checking dependencies..."
	@if [ ! -f "$(LIBFT)" ]; then \
		echo "libft not found, will build it..."; \
		make -C $(LIBFT_DIR); \
	fi

$(NAME_MINISHELL): $(OBJ_MINISHELL) $(LIBFT)
	@echo "Linking $(NAME_MINISHELL)..."
	$(CC) $(CFLAGS) $(OBJ_MINISHELL) -o $@ $(LDFLAGS) $(LDLIBS)

src/%.o: src/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	make -C $(LIBFT_DIR) clean
	rm -f $(OBJ_MINISHELL)

fclean: clean
	make -C $(LIBFT_DIR) fclean
	rm -f $(NAME_MINISHELL)

re: fclean all

debug:
	@echo "=== Debug Information ==="
	@echo "Operating system: $(shell uname -s)"
	@echo "LIBFT exists: $(shell test -f $(LIBFT) && echo 'YES' || echo 'NO')"
	@echo "Git available: $(shell which git >/dev/null 2>&1 && echo 'YES' || echo 'NO')"
	@echo "Source files:"
	@ls -la src/ 2>/dev/null || echo "src/ directory not found"
	@echo "========================"
