
DIR_BUILD	:= build
DIR_SOURCES	:= src

NAME		:= $(DIR_BUILD)/shoxy

SRC			:=	$(DIR_SOURCES)/shoxy.c
OBJ			:= $(SRC:.c=.o)

CFLAGS		:= -W -Wall -Wextra -ansi -pedantic -g -I src/include

.PHONY: all build run clean fclean re

all: build

$(NAME): $(OBJ)
	@mkdir -p $(DIR_BUILD)
	$(CC) $(OBJ) -o $(NAME)

build: $(NAME)

run: build
	./$(NAME)

clean:
	$(RM) -f $(OBJ)

fclean: clean
	$(RM) -f $(NAME)

re: fclean all