//header for instTree C interface

#ifndef __INSTTREE_H_
#define __INSTTREE_H_

void* createInstructionTree(void);
void insertIntoTree(long pN, unsigned long inst, void* tree);
long maxNode(void* tree);
void freeInstTree(void* tree);

#endif
