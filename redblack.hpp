#include <iostream>
#include <exception>
#include <stdexcept>

using namespace std;

// Copyright Adrian McMenamin 2010 - 2014
// Licensed under the GNU GPL version 2
// or any later version at your discretion

#ifndef _RBTREECPP_
#define _RBTREECPP_

template <typename T>
class redblacknode{

	template <typename Z> friend ostream& operator<<(ostream& os,
		redblacknode<Z>* rbtp);
	template <typename Z> friend void 
		streamrbt(ostream& os, redblacknode<Z>* node);

	private:
		T value;
	
	public:
		int colour;
		redblacknode* up;
		redblacknode* left;
		redblacknode* right;
		redblacknode(T& v);
		redblacknode(redblacknode* node);
		redblacknode(redblacknode& node);
		redblacknode* grandparent() const;
 		redblacknode* uncle() const;
		redblacknode* sibling() const;
		T& getvalue();
		bool bothchildrenblack() const;
		bool equals(redblacknode*) const;
		bool lessthan(redblacknode*) const;
		void assign(redblacknode*);
		void showinorder(redblacknode*) const;
		void showpreorder(redblacknode*) const;
		void showpostorder(redblacknode*) const;
};

template <typename NODE>
class redblacktree {
	private:
		void balanceinsert(NODE*);
		void rotate3(NODE*);
		void rotate2(NODE*);
		void rotate2a(NODE*);
		void rotate1(NODE*);
		void transform2(NODE*);
		void free(NODE*);
		NODE* maxleft(NODE*) const;
		NODE* minright(NODE*) const;
		int countup(NODE*) const;
	public:
		NODE* root;
		NODE* locatenode(NODE*, NODE*) const;  
		void insertnode(NODE*, NODE*);
		bool removenode(NODE&);
		bool find(NODE&) const;
		NODE* min() const;
		NODE* max() const;
		const int count() const;
		redblacktree();
		~redblacktree();
};

template <typename T> T& redblacknode<T>::getvalue()
{
	return value;
}

template <typename T> void redblacknode<T>::showinorder(redblacknode<T>* node)
									const
{
	if (node == NULL)
		return;
	showinorder(node->left);
	cout << node->value << ", ";
	showinorder(node->right);
}

template <typename T> void redblacknode<T>::showpreorder(redblacknode<T>* node)
									const
{
	if (node == NULL)
		return;
	cout << node->value << ", ";
	showpreorder(node->left);
	showpreorder(node->right);
}

template <typename T> void redblacknode<T>::showpostorder(redblacknode<T>* node)
									const
{
	if (node == NULL)
		return;
	showpostorder(node->left);
	showpostorder(node->right);
	cout << node->value << ", ";
}


template <typename T> redblacknode<T>::redblacknode(T& v): value(v)
{
	colour = 1; //red
	up = NULL;
	left = NULL;
	right = NULL;
}

template <typename T> redblacknode<T>::redblacknode(redblacknode* node)
{
	colour = node->colour;
	value = node->value;
	up = NULL;
	left = NULL;
	right = NULL;
}

template <typename T> redblacknode<T>::redblacknode(redblacknode& node)
{
	colour = node.colour;
	value = node.value;
	up = NULL;
	left = NULL;
	right = NULL;
}

template <typename T> redblacknode<T>* redblacknode<T>::sibling() const
{
	if (!up)
		return NULL;
	if (up->left == this)
		return up->right;
	else
		return up->left;
}


template <typename T> redblacknode<T>* redblacknode<T>::grandparent() const
{
	if (up)
		return up->up;
	else
		return NULL;
}

template <typename T> redblacknode<T>* redblacknode<T>::uncle() const
{
	redblacknode* g = grandparent();
	if (g) {
		if (g->left == up)
			return g->right;
		else
			return g->left;
	}
	return NULL;
}

template <typename T> bool redblacknode<T>::bothchildrenblack() const
{
	if (right && right->colour == 1)
		return false;
	if (left && left->colour == 1)
		return false;
	return true;
}


template <typename T> bool redblacknode<T>::equals(redblacknode* rbn) const
{
	if (value == rbn->value)
		return true;
	return false;
}

template <typename T> bool redblacknode<T>::lessthan(redblacknode* rbn) const
{
	if (value < rbn->value)
		return true;
	return false;
}

template <typename T> void redblacknode<T>::assign(redblacknode<T>* v)
{
	value = v->value;
}

template <typename NODE> redblacktree<NODE>::~redblacktree()
{
	free(root);
}

template <typename NODE> void redblacktree<NODE>::free(NODE* v)
{
	if (v == NULL)
		return;
	free(v->left);
	NODE* tmp = v->right;
	delete v;
	free(tmp);
}

template <typename NODE> redblacktree<NODE>::redblacktree()
{
	root = NULL;
}

