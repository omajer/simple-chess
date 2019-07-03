CFLAGS = -Wall -pedantic -Wno-long-long -O0 -ggdb

all: compile doc

compile: ./src/Bishop.cpp ./src/Board.h ./src/Knight.cpp ./src/Pawn.cpp ./src/Piece.h ./src/Rook.cpp ./src/Server.h ./src/Bishop.h ./src/King.cpp  ./src/Knight.h ./src/Pawn.h ./src/Queen.cpp ./src/Rook.h ./src/Board.cpp  ./src/King.h ./src/main.cpp ./src/Piece.cpp ./src/Queen.h ./src/Server.cpp
	g++ $(CFLAGS) -o ./majeron1 ./src/Bishop.cpp  ./src/Board.cpp  ./src/King.cpp  ./src/Knight.cpp  ./src/main.cpp  ./src/Pawn.cpp  ./src/Piece.cpp  ./src/Queen.cpp  ./src/Rook.cpp  ./src/Server.cpp

run: 
	./majeron1
clean:
	rm ./majeron1
	rm -r doc
doc:
	cd src; doxygen Doxyfile
