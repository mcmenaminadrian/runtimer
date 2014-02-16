all: runtimer 

runtimer: runtimer.o
	g++ -O2 -o runtimer -Wall runtimer.o -lexpat

runtimer.o: runtimer.c 
	gcc -O2 -o runtimer.o -c -Wall runtimer.c