// turn line of two reds and a black into black with two children
template <typename NODE> void redblacktree<NODE>::rotate2(NODE* node)
{
	if (!node || !node->up)
		return;
	NODE* gp = node->grandparent();
	NODE* par = NULL;
	
	NODE* centrenode = node->up;
	if (gp) {
		par = gp->up;
		if (par) {
			if (par->left == gp)
				par->left = centrenode;
			else
				par->right = centrenode;
		} 
	}
	
	if (node->up->right == node)
	{
		NODE* centreleft = centrenode->left;
		centrenode->colour = 0;
		centrenode->left = gp;
		if (gp) {
			gp->up = centrenode;
			gp->colour = 1;
			gp->right = centreleft;
			if (centreleft)
				centreleft->up = gp;
		}
	} else {
		NODE* centreright = centrenode->right;
		centrenode->colour = 0;
		centrenode->right = gp;
		if (gp) {
			gp->up = centrenode;
			gp->colour = 1;
			gp->left = centreright;
			if (centreright)
				centreright->up = gp;
		}
	}
	centrenode->up = par;
	if (!par)
		root = centrenode;
}

template <typename NODE> void redblacktree<NODE>::rotate3(NODE* node)
{
	if (!node || !node->up)
		return;
	NODE* par = node->up;
	NODE* righty = node->right;
	NODE* lefty = node->left;

	if (par->left == node) {
		par->left = righty;
		righty->colour = 0;
		righty->up = par;
		node->up = righty;
		node->right = righty->left;
		righty->left = node;
		node->colour = 1;
	}
	else {
		par->right = lefty;
		lefty->colour = 0;
		lefty->up = par;
		node->up = lefty;
		node->left = lefty->right;
		lefty->right = node;
		node->colour = 1;
	}
}			

template <typename NODE> void redblacktree<NODE>::rotate2a(NODE* node)
{
	if (!node || !node->up)
		return;
	NODE* par = node->up;
	NODE* gp = node->grandparent();
	NODE* righty = node->right;
	NODE* lefty = node->left;
	if (gp) {
		if (gp->left == par)
			gp->left = node;
		else
			gp->right = node;
		node->up = gp;
	} else {
		root = node;
		node->up = NULL;
	}

	if (par->right == node) {
		node->left = par;
		par->up = node;
		par->right = lefty;
		if (lefty)
			lefty->up = par;

	} else {
		node->right = par;
		par->up = node;
		par->left = righty;
		if (righty)
			righty->up = par;
	}
	par->colour = 1;
	node->colour = 0;
}	

//straighten zig zag of two reds
template <typename NODE> void redblacktree<NODE>::rotate1(NODE* node)
{
	if (!node)
		return;
	NODE* par = node->up;
	NODE* rightnode = node->right;
	NODE* leftnode = node->left;
	NODE* rightleft = NULL;
	NODE* leftright = NULL;

	if (par) {
		if (par->left == node) {
			par->left = rightnode;
			if (rightnode) {
				rightleft = rightnode->left;
				rightnode->up = par;
				rightnode->left = node;
			}
			node->right = rightleft;
			if (rightleft)
				rightleft->up = node;
			node->up = rightnode;
		} else {
			par->right = leftnode;
			if (leftnode) {
				leftright = leftnode->right;
				leftnode->up = par;
				leftnode->right = node;
			}
			node->left = leftright;
			if (leftright)
				leftright->up = node;
			node->up = leftnode;
		}
	}
}

template <typename NODE> void redblacktree<NODE>::transform2(NODE* node)
{
	int oldcolour = node->up->colour;
	rotate2a(node);
	node->colour = oldcolour;
	if (node->left)
		node->left->colour = 0;
	if (node->right)
		node->right->colour = 0;
}

template <typename NODE> void redblacktree<NODE>::balanceinsert(NODE* node)
{
	if (node->up) {
		if (node->up->colour == 0) {
			return;}

		if (node->uncle() && node->uncle()->colour == 1) {
			node->up->colour = 0;
			node->uncle()->colour = 0;
			node->grandparent()->colour = 1;
			balanceinsert(node->grandparent());
		} else {
			
			if (node->grandparent()->left == node->up) {
				if (node->up->right == node){ 
					rotate1(node->up);
					node = node->left;
				}
				rotate2(node);
			} else {
				if (node->up->left == node){
					rotate1(node->up);
					node = node->right;
				}
				rotate2(node);
			}
		}
		return;
	}
	else 
		node->colour = 0;

}

template <typename NODE> void redblacktree<NODE>::insertnode(NODE* insert,
							NODE* node)
{
	if (node == NULL) {
		root = insert;
		root->colour = 0;
		return;
	}
	if (insert->lessthan(node)) { 
		if (node->left == NULL) {
			node->left = insert;
			node->left->up = node;
			node = node->left;
			balanceinsert(node);
		} else 
			insertnode(insert, node->left);
	} else {
		if (node->right == NULL) {
			node->right = insert;
			node->right->up = node;
			node = node->right;
			balanceinsert(node);
		} else
			insertnode(insert, node->right);
	}
}

template <typename NODE> NODE* redblacktree<NODE>::locatenode(NODE* v,
		NODE* node) const
{
	if (node == NULL)
		return node;
	if (v->equals(node))
		return node;
	if (v->lessthan(node))
		return locatenode(v, node->left);
	else
		return locatenode(v, node->right);
}

