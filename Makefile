default: all

all: runtimer 

debug: debugtimer

#get rid of junk
clean:
	rm -f *.o

# normal build
runtimer: runtimer.o insttree.o rbpages.o opttree.o threadhandler.o
	g++ -O2 -o runtimer -Wall insttree.o rbpages.o opttree.o \
		threadhandler.o runtimer.o -lexpat -lpthread

insttree.o: insttree.cpp insttree.h 
	g++ -O2 -o insttree.o -c -Wall insttree.cpp

threadhandler.o: threadhandler.c threadhandler.h 
	gcc -O2 -o threadhandler.o -c -Wall threadhandler.c

opttree.o: opttree.cpp redblack.hpp
	g++ -O2 -o opttree.o -c -Wall opttree.cpp

rbpages.o: pages.cpp 
	g++ -O2 -o rbpages.o -c -Wall pages.cpp

runtimer.o: runtimer.c threadhandler.h 
	gcc -O2 -o runtimer.o -c -Wall runtimer.c

# debug build
debugtimer: druntimer.o dinsttree.o drbpages.o dopttree.o dthreadhandler.o
	g++ -g -o runtimer -Wall dinsttree.o drbpages.o dopttree.o \
		dthreadhandler.o druntimer.o -lexpat -lpthread

dinsttree.o: insttree.cpp insttree.h 
	g++ -g -o dinsttree.o -c -Wall insttree.cpp

dthreadhandler.o: threadhandler.c threadhandler.h
	gcc -g -o dthreadhandler.o -c -Wall threadhandler.c

dopttree.o: opttree.cpp redblack.hpp
	g++ -g -o dopttree.o -c -Wall opttree.cpp

drbpages.o: pages.cpp
	g++ -g -o drbpages.o -c -Wall pages.cpp

druntimer.o: runtimer.c threadhandler.h 
	gcc -g -o druntimer.o -c -Wall runtimer.c


