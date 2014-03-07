#include <iostream>
#include <ctime>
#include <vector>
#include <stdexcept>
#include "pthread.h"
#include "redblack.hpp"
#include "threadhandler.h"

using namespace std;

class PageRecord {
	protected:
	long pageNumber;
	time_t lruNumber;
	public:
	PageRecord(const long pgN, const time_t lruN);
	const long getPageNumber(void) const;
	const time_t getLRUNumber(void) const;
	void setLRUNumber(const time_t lruN);
	virtual bool operator==(PageRecord& pR) const;
	virtual bool operator<(PageRecord& pR) const;
};

class PageRecordLRU {
	protected:
	time_t lruNumber;
	vector<long> pageNumbers;
	public:
	PageRecordLRU(const long pgN, const time_t lruN);
	vector<long>& getPageNumbers(void);
	void setPageNumbers(const vector<long> vectPN);
	const time_t getLRUNumber(void) const;
	void setLRUNumber(const time_t lruN);
	void addPage(const long pgN);
	virtual bool operator==(PageRecordLRU& pRLRU) const;
	virtual bool operator<(PageRecordLRU& pRLRU) const;
};

class PageRecordTree {
	public:
	redblacktree<redblacknode<PageRecord> >* pageRecordTree;
	redblacktree<redblacknode<PageRecordLRU> >* pageRecordLRUTree;
	pthread_mutex_t tree_lock;
	PageRecordTree(redblacktree<redblacknode<PageRecord> >* prTree,
		redblacktree<redblacknode<PageRecordLRU> >* prLRUTree);
};

PageRecordTree::PageRecordTree(redblacktree<redblacknode<PageRecord> >* prTree,
	redblacktree<redblacknode<PageRecordLRU> >* prLRUTree)
{
	pageRecordTree = prTree;
	pageRecordLRUTree = prLRUTree;
	pthread_mutex_init(&tree_lock, NULL);
}

PageRecord::PageRecord(const long pgN, const time_t lruN)
{
	pageNumber = pgN;
	lruNumber = lruN;
}

PageRecordLRU::PageRecordLRU(const long pgN, const time_t lruN)
{
	lruNumber = lruN;
	pageNumbers.push_back(pgN);
}

vector<long>& PageRecordLRU::getPageNumbers(void)
{
	return pageNumbers;
}

void PageRecordLRU::setPageNumbers(const vector<long> pgNumbers)
{
	pageNumbers.resize(0);
	pageNumbers = pgNumbers;
}

const time_t PageRecordLRU::getLRUNumber(void) const
{
	return lruNumber;
}

void PageRecordLRU::setLRUNumber(const time_t lruN)
{
	lruNumber = lruN;
}

void PageRecordLRU::addPage(const long pgN)
{
	pageNumbers.push_back(pgN);
}

bool PageRecord::operator==(PageRecord& pRecord) const
{
	return (pageNumber == pRecord.getPageNumber());
}

bool PageRecord::operator<(PageRecord& pRecord) const
{
	return (pageNumber < pRecord.getPageNumber());
}

bool PageRecordLRU::operator==(PageRecordLRU& pRecordLRU) const
{
	return (lruNumber == pRecordLRU.getLRUNumber());
}

bool PageRecordLRU::operator<(PageRecordLRU& pRecordLRU) const
{
	return (lruNumber < pRecordLRU.getLRUNumber());
}

const long PageRecord::getPageNumber(void) const
{
	return pageNumber;
}

const time_t PageRecord::getLRUNumber(void) const
{
	return lruNumber;
}

void PageRecord::setLRUNumber(const time_t lruN)
{
	lruNumber = lruN;
}