template <typename NODE> NODE* redblacktree<NODE>::minright(NODE* node) const
{

	if (node->left)
		return minright(node->left);
	else
		return node;
}

template <typename NODE> NODE* redblacktree<NODE>::min() const
{
	if (!root)
		return NULL;
	NODE* p = root;
	do {
		if (!p->left)
			return p;
		p = p->left;
	} while(true);
}

template <typename NODE> NODE* redblacktree<NODE>::max() const
{
	if (!root)
		return NULL;
	NODE* p = root;
	do {
		if (!p->right)
			return p;
		p = p->right;
	} while(true);
}

template <typename NODE> int 
	redblacktree<NODE>::countup(NODE* node) const
{
	int count = 0
	if (node != NULL) {
		count = 1;
		count += count(node->left);
		count += count(node->right);
	}
	return count;
}

template <typename NODE> const int redblacktree<NODE>::count() const
{
	return countup(root);
}

template <typename NODE> NODE* redblacktree<NODE>::maxleft(NODE* node) const
{
	if (node->right)
		return maxleft(node->right);
	else
		return node;
}

template <typename NODE> bool redblacktree<NODE>::find(NODE& v) const
{
	NODE* located = locatenode(&v, root);
	if (located)
		return true;
	else
		return false;
}

template <typename NODE> bool redblacktree<NODE>::removenode(NODE& v)
{
	if (&v == NULL) {
		throw invalid_argument("Attempted to remove NULL node");
	}
	NODE* located = locatenode(&v, root);
	NODE* altnode = NULL;
	if (located == NULL)
		return false;
	
	NODE* lefty = located->left;
	NODE* righty =  located->right;
	if (lefty && righty){
		altnode = maxleft(located->left);
		if (altnode->colour == 0) 
			altnode = minright(located->right);
		located->assign(altnode);
		located = altnode; 
		lefty = located->left;
		righty = located->right;
	}

	//located is now a node with only one child at most
	NODE* par = located->up;
	NODE* sibling = located->sibling();
	NODE* follow = NULL;
	if (lefty)
		follow = lefty;
	else
		follow = righty;

	if (par) {
		if (par->left == located) {
			par->left = follow;
		}
		else {
			par->right = follow;
		}
	}
	else
		root = follow;
	
	if (follow)
		follow->up = par;

	//easy to remove a red
	if (located->colour == 1) {
		delete located;
		return true;
	}

	//also easy if follow is red
	if (follow && follow->colour == 1) {
		follow->colour = 0;
		delete located;
		return true;
	}

	if (!follow && !par)
	{
		//tree is now empty
		delete located;
		root = NULL;
		return true;
	}

	//loop through the fixes
	do {
		if (sibling == root)
		{
			delete located;
			return true;
		}
		//test sibling status
		if (sibling) {
			//red?
			if (sibling->colour == 1) {
				bool leftist = (par->left == sibling);
				rotate2a(sibling);
				sibling->colour = 0;
				par->colour = 1;
				if (follow)
					sibling = follow->sibling(); 
				else {
					if (leftist)
						sibling = par->left;
					else
						sibling = par->right;
				}
			}
			//case above can fall directly into case below
			if (par->colour == 1) {
				if (sibling->bothchildrenblack()) {
					sibling->colour = 1;
					par->colour = 0;
					delete located;
					return true;
				}
			}
			else if (sibling->bothchildrenblack()){
				sibling->colour = 1;
				follow = par;
				sibling = follow->sibling();
				par = follow->up;
				if (par == NULL) {
					//at root can go no further
					delete located;
					return true;
				}
				continue;
			}
			if (par->right == sibling) {
				if (sibling->left &&
					sibling->left->colour == 1
					&& (sibling->right == NULL ||
					(sibling->right &&
					sibling->right->colour == 0))){
						rotate3(sibling);
						sibling = sibling->up;
						continue;
					}
				else {
					transform2(sibling);
					delete located;
					return true;
				}
			}
			else if (par->left == sibling) {
				if (sibling->right &&
					sibling->right->colour == 1 &&
					(sibling->left == NULL ||
					(sibling->left &&
					sibling->left->colour == 0))){
						rotate3(sibling);
						sibling = sibling->up;
						continue;
					}
				else {
					transform2(sibling);
					delete located;
					return true;
				}
			}
		} else {
			if (par->colour == 1) {
				par->colour = 0;
				delete located;
				return true;
			}
			else { 
				if (!follow)
					follow = located->up;	
			else
					follow = follow->up;
				par = follow->up;
			}
		}
		if (follow)
			sibling = follow->sibling();

	}while(true);
}

template <typename T> void streamrbt(ostream& os, redblacknode<T>* node)
{
	if (node == NULL) 
		return;
	os << "(" << node->value;
	if (node->colour == 0)
		os << "[BLACK]";
	else
		os << "[RED]";
	streamrbt(os, node->left);
	cout << ",";
	streamrbt(os, node->right);
	os << ")";
}

template <typename T> ostream& operator<<(ostream& os, redblacknode<T>* rbn)
{
	streamrbt(os, rbn);
	return os;
}		

#endif
