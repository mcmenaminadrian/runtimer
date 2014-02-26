#include <stdio.h>
#include <stdlib.h>
#include <expat.h>
#include <pthread.h>


//C code that parses a thread

void* startThreadHandler(void *resources)
{
	struct threadResources thResources;
	thResources = (struct threadResources*)resources;
	printf("Setting up parser for thread %i\n",
		thResources->local->threadNumber);
	//Setup the Parser
	XML_Parser p_threadParser = XML_ParserCreate("UTF-8");
	if (!p_threadParser) {
		fprintf(stderr, "Could not create parser for thread\n");
		return (void*)(-1);
	}
	//Pass the parser thread specific data
	XML_SetUserData(p_threadParser, resources);
	//Start the Parser

	return NULL;
}
