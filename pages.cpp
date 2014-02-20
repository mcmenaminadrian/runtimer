#include <iostream>
#include "pthread.h"
#include "redblack.hpp"

using namespace std;

class PageRecord {
	protected:
		long pageNumber;
		long lruNumber;
	public:
		PageRecord(const long pgN, const long lruN);
		const long getPageNumber(void) const;
		const long getLRUNumber(void) const;
		void setLRUNumber(const long lruN);
		virtual bool operator==(PageRecord& pR) const;
		virtual bool operator<(PageRecord& pR) const;
};

class PageRecordLRU: public PageRecord {
	public:
	virtual bool operator==(PageRecord& pRLRU) const;
	virtual bool operator<(PageRecord& pRLRU) const;
};

class PageRecordTree {
	public:
	redblacktree<redblacknode<PageRecord> >* pageRecordTree;
	redblacktree<redblacknode<PageRecordLRU> >* pageRecordLRUTree;
	pthread_mutex_t tree_mutex;

	PageRecordTree(redblacktree<redblacknode<PageRecord> >*,
		redblacktree<redblacknode<PageRecordLRU> >*);
};

void* getMatchingTree(void* tree)
{
	redblacktree<redblacknode<PageRecord> >* testMatch =
		static_cast<void *>(pageRecordTree);
	redblacktree<redblacknode<PageRecord> >* testMatchLRU =
		static_cast<void *>(pageRecordLRUTree);


	if (tree == testMatch)
		return pageRecordTree
	else if (tree == testMatchLRU)
		return pageRecordLRUTree;

	return NULL;
}

PageRecordTree::PageRecordTree(redblacktree<redblacknode<PageRecord> >* prTree,
	redblacktree<redblacknode<PageRecordLRU> >* prLRUTree)
{
	pageRecordTree = prTree;
	pageRecordLRUTree = prLRUTree;
	tree_mutex = PTHREAD_MUTEX_INITIALIZER;
}

PageRecord::PageRecord(const long pgN, const long lruN)
{
	pageNumber = pgN;
	lruNumber = lruN;
}

bool PageRecord::operator==(PageRecord& pRecord) const
{
	return (pageNumber == pRecord.getPageNumber());
}

bool PageRecord::operator<(PageRecord& pRecord) const
{
	return (pageNumber < pRecord.getPageNumber());
}

bool PageRecordLRU::operator==(PageRecord& pRecordLRU) const
{
	return (lruNumber == pRecordLRU.getLRUNumber());
}

bool PageRecordLRU::operator<(PageRecord& pRecordLRU) const
{
	return (lruNumber < pRecordLRU.getLRUNumber());
}

const long PageRecord::getPageNumber(void) const
{
	return pageNumber;
}

const long PageRecord::getLRUNumber(void) const
{
	return lruNumber;
}

void PageRecord::setLRUNumber(const long lruN)
{
	lruNumber = lruN;
}

redblacknode<PageRecord>* findPageInTree(redblacknode<PageRecord>* node, 
	redblacktree<redblacknode<PageRecord> > *tree)
{
	redblacknode<PageRecord> *root = tree->root;
	return nodeTree->locatenode(findNode, root);
}

redblacknode<PageRecord>* locatePR(long PageNumber, void* tree)
{
	PageRecordTree *prTree;
	redblacknode<PageRecord> *finder;
	PageRecord prFind = PageRecord(pageNumber, -1);
	prTree = static_cast<PageRecordTree *>(tree);
	finder = new redblacknode<PageRecord>(prFind)
	pageNode = findPageInTree(prFind, prTree->pageRecordTree);
	delete finder;
	return pageNode;
}

redblacknode<PageRecordLRU>* locateLRU(long PageNumber, void* tree)
{
	PageRecordTree *prTree;
	redblacknode<PageRecord> *finder, *pageNode, *lruNode;
	redblacknode<PageRecordLRU> *finderLRU;

	PageRecord prFind = PageRecord(pageNumber, -1);
	prTree = static_cast<PageRecordTree *>(tree);
	finder = new redblacknode<PageRecord>(prFind)
	
	pageNode = findPageInTree(finder, prTree->pageRecordTree);
	delete finder;	
	PageRecordLRU lruFind = PageRecordLRU(pageNumber, pageNode->getLRUN());
	finderLRU = new redblacknode<PageRecordLRU>(lruFind);
	lruNode = findPageInTree(finderLRU, prTree->pageRecordLRUTree);
	delete finderLRU;

	return lruNode;
}

