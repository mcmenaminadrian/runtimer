#include <iostream>
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
	PageRecordTree(redblacktree<redblacknode<PageRecord> >*,
		redblacktree<redblacknode<PageRecordLRU> >*);
};

PageRecordTree::PageRecordTree(redblacktree<redblacknode<PageRecord> >* prTree,
	redblacktree<redblacknode<PageRecordLRU> >* prLRUTree)
{
	pageRecordTree = prTree;
	pageRecordLRUTree = prLRUTree;
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

redblacknode<PageRecord>* 
	findPageInTree(long pageNumber, void *tree, void *root)
{
	redblacknode<PageRecord> *rootNode, *findNode;
	redblacktree<redblacknode<PageRecord> >* nodeTree;

	//set LRU time to -1 - only looking for PageNumber match
	PageRecord addPR = PageRecord(pageNumber, -1);
	findNode = new redblacknode<PageRecord>(addPR);
	
	rootNode = static_cast<redblacknode<PageRecord>*>(root);
	nodeTree = static_cast<redblacktree<redblacknode<PageRecord> >*>(tree);
	return nodeTree->locatenode(findNode, rootNode);
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
	redblacktree<redblacknode<PageRecord> >* rbtree;
	rbtree = (static_cast<redblacktree<redblacknode<PageRecord> >* >(tree));
	delete rbtree;
}

void* getrootPageTree(void* tree)
{
	redblacktree<redblacknode<PageRecord> >* nodetree =
		static_cast<redblacktree<redblacknode<PageRecord> >*>(tree);
	return static_cast<void*>(nodetree->root);
}

void insertIntoPageTree(long pageNumber, long lruTime, void* tree, void* root)
{
	redblacknode<PageRecord> *rootNode, *additionalNode;
	redblacktree<redblacknode<PageRecord> >* nodeTree;

	PageRecord addPR = PageRecord(pageNumber, lruTime);
	additionalNode = new redblacknode<PageRecord>(addPR);
	
	rootNode = static_cast<redblacknode<PageRecord>*>(root);
	nodeTree = static_cast<redblacktree<redblacknode<PageRecord> >*>(tree);
	nodeTree->insertnode(additionalNode, rootNode);
}

void* locateInPageTree(long pageNumber, void* tree, void* root)
{
	
	redblacknode<PageRecord> *pageNode;
	pageNode = findPageInTree(pageNumber, tree, root);
	return static_cast<void *>(pageNode);
}

void removeFromPageTree(long pageNumber, void* tree, void* root)
{
	redblacknode<PageRecord> *pageNode;
	redblacktree<redblacknode<PageRecord> >* nodeTree;
	pageNode = findPageInTree(pageNumber, tree, root);
	nodeTree = static_cast<redblacktree<redblacknode<PageRecord> >*>(tree);
	nodeTree->removenode(*pageNode);
}

void updateLRU(long pageNumber, long lruTime, void* tree, void* root)
{
	removeFromPageTree(pageNumber, tree, root);
	insertIntoPageTree(pageNumber, lruTime, tree, root);
}

	

}// end extern "C"		
	
	
	 


		
