GCC=gcc
CFLAGS=-Wall -g -pedantic
NAME=main

$(NAME): $(NAME).c
	$(GCC) $(CFLAGS) $< -o $@

test: $(NAME)
	./$(NAME)

clean:
	rm -f $(NAME) *.o