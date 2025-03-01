CC = gcc
PP = g++

C_SOURCE = src/c
H_SOURCE = src/h
TEST_SOURCE = test

OBJECT = obj
EXECUTABLE = exe

CFLAGS = -Wall -Werror -Wextra -O3 -I$(H_SOURCE) -static

C_GAME_LIBRARY_FILES = -luser32 -lgdi32

$(OBJECT):
	mkdir $(OBJECT)

$(EXECUTABLE):
	mkdir $(EXECUTABLE)

# all -> hl
all: $(OBJECT)/main.o | $(EXECUTABLE)/hl

# hl
$(EXECUTABLE)/hl: $(OBJECT)/main.o | $(EXECUTABLE)
	$(PP) $(CFLAGS) $(OBJECT)/main.o $(C_GAME_LIBRARY_FILES) -o $(EXECUTABLE)/hl
	strip $(EXECUTABLE)/hl.exe

# main.cpp
$(OBJECT)/main.o: $(C_SOURCE)/main.cpp | $(OBJECT)
	$(PP) $(CFLAGS) -c $(C_SOURCE)/main.cpp -o $(OBJECT)/main.o

clean:
	del /Q obj\*