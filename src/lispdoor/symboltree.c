/**
 *   \file symboltree.c
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */

#include "lispdoor/symboltree.h"

#include "lispdoor/gc.h"
#include "lispdoor/print.h"

// symbol table
// ---------------------------------------------------------------
// A utility function to get the height of the tree
static inline int8_t TreeHeight(struct LispSymbol *o) {
  if (o == NULL) return 0;
  return o->height;
}

// A utility function to get maximum of two integers
static inline int8_t TreeMax(int8_t a, int8_t b) { return (a > b) ? a : b; }

// Get Balance factor of node N
int8_t TreeGetBalance(struct LispSymbol *N) {
  if (N == NULL) return 0;
  return TreeHeight(N->right) - TreeHeight(N->left);
}

/*
 * Helper function to update the height of a node, based on the height of
 * its children.
 */
void TreeUpdateHeight(struct LispSymbol *n) {
  // Height is 1 more than the maximum child height.
  n->height = TreeMax(TreeHeight(n->left), TreeHeight(n->right)) + (int8_t)1;
}

/*
 * This function performs a left rotation on the subtree rooted at a given
 * node.  The rotation is centered around the node's right child.  The new
 * subtree root (the rotation's center) is returned.
 */
struct LispSymbol *TreeRotateLeft(struct LispSymbol *n) {
  struct LispSymbol *center = n->right;

  // The center's left child and n "trade places" in the tree.
  n->right = center->left;
  center->left = n;

  TreeUpdateHeight(n);
  TreeUpdateHeight(center);

  return center;
}

/*
 * This function performs a right rotation on the subtree rooted at a given
 * node.  The rotation is centered around the node's right child.  The new
 * subtree root (the rotation's center) is returned.
 */
struct LispSymbol *TreeRotateRight(struct LispSymbol *n) {
  struct LispSymbol *center = n->left;

  // The center's right child and n "trade places" in the tree.
  n->left = center->right;
  center->right = n;

  TreeUpdateHeight(n);
  TreeUpdateHeight(center);

  return center;
}

/*
 * Helper function to compute a node's balance factor, which is the signed
 * difference in heights between its right subtree and left subtree.  A
 * negative balance factor means the node is left-heavy, a positive balance
 * factor means the node is right-heavy, and a zero balance factor means the
 * node is height-balanced.
 */

struct LispSymbol *TreeBalance(struct LispSymbol *n) {
  int8_t bf = TreeGetBalance(n);
  if (bf < -1) {
    /*
     * Here, n is height-imbalanced on the left.  We need a double rotation if
     * n’s left child is right-heavy.
     */
    if (TreeGetBalance(n->left) > 0) {
      n->left = TreeRotateLeft(n->left);
    }
    return TreeRotateRight(n);

  } else if (bf > 1) {
    /*
     * Here, n is height-imbalanced on the right.  We need a double rotation
     * if n’s right child is left-heavy.
     */
    if (TreeGetBalance(n->right) < 0) {
      n->right = TreeRotateRight(n->right);
    }
    return TreeRotateLeft(n);

  } else {
    // Just update the height if we didn’t rotate.
    TreeUpdateHeight(n);
    return n;
  }
}

/* Helper function that allocates a new node with the given key and
    NULL left and right pointers. */
struct LispSymbol *TreeNewLispSymbol(char *str) {
  struct LispSymbol *node =
      (struct LispSymbol *)LispAllocObject(kSymbol, (LispIndex)strlen(str));
  node->value = LISP_UNBOUND;
  node->stype = kSymOrdinary;
  strcpy(node->name, str);
  node->left = node->right = NULL;
  node->height = 0;
  return (node);
}

// Recursive function to insert a key in the subtree rooted
// with node and returns the new root of the subtree.
struct LispSymbol *TreeInsert(struct LispSymbol *n, char *name) {
  static LispFixNum x;

  if (n == NULL) {
    /*
     * If n is NULL, we know we've reached a place to insert val, so we
     * create a new node holding val and return it.
     */
    return TreeNewLispSymbol(name);
  }
  x = strcmp(name, n->name);
  if (x == 0) {
    return n;
  } else if (x < 0) {
    /*
     * If val is less than the value at n, we insert val in n's left subtree
     * (somewhere) and update n->left to point to the modified subtree (with
     * val inserted).
     */
    n->left = TreeInsert(n->left, name);

  } else {
    /*
     * If val is greater than or equal to the value at n, we insert val in n's
     * right subtree (somewhere) and update n->right to point to the modified
     * subtree (with val inserted).
     */
    n->right = TreeInsert(n->right, name);
  }

  /*
   * For the else if and else conditions, the subtree rooted at n has already
   * been modified (by setting n->left or n->right above), so we can just
   * return n here, after balancing n's subtree.
   */
  return TreeBalance(n);
}

/* Function to print binary tree in 2D */
/* It does reverse inorder traversal */
void SymbolTreePrint2D(struct LispSymbol *root, int space) {
  // Base case
  if (root == NULL) return;

  // Increase distance between levels
  space += COUNT;

  // Process right child first
  SymbolTreePrint2D(root->right, space);

  // Print current node after space
  // count
  LispPrintByte('\n');
  for (int i = COUNT; i < space; i++) {
    LispPrintByte(' ');
  }
  LispPrintStr(root->name);
  LispPrintByte('\n');

  // Process left child
  SymbolTreePrint2D(root->left, space);
}

struct LispSymbol *SymTreeLookUp(struct LispSymbol *ptree, char *str) {
  LispFixNum x;

  while (ptree != NULL) {
    x = strcmp(str, (ptree)->name);
    if (x == 0) return ptree;
    if (x < 0)
      ptree = (ptree)->left;
    else
      ptree = (ptree)->right;
  }
  return ptree;
}

LispObject LispMakeSymbol(char *str) {
  struct LispSymbol *pnode;
  struct LispSymbol **symbol_tree =
      (struct LispSymbol **)&LispEnv()->symbol_tree;

  pnode = SymTreeLookUp(*symbol_tree, str);
  if (pnode == NULL) {
    *symbol_tree = TreeInsert(*symbol_tree, str);
    pnode = SymTreeLookUp(*symbol_tree, str);
  }
  return (LispObject)pnode;
}
