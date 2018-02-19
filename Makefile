
DIR_BUILD	:= build
DIR_SOURCES	:= src

NAME		:= $(DIR_BUILD)/shoxy

ARGS		?= 127.0.0.1 2222
DEBUG		?= 0

SRC			:=	$(DIR_SOURCES)/shoxy.c \
						$(DIR_SOURCES)/signal.c \
						$(DIR_SOURCES)/logger.c \
						$(DIR_SOURCES)/client.c \
						$(DIR_SOURCES)/network/network.c \
						$(DIR_SOURCES)/network/poll.c \
						$(DIR_SOURCES)/network/tcp.c \
						$(DIR_SOURCES)/ssh/session.c \
						$(DIR_SOURCES)/ssh/callback_request.c \
						$(DIR_SOURCES)/ssh/callback_channel.c \
						$(DIR_SOURCES)/ssh/exec.c
OBJ			:= $(SRC:.c=.o)

CFLAGS	:= -W -Wall -Wextra -pedantic -I src/include
ifeq ($(DEBUG), 1)
	CFLAGS += -g
endif
LDFLAGS	:= -l ssh -l util

.PHONY: all build run clean fclean re

all: build

$(NAME): $(OBJ)
	@mkdir -p $(DIR_BUILD)
	$(CC) $(OBJ) $(LDFLAGS) -o $(NAME)

build: $(NAME)

run: build
ifeq ($(DEBUG), 1)
	valgrind --leak-check=full ./$(NAME) $(ARGS)
else
	./$(NAME) $(ARGS)
endif

clean:
	$(RM) -f $(OBJ)

fclean: clean
	$(RM) -f $(NAME)

re: fclean all
