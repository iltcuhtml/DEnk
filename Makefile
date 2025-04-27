CC = gcc
PP = g++

C_SOURCE = src
H_SOURCE = src
TEST_SOURCE = test

OBJECT = obj
EXECUTABLE = exe

CFLAGS = -std=c++23 -Wall -Werror -Wextra -O3 -I$(H_SOURCE) -static

$(OBJECT):
	mkdir $(OBJECT)

$(EXECUTABLE):
	mkdir $(EXECUTABLE)

# all -> DEnk
all: $(OBJECT)/main.o | $(EXECUTABLE)/DEnk

# DEnk
$(EXECUTABLE)/DEnk: $(OBJECT)/main.o | $(EXECUTABLE)
	$(PP) $(CFLAGS) $(OBJECT)/main.o -o $(EXECUTABLE)/DEnk
	strip $(EXECUTABLE)/DEnk.exe

# main.cpp
$(OBJECT)/main.o: $(C_SOURCE)/main.cpp | $(OBJECT)
	$(PP) $(CFLAGS) -c $(C_SOURCE)/main.cpp -o $(OBJECT)/main.o

obj_clean:
	del /Q obj\*

out_clean:
	del /Q out