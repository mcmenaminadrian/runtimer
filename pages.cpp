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
	PageRecordLRU(const long pgN, const long lruN):PageRecord(pgN, lruN){};
	virtual bool operator==(PageRecord& pRLRU) const;
	virtual bool operator<(PageRecord& pRLRU) const;
};

class PageRecordTree {
	public:
	redblacktree<redblacknode<PageRecord> >* pageRecordTree;
	redblacktree<redblacknode<PageRecordLRU> >* pageRecordLRUTree;
	pthread_mutex_t tree_lock;

	PageRecordTree(redblacktree<redblacknode<PageRecord> >*,
		redblacktree<redblacknode<PageRecordLRU> >*);
};

PageRecordTree::PageRecordTree(redblacktree<redblacknode<PageRecord> >* prTree,
	redblacktree<redblacknode<PageRecordLRU> >* prLRUTree)
{
	pageRecordTree = prTree;
	pageRecordLRUTree = prLRUTree;
	pthread_mutex_init(&tree_lock, NULL);
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
	return (tree->locatenode(node, root));
}

redblacknode<PageRecordLRU>*
findPageLRUInTree(redblacknode<PageRecordLRU>* node,
	redblacktree<redblacknode<PageRecordLRU> > *tree)
{
	redblacknode<PageRecordLRU> *root = tree->root;
	return (tree->locatenode(node, root));
}	

redblacknode<PageRecord>* locatePR(long pageNumber, void* tree)
{
	PageRecordTree *prTree;
	redblacknode<PageRecord> *finder, *pageNode;
	PageRecord prFind = PageRecord(pageNumber, -1);
	prTree = static_cast<PageRecordTree *>(tree);
	finder = new redblacknode<PageRecord>(prFind);
	pageNode = findPageInTree(finder, prTree->pageRecordTree);
	delete finder;
	return pageNode;
}

redblacknode<PageRecordLRU>* locateLRU(long pageNumber, void* tree)
{
	PageRecordTree *prTree;
	redblacknode<PageRecord> *finder, *pageNode;
	redblacknode<PageRecordLRU> *finderLRU, *lruNode;

	PageRecord prFind = PageRecord(pageNumber, -1);
	prTree = static_cast<PageRecordTree *>(tree);
	finder = new redblacknode<PageRecord>(prFind);
	
	pageNode = findPageInTree(finder, prTree->pageRecordTree);
	delete finder;
	
	PageRecordLRU lruFind = PageRecordLRU(pageNumber,
		pageNode->getvalue().getLRUNumber());
	finderLRU = new redblacknode<PageRecordLRU>(lruFind);
	lruNode = findPageLRUInTree(finderLRU, prTree->pageRecordLRUTree);
	delete finderLRU;

	return lruNode;
}

