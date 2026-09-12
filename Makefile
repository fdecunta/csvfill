PROG = updatecsv
SRC  = updatecsv.c

CC     = cc
CFLAGS = -Wall -Wextra -Werror -Wconversion -g 

$(PROG): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(PROG)

clean:
	rm -f $(PROG)

run: $(PROG)
	@./$(PROG) -1 1 -2 1 tests/example_dataframe.csv < tests/example_input.csv
#	@./$(PROG) -1 1 -2 1 tests/01_good_db.csv < tests/01_good_input.csv

mem: $(PROG)
	valgrind --leak-check=full ./updatecsv tests/example_dataframe.csv < tests/example_input.csv

test: $(PROG) run_test.sh
	./run_test.sh

.PHONY: run clean test
