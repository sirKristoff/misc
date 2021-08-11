/**
 ******************************************************************************
 * @file      IBinarySearchTree.h
 *
 * @brief     BinarySearchTree interface
 ******************************************************************************
 */

#ifndef IBINARYSEARCHTREE_H
#define IBINARYSEARCHTREE_H

/*
 ------------------------------------------------------------------------------
 Include files
 ------------------------------------------------------------------------------
 */

#include "RoboticTypes.h"

/*
 ------------------------------------------------------------------------------
 Defines
 ------------------------------------------------------------------------------
 */

/*
 ------------------------------------------------------------------------------
 Type definitions
 ------------------------------------------------------------------------------
 */

typedef enum
{
    BST_COMPARE_MIN = -1,
    BST_COMPARE_EQ  = 0,
    BST_COMPARE_MAX = 1
} tIBinarySearchTree_CompareResult;

/* Function for comparing data of two nodes in BST.
 * Returns BST_COMPARE_MIN if pDataA < pDataB.
 * Returns BST_COMPARE_EQ if pDataA == pDataB.
 * Returns BST_COMPARE_MAX if pDataA > pDataB. */
typedef tIBinarySearchTree_CompareResult (*tIBinarySearchTree_CompareFun)( const void *pDataA, const void *pDataB );

typedef struct tIBinarySearchTree_Node
{
    void* data;                                /* Node data */
    struct tIBinarySearchTree_Node* left;      /* Pointer to left subtree */
    struct tIBinarySearchTree_Node* right;     /* Pointer to right subtree */
    bool isValid;                              /* Node status */
} tIBinarySearchTree_Node;

typedef struct tIBinarySearchTree
{
    uint16 size;                               /* Current number of nodes in BST */
    uint16 maxSize;                            /* Maximum number of nodes in BST */
    tIBinarySearchTree_CompareFun compareFun;  /* Function for comparing data of two nodes */
    tIBinarySearchTree_Node* root;             /* Root node in BST */
    tIBinarySearchTree_Node* nodeBuffer;       /* Pointer to buffer where nodes are stored */
} tIBinarySearchTree;

/*
-------------------------------------------------------------------------------
 Interface functions
-------------------------------------------------------------------------------
*/

/**
 ******************************************************************************
 * @brief   Initializes the BST by providing memory buffer for the
 *          tree's nodes.
 *          NOTE: SOFTWARE_ASSERTs that pointers are not null.
 * @param   pBST
 *          The BST to initialize.
 * @param   maxSize
 *          Maximum number of nodes to store in BST.
 * @param   pMemBuffer
 *          Pointer to the provided memory buffer where nodes will be stored.
 * @param   compareFun
 *          Function for comparing data between nodes in the tree.
 ******************************************************************************
 */
void IBinarySearchTree_Init( tIBinarySearchTree *pBST,
                             uint16 maxSize,
                             void* pMemBuffer,
                             tIBinarySearchTree_CompareFun compareFun );

/**
 ******************************************************************************
 * @brief   De-initializes the BST.
 *          NOTE! In case malloc-ed memory is provided in the Init() function
 *          call, the caller **MUST** free it before this function
 *          is called, as the pointer to the buffer is cleared.
 * @param   pBST
 *          The BST to de-initialize
 ******************************************************************************
 */
void IBinarySearchTree_DeInit( tIBinarySearchTree *pBST );

/**
 ******************************************************************************
 * @brief   Inserts a node in the BST.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized.
 * @param   pBST
 *          The BST to add node to.
 * @param   pData
 *          Pointer to data contained in the new inserted node.
 * @return  Pointer to inserted node if successful, otherwise NULL.
 *          NOTE: Duplicates of nodes containing identical data not supported,
 *          i.e. NULL returned if a node with pData is already in BST.
 ******************************************************************************
 */
tIBinarySearchTree_Node* IBinarySearchTree_Insert( tIBinarySearchTree *pBST, void *pData );

/**
 ******************************************************************************
 * @brief   Removes a node from the BST.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized.
 * @param   pBST
 *          The BST to remove node from.
 * @param   pData
 *          Pointer to data contained in node to be removed.
 ******************************************************************************
 */
void IBinarySearchTree_Remove( tIBinarySearchTree *pBST, void *pData );

/**
 ******************************************************************************
 * @brief   Search for a node in the BST.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized.
 * @param   pBST
 *          The BST to search in.
 * @param   pData
 *          Pointer to data contained in node to be searched for.
 * @return  Pointer to node if found in BST, otherwise NULL.
 ******************************************************************************
 */
tIBinarySearchTree_Node* IBinarySearchTree_Search( tIBinarySearchTree *pBST, void *pData );

/**
 ******************************************************************************
 * @brief   Gets in-order successor (i.e. next) node in the BST given
 *          a reference node.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized.
 * @param   pBST
 *          The BST to search in.
 * @param   pRefNode
 *          Pointer to reference node in BST.
 * @return  Pointer to in-order successor node in BST if existing, otherwise NULL.
 ******************************************************************************
 */
tIBinarySearchTree_Node* IBinarySearchTree_Next( tIBinarySearchTree *pBST, tIBinarySearchTree_Node* pRefNode );

/**
 ******************************************************************************
 * @brief   Gets in-order predecessor (i.e. previous) node in the BST given
 *          a reference node.
 *          NOTE: SOFTWARE_ASSERTs that pointers are initialized.
 * @param   pBST
 *          The BST to search in.
 * @param   pRefNode
 *          Pointer to reference node in BST.
 * @return  Pointer to in-order predecessor node in BST if existing, otherwise NULL.
 ******************************************************************************
 */
tIBinarySearchTree_Node* IBinarySearchTree_Previous( tIBinarySearchTree *pBST, tIBinarySearchTree_Node* pRefNode );

#endif /* IBINARYSEARCHTREE_H */
