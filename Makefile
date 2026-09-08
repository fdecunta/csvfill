PROG = updatecsv
SRC  = updatecsv.c

CC     = cc
CFLAGS = -Wall -Wextra -Werror -Wconversion -g

$(PROG): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(PROG)

clean:
	rm -f $(PROG)

run: $(PROG)
	./$(PROG) -1 1 -2 1 tests/example_dataframe.csv < tests/example_input.csv

.PHONY: run clean
