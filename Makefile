CXX=clang++
CXXFLAGS=-Wall -Wextra -std=c++23 -pedantic -ggdb -Og
LIBS=-lraylib

DEPS=main.h player.h physics.h

OBJ=main.o physics.o

game: $(OBJ)
	$(CXX) $(CXXFLAGS) $(LIBS) $^ -o $@

run: game
	./$<

%.o: %.cc Makefile $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm *.o game

.PHONY: clean
