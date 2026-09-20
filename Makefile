PROG = csvfill
SRC  = csvfill.c
MAN  = csvfill.1

CC     = cc
CFLAGS = -Wall -Wextra -Werror -Wconversion

BINDIR = /usr/local/bin
MANDIR = /usr/local/man/man1

$(PROG): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(PROG)

install: $(PROG)
	cp $(PROG) $(BINDIR)/$(PROG)
	mkdir -p $(MANDIR)
	cp $(MAN) $(MANDIR)/$(MAN)

remove: $(PROG)
	rm -f $(BINDIR)/$(PROG)
	rm -f $(MANDIR)/$(MAN)

clean:
	rm -f $(PROG)

test: $(PROG) run_test.sh
	./run_test.sh

.PHONY: install remove clean test
