CC := gcc
CFLAGS := -Wall -Iinclude
LDFLAGS :=
ifeq ($(DEBUG),1)
CFLAGS += -D_DEBUG
endif

OBJECTS := main.o 
EXECUTABLE := ./main
TEST := ./tests/test1.txt

build: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXECUTABLE) $(OBJECTS)

%.o: src/%.c
	@$(CC) $(CFLAGS) -c $^

test: clean build
	cat $(TEST) | $(EXECUTABLE) 2>/dev/null

exec: clean build
	$(EXECUTABLE) 2>/dev/null

checkpatch:
	scripts/checkpatch.pl --no-tree -f src/*

clean:
	rm -f $(EXECUTABLE)
	rm -f $(OBJECTS)
