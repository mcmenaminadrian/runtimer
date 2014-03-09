//C interface

#ifndef _OPTTREE_H_
#define _OPTTREE_H_

void* createOPTTree(void);
void readOPTTree(void* tree, char* path);
void removeOPTTree(void* tree);
struct pair* getPairChain(void* tree);
long nextInChain(long pageNumber, long instructionCount, void* tree);
void outlineOpt(void* tree);
#endif
