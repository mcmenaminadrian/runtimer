all: runtimer 

runtimer: runtimer.o rbpages.o opttree.o threadhandler.o
	g++ -O2 -o runtimer -Wall rbpages.o opttree.o threadhandler.o runtimer.o -lexpat -lpthread

threadhandler.o: threadhandler.c threadhandler.h
	gcc -O2 -o threadhandler.o -c -Wall threadhandler.c

opttree.o: opttree.cpp redblack.hpp
	g++ -O2 -o opttree.o -c -Wall opttree.cpp

rbpages.o: pages.cpp redblack.hpp
	g++ -O2 -o rbpages.o -c -Wall pages.cpp

runtimer.o: runtimer.c 
	gcc -O2 -o runtimer.o -c -Wall runtimer.c