extern "C" {

void* createPageTree(void)
{
	redblacktree<redblacknode<PageRecord> >* treePR;
	redblacktree<redblacknode<PageRecordLRU> >* treeLRU;
	PageRecordTree prTree;
	treePR = new redblacktree<redblacknode<PageRecord> >();
	treeLRU = new redblacktree<redblacknode<PageRecordLRU> >();
	prTree = new PageRecordTree(treePR, treeLRU)
	return static_cast<void*>(prTree);
}

void removePageTree(void* tree)
{
	PageRecordTree* prTree;
	prTree = static_cast<PageRecordTree>(tree);
	pthread_mutex_lock(&prTree->tree_mutex);
	delete prTree->pageRecordTree;
	delete prTree->pageRecordLRUTree;
	pthread_mutex_unlock(&prTree->tree_mutex);
	delete prTree;
}

void* getRootPageTree(void* tree)
{
	PageRecordTree *prTree;
	prTree = static_cast<PageRecordTree *>tree;
	return static_cast<void*>(prTree->pageRecordTree->root);
}

void* getRootPageTreeLRU(void* tree)
{
	PageRecordTree *prTree;
	prTree = static_cast<PageRecordLRUTree *>tree;
	return static_cast<void*>(prTree->pageRecordLRUTree->root);
}

	

void insertIntoPageTree(long pageNumber, long lruTime, void* tree)
{
	PageRecordTree *prTree;
	redblacknode<PageRecord> *additionPRNode, *rootPR;
	redblacknode<PageRecordLRU> *additionLRUNode, *rootLRU;

	prTree = static_cast<PageRecordTree *>(tree);
	PageRecord addPR = PageRecord(pageNumber, lruTime);
	PageRecord addPRLRU = PageRecordLRU(pageNumber, lruTime);

	PageRecord addPR = PageRecord(pageNumber, lruTime);
	additionNode = new redblacknode<PageRecord>(addPR);
	additionLRUNode = new redblacknode<PageRecordLRU>(addPRLRU);

	pthread_mutex_lock(&prTree->tree_mutex);
	rootPR = getRootPageTree(tree);
	rootLRU = getRootPageTreeLRU(tree)
	
	prTree->pageRecordTree->insertnode(additionPRNode, rootPR);
	prTree->pageRecordLRUTree->insertnode(additionLRUNode, rootLRU);
	pthread_mutex_unlock(&prTree->tree_mutex);
}

void* locateInPageTreePR(long pageNumber, void* tree)
{
	PageRecordTree *prTree;
	prTree = static_cast<PageRecordTree *>(tree);
	pthread_mutex_lock(&prTree->tree_lock);
	redblacknode<PageRecord> *pageNode = findPR(pageNumber, tree);
	pthread_mutex_unlock(&prTree->tree_lock);
	return static_cast<void *>(pageNode);
}

void* locateInPageTreeLRU(long pageNumber, void* tree)
{
	PageRecordTree *prTree;
	prTree = static_cast<PageRecordTree *>(tree);
	pthread_mutex_lock(&prTree->tree_lock);
	redblacknode<PageRecord> *lruNode = locateLRU(pageNumber, tree);
	pthread_mutex_unlock(&prTree->tree_lock);
	return static_cast<void*>(lruNode);
}


void removeFromPageTree(long pageNumber, void* tree)
{
	PageRecordTree *prTree;
	prTree = static_cast<PageRecordTree *>(tree);
	pthread_mutex_lock(&prTree->tree_lock);
	redblacknode<PageRecord> *pageNode = findPR(pageNumber, tree);
	redblacknode<PageRecordLRU> *lruNode = locateLRU(pageNumber, tree);
	prTree->pageRecordTree->removenode(*pageNode);
	prTree->pageRecordLRUTree->removenode(*lruNode);
	pthread_mutex_unlock(&prTree->tree_lock);
}
//this far
void updateLRU(long pageNumber, long lruTime, void* tree, void* root)
{
	removeFromPageTree(pageNumber, tree, root);
	insertIntoPageTree(pageNumber, lruTime, tree, root);
}

	

}// end extern "C"		
	
	
	 


		
