BINDIR=./bin
OBJDIR=./obj
SRCDIR=./src
INCDIR=./include
TESTDIR=./test

CC=gcc
CFLAGS=-std=c99 -O0 -g -Wall -Wextra -I$(INCDIR)
LDFLAGS=-g -lm

SRC_FILES=$(wildcard $(SRCDIR)/*.c)
OBJ_FILES=$(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC_FILES))

##TESTS=$(TESTDIR)/test1 $(TESTDIR)/test2

##EXEC=$(BINDIR)/exec

all: #$(BINDIR)/exec

# Règle générique

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCDIR)/%.h
	$(CC) -c $(CFLAGS) $< -o $@

# Coeur du projet

$(EXEC): $(OBJ_FILES)
	$(CC) $(LDFLAGS) $^ -o $@

# Non-generic rule
#$(OBJDIR)/x.o: $(INCDIR)/i1.h $(INCDIR)/i2.h

.PHONY: clean

# Tests

#$(TESTDIR)/test_1.o: $(TESTDIR)/test1.c include/i1.h include/i2.h ...
	$(CC) -c $(CFLAGS) $< -o $@

#$(TESTDIR)/test_1: $(TESTDIR)/test_1.o $(OBJDIR)/obj1.o $(OBJDIR)/obj2.o
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(EXEC) $(OBJ_FILES) $(TESTS_OBJ) $(TESTS) $(TESTDIR)/*.o
