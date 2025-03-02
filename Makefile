CC = gcc
PP = g++

C_SOURCE = src
H_SOURCE = src
TEST_SOURCE = test

OBJECT = obj
EXECUTABLE = exe

CFLAGS = -Wall -Werror -Wextra -O3 -I$(H_SOURCE) -static

C_GAME_LIBRARY_FILES = -luser32 -lgdi32

$(OBJECT):
	mkdir $(OBJECT)

$(EXECUTABLE):
	mkdir $(EXECUTABLE)

# all -> jschlang
all: $(OBJECT)/main.o | $(EXECUTABLE)/jschlang

# jschlang
$(EXECUTABLE)/jschlang: $(OBJECT)/main.o | $(EXECUTABLE)
	$(PP) $(CFLAGS) $(OBJECT)/main.o $(C_GAME_LIBRARY_FILES) -o $(EXECUTABLE)/jschlang
	strip $(EXECUTABLE)/jschlang.exe

# main.cpp
$(OBJECT)/main.o: $(C_SOURCE)/main.cpp | $(OBJECT)
	$(PP) $(CFLAGS) -c $(C_SOURCE)/main.cpp -o $(OBJECT)/main.o

clean:
	del /Q obj\*