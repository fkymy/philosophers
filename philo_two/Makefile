# Makefile for philosophers, updated Sat Mar  6 16:06:48 JST 2021

SRC := dine.c eat_sleep.c main.c utils.c

OBJ := dine.o eat_sleep.o main.o utils.o

HDR := philosophers.h

# DO NOT ADD OR MODIFY ANY LINES ABOVE THIS -- run 'make source' to add files

NAME := philo_two

CC := gcc
CFLAGS := -Wall -Wextra -Werror

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