redblacknode<PageRecord>* findPagePRInTree(redblacknode<PageRecord>* node, 
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

redblacknode<PageRecord>* locatePR(const long pageNumber,
	PageRecordTree* prTree)
{
	redblacknode<PageRecord> *finder, *pageNode;
	PageRecord prFind = PageRecord(pageNumber, 0);
	finder = new redblacknode<PageRecord>(prFind);
	pageNode = findPagePRInTree(finder, prTree->pageRecordTree);
	delete finder;
	return pageNode;
}

redblacknode<PageRecordLRU>* locateLRU(const time_t timeFind,
	PageRecordTree* prTree)
{
	redblacknode<PageRecordLRU> *finder, *lruNode;
	PageRecordLRU lruFind = PageRecordLRU(0, timeFind);
	finder = new redblacknode<PageRecordLRU>(lruFind);
	lruNode = findPageLRUInTree(finder, prTree->pageRecordLRUTree);
	delete finder;
	return lruNode;
}

void 
buildPageChain(struct PageChain** headChain,
	struct PageChain** activeChain, redblacknode<PageRecord>* node)
{
	if (node == NULL) {
		return;
	}
	buildPageChain(headChain, activeChain, node->left);
	struct PageChain *nextChain =
		(struct PageChain*)malloc(sizeof(struct PageChain));
	nextChain->next = NULL;
	nextChain->page = node->getvalue().getPageNumber();
	if (*activeChain == NULL) {
		*activeChain = nextChain;
		*headChain = *activeChain;
	} else {
		(*activeChain)->next = nextChain;
		*activeChain = nextChain;
	}
	buildPageChain(headChain, activeChain, node->right);
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

void insertIntoPageTree(long pageNumber, time_t lruTime, void* tree)
{
	PageRecordTree *prTree;
	redblacknode<PageRecord> *additionPRNode;
	redblacknode<PageRecordLRU> *additionLRUNode;

	prTree = static_cast<PageRecordTree *>(tree);
	PageRecord addPR = PageRecord(pageNumber, lruTime);
	PageRecordLRU addLRU = PageRecordLRU(pageNumber, lruTime);
	additionPRNode = new redblacknode<PageRecord>(addPR);
	additionLRUNode = new redblacknode<PageRecordLRU>(addLRU);
	pthread_mutex_lock(&prTree->tree_lock);
	prTree->pageRecordTree->insertnode(additionPRNode,
		prTree->pageRecordTree->root);
	redblacknode<PageRecordLRU>* foundLRU = locateLRU(lruTime, prTree);
	if (foundLRU) {
		foundLRU->getvalue().addPage(pageNumber);
		delete additionLRUNode;
	} else {	
		prTree->pageRecordLRUTree->insertnode(additionLRUNode,
			prTree->pageRecordLRUTree->root);
	}
	pthread_mutex_unlock(&prTree->tree_lock);
}

void* locatePageTreePR(long pageNumber, void* tree)
{
	PageRecordTree *prTree;
	prTree = static_cast<PageRecordTree *>(tree);
	pthread_mutex_lock(&prTree->tree_lock);
	redblacknode<PageRecord> *pageNode = locatePR(pageNumber, prTree);
	pthread_mutex_unlock(&prTree->tree_lock);
	return static_cast<void*>(pageNode);
}

void removeFromPageTree(long pageNumber, void* tree)
{
	PageRecordTree *prTree;
	prTree = static_cast<PageRecordTree *>(tree);
	pthread_mutex_lock(&prTree->tree_lock);
	redblacknode<PageRecord> *pageNode = locatePR(pageNumber, prTree);
	if (!pageNode) {
		throw runtime_error("Could not locate Page item");
	}
	redblacknode<PageRecordLRU>* foundLRU = locateLRU(
		pageNode->getvalue().getLRUNumber(), prTree);
	if (!foundLRU) {
		throw runtime_error("Could not locate LRU item.");
	}
	vector<long> pages = foundLRU->getvalue().getPageNumbers();
	bool gotPage = false;
	for (auto it = pages.begin(); it != pages.end(); ++it) {
		if (*it == pageNumber) {
			pages.erase(it);
			gotPage = true;
			break;
		}
	}
	if (!gotPage) {
		throw runtime_error("Page does not seem to exist in LRU heap");
	}
	printf("NOES\n");if (!(prTree->pageRecordTree->removenode(*pageNode))) {
		throw runtime_error("Attempting to remove non-existant node");
	} printf("YES\n");
	pthread_mutex_unlock(&prTree->tree_lock);
}

void* removeOldestFromPageTree(void* tree)
{
	PageRecordTree *prTree;
	redblacknode<PageRecord> *pageNode;
	redblacknode<PageRecordLRU> *lruNode;

	prTree = static_cast<PageRecordTree *>(tree);
	pthread_mutex_lock(&prTree->tree_lock);
	redblacknode<PageRecordLRU> *oldest = prTree->pageRecordLRUTree->min();
	if (oldest == NULL) {
		pthread_mutex_unlock(&prTree->tree_lock);
		return NULL;
	}
	vector<long> pagesList = oldest->getvalue().getPageNumbers();
	if (pagesList.size() > 1) {
		long pageToKill = *(pagesList.begin());
		pagesList.erase(pagesList.begin());
		pageNode = locatePR(pageToKill, prTree);
		prTree->pageRecordTree->removenode(*pageNode);
	} else {
		//have to kill LRU node also
		long pageToKill = *(pagesList.begin());
		pagesList.resize(0);
		lruNode = locateLRU(oldest->getvalue().getLRUNumber(), prTree);
		prTree->pageRecordLRUTree->removenode(*lruNode);
		pageNode = locatePR(pageToKill, prTree);
		prTree->pageRecordTree->removenode(*pageNode);	
	}
	pthread_mutex_unlock(&prTree->tree_lock);
	return oldest;
}

int countPageTree(void* tree)
{
	PageRecordTree *prTree;
	prTree = static_cast<PageRecordTree *>(tree);
	return prTree->pageRecordTree->count();
}

void updateLRU(long pageNumber, time_t lruTime, void* tree)
{
	printf("UU\n");removeFromPageTree(pageNumber, tree);printf("VV\n");
	insertIntoPageTree(pageNumber, lruTime, tree);printf("WW\n");
}

struct PageChain* getPageChain(void *tree)
{
	PageRecordTree *prTree;
	struct PageChain* activeChain = NULL;
	struct PageChain* headChain = NULL;
	prTree = static_cast<PageRecordTree *>(tree);
	buildPageChain(&headChain, &activeChain,
		prTree->pageRecordTree->root);
	return activeChain;
}

void cleanPageChain(struct PageChain* inChain)
{
	if (inChain == NULL){
		return;
	}
	struct PageChain* nextChain = inChain->next;
	delete inChain;
	cleanPageChain(nextChain);
}

}// end extern "C"		