extern "C" {

void* createPageTree(void)
{
	redblacktree<redblacknode<PageRecord> >* treePR;
	redblacktree<redblacknode<PageRecordLRU> >* treeLRU;
	PageRecordTree* prTree;
	treePR = new redblacktree<redblacknode<PageRecord> >();
	treeLRU = new redblacktree<redblacknode<PageRecordLRU> >();
	prTree = new PageRecordTree(treePR, treeLRU);
	return static_cast<void*>(prTree);
}

void removePageTree(void* tree)
{
	PageRecordTree* prTree;
	prTree = static_cast<PageRecordTree *>(tree);
	pthread_mutex_lock(&prTree->tree_lock);
	delete prTree->pageRecordTree;
	delete prTree->pageRecordLRUTree;
	pthread_mutex_unlock(&prTree->tree_lock);
	delete prTree;
}

inline void* getRootPageTree(void* tree)
{
	PageRecordTree *prTree;
	prTree = static_cast<PageRecordTree *>(tree);
	return static_cast<void*>(prTree->pageRecordTree->root);
}

inline void* getRootPageTreeLRU(void* tree)
{
	PageRecordTree *prTree;
	prTree = static_cast<PageRecordTree *>(tree);
	return static_cast<void*>(prTree->pageRecordLRUTree->root);
}

void insertIntoPageTree(long pageNumber, long lruTime, void* tree)
{
	PageRecordTree *prTree;
	redblacknode<PageRecord> *additionPRNode;
	redblacknode<PageRecordLRU> *additionLRUNode;

	prTree = static_cast<PageRecordTree *>(tree);
	PageRecord addPR = PageRecord(pageNumber, lruTime);
	PageRecordLRU addPRLRU = PageRecordLRU(pageNumber, lruTime);

	additionPRNode = new redblacknode<PageRecord>(addPR);
	additionLRUNode = new redblacknode<PageRecordLRU>(addPRLRU);

	pthread_mutex_lock(&prTree->tree_lock);
	
	prTree->pageRecordTree->insertnode(additionPRNode,
		prTree->pageRecordTree->root);
	prTree->pageRecordLRUTree->insertnode(additionLRUNode,
		prTree->pageRecordLRUTree->root);
	pthread_mutex_unlock(&prTree->tree_lock);
}

void* locatePageTreePR(long pageNumber, void* tree)
{
	PageRecordTree *prTree;
	prTree = static_cast<PageRecordTree *>(tree);
	pthread_mutex_lock(&prTree->tree_lock);
	redblacknode<PageRecord> *pageNode = locatePR(pageNumber, tree);
	pthread_mutex_unlock(&prTree->tree_lock);
	return static_cast<void*>(pageNode);
}

void* locatePageTreeLRU(long pageNumber, void* tree)
{
	PageRecordTree *prTree;
	prTree = static_cast<PageRecordTree *>(tree);
	pthread_mutex_lock(&prTree->tree_lock);
	redblacknode<PageRecordLRU> *lruNode = locateLRU(pageNumber, tree);
	pthread_mutex_unlock(&prTree->tree_lock);
	return static_cast<void*>(lruNode);
}


void removeFromPageTree(long pageNumber, void* tree)
{
	PageRecordTree *prTree;
	prTree = static_cast<PageRecordTree *>(tree);
	pthread_mutex_lock(&prTree->tree_lock);
	redblacknode<PageRecord> *pageNode = locatePR(pageNumber, tree);
	redblacknode<PageRecordLRU> *lruNode = locateLRU(pageNumber, tree);
	prTree->pageRecordTree->removenode(*pageNode);
	prTree->pageRecordLRUTree->removenode(*lruNode);
	pthread_mutex_unlock(&prTree->tree_lock);
}

void* removeOldestFromPageTree(void* tree)
{
	PageRecordTree *prTree;
	long pageNumber;
	redblacknode<PageRecord> *pageNode;
	redblacknode<PageRecordLRU> *lruNode;

	prTree = static_cast<PageRecordTree *>(tree);
	pthread_mutex_lock(&prTree->tree_lock);
	redblacknode<PageRecordLRU> *oldest = prTree->pageRecordLRUTree->max();
	if (oldest == NULL)
		goto exit;
	pageNumber = oldest->getvalue().getPageNumber();
	pageNode = locatePR(pageNumber, tree);
	lruNode = locateLRU(pageNumber, tree);
	prTree->pageRecordTree->removenode(*pageNode);
	prTree->pageRecordLRUTree->removenode(*lruNode);	
exit:
	pthread_mutex_unlock(&prTree->tree_lock);
	return oldest;
}

int countPageTree(void* tree)
{
	PageRecordTree *prTree;
	prTree = static_cast<PageRecordTree *>(tree);
	return prTree->pageRecordTree->count();
}

void updateLRU(long pageNumber, long lruTime, void* tree)
{
	removeFromPageTree(pageNumber, tree);
	insertIntoPageTree(pageNumber, lruTime, tree);
}

}// end extern "C"		
