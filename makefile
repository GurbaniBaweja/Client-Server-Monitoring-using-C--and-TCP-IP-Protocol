all: client server

client: client.o tands.o
	g++ client.o tands.o -o client -lstdc++

server: server.o tands.o
	g++ server.o tands.o -o server -lstdc++

client.o: client.cpp tands.h
	g++ client.cpp -o client.o -c -Wall -lstdc++

server.o: server.cpp tands.h
	g++ server.cpp -o server.o -c -Wall -lstdc++

tands.o: tands.cpp tands.h
	g++ tands.cpp -o tands.o -c -Wall -lstdc++

manpages: server.man client.man
	groff -Tpdf -man server.man>|serverman.pdf
	groff -Tpdf -man client.man>|clientman.pdf
	
clean: 
	@rm -f client.o client server.o server tands.o




