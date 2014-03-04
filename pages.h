//C header file for C++ functions
//Copyright Adrian McMenamin, 2014
//Licensed for reuse under the GPL

#ifndef __PAGES_H_
#define __PAGES_H_

void* createPageTree(void);
void removePageTree(void* tree);
inline void* getRootPageTree(void* tree);
inline void* getRootPageTreeLRU(void* tree);
void insertIntoPageTree(long pageNumber, time_t lruTime, void* tree);
void* locatePageTreePR(long pageNumber, void* tree);
void* locatePageTreeLRU(long pageNumber, void* tree);
void removeFromPageTree(long pageNumber, void* tree);
void* removeOldestFromPageTree(void* tree);
int countPageTree(void* tree);
void updateLRU(long pageNumber, time_t lruTime, void* tree);
struct PageChain* getPageChain(void* tree);

#endif
