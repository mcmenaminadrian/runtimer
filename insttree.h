//header for instTree C interface

#ifndef __INSTTREE_H_
#define __INSTTREE_H_

void* createInstructionTree(void);
void insertIntoTree(long pN, long inst, void* tree);
long maxNodePage(void* tree);
long maxNodeDistance(void* tree);
void freeInstTree(void* tree);

#endif
