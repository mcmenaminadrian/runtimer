all: runtimer 

runtimer: runtimer.o rbpages.o
	g++ -O2 -o runtimer -Wall rbpages.o runtimer.o -lexpat -lpthread

rbpages.o: pages.cpp redblack.hpp
	g++ -O2 -o rbpages.o -c -Wall pages.cpp

runtimer.o: runtimer.c 
	gcc -O2 -o runtimer.o -c -Wall runtimer.c

