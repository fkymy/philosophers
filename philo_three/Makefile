# Makefile for philosophers, updated Mon Mar  1 11:45:20 JST 2021

SRC := main.c

OBJ := main.o

HDR := philosophers.h

# DO NOT ADD OR MODIFY ANY LINES ABOVE THIS -- run 'make source' to add files

NAME := philo_three

CC := gcc
CFLAGS := -Wall -Wextra -Werror -fsanitize=address

BINDIR := /usr/local/bin

LIBS :=

.PHONY: all re fclean clean source lint
all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)
	@echo "\nCompiled Successfully.\n"

$(OBJ): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

re: fclean all

fclean: clean
	-rm -f $(NAME)

clean:
	-rm -f Makefile.bak $(OBJ)

source:
	@mv Makefile Makefile.bak
	@echo "# Makefile for philosophers, updated `date`" > Makefile
	@echo '' >> Makefile
	@echo SRC := `ls *.c` >> Makefile
	@echo '' >> Makefile
	@echo OBJ := `ls *.c | sed s/c$$/o/` >> Makefile
	@echo '' >> Makefile
	@echo HDR := `ls *.h` >> Makefile
	@echo '' >> Makefile
	@sed -n -e '/^# DO NOT ADD OR MODIFY/,$$p' < Makefile.bak >> Makefile

lint:
	~/.norminette/norminette.rb $(SRC) $(HDR)

