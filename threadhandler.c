#include <stdio.h>
#include <stdlib.h>
#include <expat.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "threadhandler.h"
#include "pages.h"

//C code that parses a thread

static void XMLCALL
threadXMLProcessor(void* data, const XML_Char *name, const XML_Char **attr)
{
	int i;
	long address;
	struct threadResources *thResources;
	struct threadGlobal *globals;
	struct threadLocal *local;
	thResources = (struct threadResources *)data;
	globals = thResources->globals;
	local = thResources->local;
	if (strcmp(name, "instruction") == 0 || strcmp(name, "load") == 0 ||
		strcmp(name, "modify") == 0 || strcmp(name, "store") == 0) {
		for (i = 0; attr[i]; i += 2) {
			if (strcmp(attr[i], "address") == 0) {
				time_t now = time(NULL);
				address = strtol(attr[i+1], NULL, 16);
				//have address - is it already present?
				pthread_mutex_lock(&globals->threadGlobalLock);
				if (locatePageTreePR(address >> BITSHIFT,
						globals->globalTree)) {
					updateLRU(address >> BITSHIFT,
						now,
						globals->globalTree);
					pthread_mutex_unlock(
						&globals->threadGlobalLock);
					if (locatePageTreePR(
						address >> BITSHIFT,
						local->localTree)) {
						updateLRU(
							address >> BITSHIFT,
							now, local->localTree);
					} else {
						insertIntoPageTree(
						address >> BITSHIFT,
						now, local->localTree);
					}
					local->instructionCount++;
				} else {
					pthread_mutex_unlock(
						&globals->threadGlobalLock);
					if (faultPage(address >> BITSHIFT)) {
						pthread_mutex_lock(
						&globals->threadGlobalLock);
						//have to add the page
						int howMany =
						countPageTree(
							globals->globalTree);
						if (howMany >= CORES * COREMEM)
						{
							replacePage(
							address >> BITSHIFT,
							thResources);
							pthread_mutex_unlock(
							&globals->
							threadGlobalLock);
						} else {
							insertIntoPageTree(
							address >> BITSHIFT,
							now,
							globals->globalTree);
							pthread_mutex_unlock(
							&globals->
							threadGlobalLock);
							insertIntoPageTree(
							address >>BITSHIFT,
							now,
							local->localTree);
						}
					}
				}
			}
		}
	}
}

						

void* startThreadHandler(void *resources)
{
	struct threadResources *thResources;
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
	XML_SetStartElementHandler(p_threadParser, threadXMLProcessor);
	return NULL;
}
