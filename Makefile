all:
	rm -f ishell 
	make ishell 
	
ishell:
	g++ -Wall -g -o ishell ishell.cpp
