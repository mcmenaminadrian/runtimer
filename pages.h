//C header file for C++ functions
//Copyright Adrian McMenamin, 2014
//Licensed for reuse under the GPL

void* createPageTree(void);
void removePageTree(void* tree);
inline void* getRootPageTree(void* tree);
inline void* getRootPageTreeLRU(void* tree);
void insertIntoPageTree(long pageNumber, long lruTime, void* tree);
void* locatePageTreePR(long pageNumber, void* tree);
void* locatePageTreeLRU(long pageNumber, void* tree);
void removeFromPageTree(long pageNumber, void* tree);
void* removeOldestFromPageTree(void* tree);
int countPageTree(void* tree);
void updateLRU(long pageNumber, long lruTime, void* tree);
