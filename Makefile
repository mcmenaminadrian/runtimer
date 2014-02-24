all: runtimer 

runtimer: runtimer.o rbpages.o opttree.o
	g++ -O2 -o runtimer -Wall rbpages.o opttree.o runtimer.o -lexpat -lpthread

opttree.o: opttree.cpp redblack.hpp
	g++ -O2 -o opttree.o -c -Wall opttree.cpp

rbpages.o: pages.cpp redblack.hpp
	g++ -O2 -o rbpages.o -c -Wall pages.cpp

runtimer.o: runtimer.c 
	gcc -O2 -o runtimer.o -c -Wall runtimer.c

