#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <expat.h>


#define BUFFSZ 512
#define BITSHIFT 12
#define CORES 16
#define COREMEM 32


struct threadRecord
{
	int number;
	char path[BUFFSZ];
	struct threadRecord *next;
};

static struct threadRecord *startTR = NULL; 

void mapThread(struct threadRecord *root, int tNum, char* fileName)
{
	if (root == NULL) {
		struct threadRecord *newThread =
			(struct threadRecord*)
				malloc( sizeof (struct threadRecord));
		threadRecord->number = tNum;
		strcpy(threadRecord->path, fileName);
		threadRecord->next = NULL;
		root = threadRecord;
	} else 
		mapThread(root->next, tNum, fileName);
}

void usage()
{
	printf "USAGE:runtimer controlfile prefix\n"
}

static void XMLCALL
	starthandler(void *data, const XML_Char *name, const XML_Char **attr)
{
	int i;
	int threadID;
	char threadPath[BUFFSZ];
	if (strcmp(name, "file") == 0) {
		for (i = 0; attr[i]; i += 2) {
			if (strcmp(attr[i], "thread") == 0) {
				threadID = atoi(attr[i + 1]);
				break;
			}
		}
		for (i = 0; attr[i]; i += 2) {
			if (strcmp(attr[i], "path") == 0) {
				strcpy(threadPath, attr[i + 1];
				break;
			}
		}
		mapThread(startTR, threadID, threadPath);
		printf("Mapped thread %i at %s\n", threadID, threadPath);
	}
}

void main(int argc, char* argv[])
{
	FILE* inXML;
	char data[BUFFSZ]; 
	size_t len = 0;
	int done;	

	if (argc < 3) {
		usage();
		exit(-1);
	}

	strcpy(outputprefix, argv[2]);

	XML_Parser p_ctrl = XML_ParserCreate("UTF-8");
	if (!p_ctrl) {
		fprintf(stderr, "Could not create parser\n");
		exit(-1);
	}

	XML_SetStartElementHandler(p_ctrl, starthandler);
	inXML = fopen(argv[1], "r");
	if (inXML == NULL) {
		fprintf(stderr, "Could not open %s\n", argv[1]);
		XML_ParserFree(p_ctrl);
		exit(-1);
	}

	do {
		len = fread(data, 1, sizeof(data), inXML);
		done = len < sizeof(data);

		if (XML_Parse(p_ctrl, data, len, 0) == 0) {
			enum XML_Error errcde = XML_GetErrorCode(p_ctrl);
			printf("ERROR: %s\n", XML_ErrorString(errcde));
			printf("Error at column number %lu\n", XML_GetCurrentColumnNumber(p_ctrl));
			printf("Error at line number %lu\n", XML_GetCurrentLineNumber(p_ctrl));
		}
	} while(!done);

	XML_ParserFree(p_ctrl);
	fclose(inXML);
	return 0;
}
