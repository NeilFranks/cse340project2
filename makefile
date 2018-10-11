a.out: main.o lexer.o inputbuf.o
	g++ main.o lexer.o inputbuf.o -o a.out

main.o: main.cpp
	g++ -c main.cpp

lexer.o: lexer.cpp
	g++ -c lexer.cpp

inputbuf.o: inputbuf.cpp
	g++ -c inputbuf.cpp

clean:
	rm *.o output


