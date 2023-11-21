CC = gcc
EXECUTABLES = gen_numbers.out select_arg.out gen_in_range.out
SOURCES = gen_numbers.c select_arg.c gen_in_range.out
HEADERS = utils.h


.PHONY: all clean test


UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
all: $(EXECUTABLES) 
else
all:
	@echo "Sorry, I prefer Linux"
endif

gen_numbers.out: gen_numbers.c utils.c $(HEADERS)
	$(CC) -o $@ $^

select_arg.out: select_arg.c utils.c $(HEADERS)
	$(CC) -o $@ $^

gen_in_range.out: gen_in_range.c utils.c $(HEADERS)
	$(CC) -o $@ $^


clean:
	rm -f $(EXECUTABLES)

test: $(EXECUTABLES)
	./gen_numbers.out 6
	./select_arg.out one two three
	./gen_in_range.out 10 40

