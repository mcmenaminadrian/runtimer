//header for instTree C interface

#ifndef __INSTTREE_H_
#define __INSTTREE_H_

void* createInstructionTree(void);
void insertIntoTree(long pN, long inst, void* tree);
long maxNodePage(void* tree);
long maxNodeDistance(void* tree);
void freeInstTree(void* tree);
void* createMinTree(void);
void pushToMinTree(void* mTree, void* iTree);
long getPageToKill(void* tree);
void killMinTree(void* tree);
#endif
