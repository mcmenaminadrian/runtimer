#include <stdio.h>
#include <stdlib.h>
#include <expat.h>
#include <pthread.h>


//C code that parses a thread

void* startThreadHandler(void *resources)
{
	struct threadResources thResources;
	thResources = (struct threadResources*)resources;

	return NULL;
}
