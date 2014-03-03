#include <stdio.h>
#include <stdlib.h>
#include <expat.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "threadhandler.h"
#include "pages.h"
#include "insttree.h"
#include "opttree.h"

//C code that parses a thread

static void replacePage(long pageNumber, struct ThreadResources *thResources)
{ printf("HERE>>>");
	//find the page with the longest reuse distance,
	//otherwise find the page with the oldest date
	//either for this thread or all threads
	struct PageChain *currentChain =
		getPageChain(thResources->local->localTree);	
	void* instructionTree = createInstructionTree();
	while (currentChain) {
		long instructionNext =
			nextInChain(currentChain->page,
			thResources->local->instructionCount,
			thResources->local->optTree);
		if (instructionNext == -1) {
			fprintf(stderr,
			"ERROR: Page %li in localTree but not in optTree",
			currentChain->page);
		} else {
			insertIntoTree(currentChain->page, instructionNext,
				instructionTree);
		}
		currentChain = currentChain->next;
	}
	//now get the maximum
	int pageFound = 0;
	do {
		long maximumReuse = maxNode(instructionTree);
		if (maximumReuse) {
			//lock the global tree hard
			pthread_mutex_lock(
				&thResources->globals->threadGlobalLock);
			if (locatePageTreePR(maximumReuse,
				thResources->globals->globalTree)) {
				pageFound = 1;
				removeFromPageTree(maximumReuse,
					thResources->globals->globalTree);
			}
			pthread_mutex_unlock(
				&thResources->globals->threadGlobalLock);
		} else {
			break;
		}
	} while (pageFound == 0);

	//if we didn't find the page, kill the oldest page
	if (pageFound == 0) {
		pthread_mutex_lock(&thResources->globals->threadGlobalLock);
		removeOldestFromPageTree(thResources->globals->globalTree);
		pthread_mutex_unlock(&thResources->globals->threadGlobalLock);
	}
	freeInstTree(instructionTree);
}
		
static int faultPage(long pageNumber, struct ThreadResources *thResources)
{
	int countDown = 100 * MEMWIDTH;
	while (countDown) {
		if (locatePageTreePR(pageNumber,
			thResources->globals->globalTree)) {
			return 0;
		}
		thResources->local->instructionCount++;
		countDown--;
	}
	thResources->local->faultCount++;
	return 1;
}

static void inGlobalTree(long pageNumber, struct ThreadResources *thResources,
	time_t *now)
{
	struct ThreadGlobal *globals = thResources->globals;
	struct ThreadLocal *local = thResources->local;
	pthread_mutex_unlock(&globals->threadGlobalLock);
	if (locatePageTreePR(pageNumber, local->localTree)) {
		updateLRU(pageNumber, *now, globals->globalTree);
	} else {
		insertIntoPageTree(pageNumber, *now, local->localTree);
	}
	local->instructionCount++;
}

static void notInGlobalTree(long pageNumber,
	struct ThreadResources *thResources, time_t *now)
{
	struct ThreadGlobal *globals = thResources->globals;
	struct ThreadLocal *local = thResources->local;
	pthread_mutex_unlock(&globals->threadGlobalLock);
	if (faultPage(pageNumber, thResources)) {
		pthread_mutex_lock(&globals->threadGlobalLock);
		if (countPageTree(globals->globalTree) >= CORES * COREMEM) {
			replacePage(pageNumber, thResources);
			pthread_mutex_unlock(&globals->threadGlobalLock);
		} else {
			insertIntoPageTree(pageNumber, *now,
				globals->globalTree);
			pthread_mutex_unlock(&globals->threadGlobalLock);
			insertIntoPageTree(pageNumber, *now, local->localTree);
		}
	}
}
	
static void XMLCALL
threadXMLProcessor(void* data, const XML_Char *name, const XML_Char **attr)
{ 
	int i;
	long address, pageNumber;
	struct ThreadResources *thResources;
	struct ThreadGlobal *globals;
	struct ThreadLocal *local;
	thResources = (struct ThreadResources *)data;
	globals = thResources->globals;
	local = thResources->local;
	if (strcmp(name, "instruction") == 0 || strcmp(name, "load") == 0 ||
		strcmp(name, "modify") == 0 || strcmp(name, "store") == 0) {
		for (i = 0; attr[i]; i += 2) {
			if (strcmp(attr[i], "address") == 0) {
				time_t now = time(NULL);
				address = strtol(attr[i+1], NULL, 16);
				pageNumber = address >> BITSHIFT;
				//have address - is it already present?
				pthread_mutex_lock(&globals->threadGlobalLock);
				if (locatePageTreePR(pageNumber,
						globals->globalTree)) {
					inGlobalTree(pageNumber, thResources,
						&now);
				} else {
					notInGlobalTree(pageNumber,
						thResources, &now);
				}
			}
		}
	}
}


void* startThreadHandler(void *resources)
{
	FILE* inThreadXML;
	size_t len = 0;
	int done;
	char data[BUFFSZ];

	struct ThreadResources *thResources;
	thResources = (struct ThreadResources*)resources;
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
	inThreadXML = fopen(thResources->records->path, "r");
	if (inThreadXML == NULL) {
		fprintf(stderr, "Could not open %s\n",
			thResources->records->path);
		XML_ParserFree(p_threadParser);
		goto cleanup;
	}

	do { 
		len = fread(data, 1, sizeof(data), inThreadXML);
		done = len < sizeof(data);
		
		if (XML_Parse(p_threadParser, data, len, 0) == 0) {
			enum XML_Error errcde = XML_GetErrorCode(p_threadParser);
			printf("PARSE ERROR: %s\n", XML_ErrorString(errcde));
			printf("Error at column number %lu\n",
				XML_GetCurrentColumnNumber(p_threadParser));
			printf("Error at line number %lu\n",
				XML_GetCurrentLineNumber(p_threadParser));
			goto cleanup;
		}
	} while(!done);

cleanup:
	free(thResources->records);
	removeOPTTree(thResources->local->optTree);
	removePageTree(thResources->local->localTree);
	removePageTree(thResources->globals->globalTree);
	free(thResources->globals);
	free(thResources);
	
	return NULL;
}
