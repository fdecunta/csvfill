PROG = csvfill
SRC  = csvfill.c

CC     = cc
CFLAGS = -Wall -Wextra -Werror -Wconversion -g 

$(PROG): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(PROG)

clean:
	rm -f $(PROG)

run: $(PROG)
	@./$(PROG) -1 1 -2 1 tests/01_good_db.csv < tests/01_good_input.csv

mem: $(PROG)
	valgrind --leak-check=full ./csvfill tests/01_good_db.csv < tests/01_good_input.csv

test: $(PROG) run_test.sh
	./run_test.sh

.PHONY: run clean test
