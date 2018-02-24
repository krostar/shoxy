VERSION		:= 1.0-1

DIR_BUILD	:= build
DIR_BUILD_DPKG := $(DIR_BUILD)/dpkg/shoxy_$(VERSION)
DIR_PACKAGE := package
DIR_SOURCES	:= src

NAME		:= $(DIR_BUILD)/shoxy

ARGS		?= config.cfg
DEBUG		?= 0

SRC			:=	$(DIR_SOURCES)/shoxy.c \
						$(DIR_SOURCES)/signal.c \
						$(DIR_SOURCES)/logger.c \
						$(DIR_SOURCES)/config.c \
						$(DIR_SOURCES)/client.c \
						$(DIR_SOURCES)/pam.c \
						$(DIR_SOURCES)/network/network.c \
						$(DIR_SOURCES)/network/poll.c \
						$(DIR_SOURCES)/network/tcp.c \
						$(DIR_SOURCES)/ssh/session.c \
						$(DIR_SOURCES)/ssh/callback_request.c \
						$(DIR_SOURCES)/ssh/callback_channel.c \
						$(DIR_SOURCES)/ssh/command.c \
						$(DIR_SOURCES)/ssh/proxy.c
OBJ			:= $(SRC:.c=.o)

CFLAGS	:= -W -Wall -Wextra -pedantic -I src/include
ifeq ($(DEBUG), 1)
	CFLAGS += -g
endif
LDFLAGS	:= -l ssh -l util -l pam

.PHONY: all build run clean fclean re

all: fclean keys build

keys:
	mkdir -p $(DIR_BUILD)/keys/
	ssh-keygen -t dsa -b 1024 -f $(DIR_BUILD)/keys/ssh_host_dsa_key -C "shoxy-server" -N ''
	ssh-keygen -t rsa -b 8192 -f $(DIR_BUILD)/keys/ssh_host_rsa_key -C "shoxy-server" -N ''

$(NAME): $(OBJ)
	mkdir -p $(DIR_BUILD)
	$(CC) $(OBJ) $(LDFLAGS) -o $(NAME)

build: $(NAME)

package: build
	mkdir -p $(DIR_BUILD_DPKG)/DEBIAN
	mkdir -p $(DIR_BUILD_DPKG)/etc/shoxy
	mkdir -p $(DIR_BUILD_DPKG)/etc/shoxy/keys
	mkdir -p $(DIR_BUILD_DPKG)/etc/systemd/system
	mkdir -p $(DIR_BUILD_DPKG)/usr/local/bin
	cp $(DIR_PACKAGE)/control $(DIR_BUILD_DPKG)/DEBIAN
	cp $(DIR_PACKAGE)/config.cfg $(DIR_BUILD_DPKG)/etc/shoxy/
	cp $(DIR_PACKAGE)/shoxy.service $(DIR_BUILD_DPKG)/etc/systemd/system/
	cp $(NAME) $(DIR_BUILD_DPKG)/usr/local/bin/
	cd $(DIR_BUILD_DPKG)/.. && dpkg-deb --build shoxy_$(VERSION)

run: build
ifeq ($(DEBUG), 1)
	valgrind --leak-check=full ./$(NAME) $(ARGS)
else
	./$(NAME) $(ARGS)
endif

clean:
	$(RM) -f $(OBJ)

fclean: clean
	$(RM) -rf $(DIR_BUILD)

re: clean build
