/**
 ******************************************************************************
 * @file      BinarySearchTree.c
 *
 * @brief     Implementation file for BinarySearchTree
 ******************************************************************************
 */

/*
 ------------------------------------------------------------------------------
 Include files
 ------------------------------------------------------------------------------
 */

#include "BinarySearchTree.h"
#include "IBinarySearchTree.h"
#include "ISoftwareException.h"

/*
 ------------------------------------------------------------------------------
 Private function prototypes
 ------------------------------------------------------------------------------
 */

/*
 ******************************************************************************
 * @brief   Create new BST node
 ******************************************************************************
 */
static tIBinarySearchTree_Node* CreateNode( tIBinarySearchTree *pBST, void *pData );

/*
 ******************************************************************************
 * @brief   Remove BST node
 ******************************************************************************
 */
static tIBinarySearchTree_Node* RemoveNode( tIBinarySearchTree* pBST,
                                            tIBinarySearchTree_Node* pRoot,
                                            void* pData );

/*
 ******************************************************************************
 * @brief   Invalidate node in buffer (i.e. indicate that it has been 'removed')
 ******************************************************************************
 */
static void InvalidateNode( tIBinarySearchTree* pBST, tIBinarySearchTree_Node* pRoot );

/*
 ******************************************************************************
 * @brief   Find node with minimum data
 ******************************************************************************
 */
static tIBinarySearchTree_Node* FindMinimumNode( tIBinarySearchTree_Node* pRoot );

/*
 ******************************************************************************
 * @brief   Find in-order predecessor/successor relative a reference node
 ******************************************************************************
 */
static void FindInorderPreAndSuccNodes( tIBinarySearchTree* pBST,
                                        tIBinarySearchTree_Node* pRoot,
                                        tIBinarySearchTree_Node* pRefNode,
                                        tIBinarySearchTree_Node** ppPreNode,
                                        tIBinarySearchTree_Node** ppSuccNode );

/*
 ------------------------------------------------------------------------------
 Interface functions
 ------------------------------------------------------------------------------
 */

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IBinarySearchTree_Init( tIBinarySearchTree *pBST,
                             uint16 maxSize,
                             void* pMemBuffer,
                             tIBinarySearchTree_CompareFun compareFun )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pBST );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pMemBuffer );

    pBST->size = 0;
    pBST->maxSize = maxSize;
    pBST->compareFun = compareFun;
    pBST->root = NULL;
    pBST->nodeBuffer = pMemBuffer;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IBinarySearchTree_DeInit( tIBinarySearchTree *pBST )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pBST );

    pBST->size = 0;
    pBST->maxSize = 0;
    pBST->compareFun = NULL;
    pBST->root = NULL;
    pBST->nodeBuffer = NULL;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tIBinarySearchTree_Node* IBinarySearchTree_Insert( tIBinarySearchTree *pBST, void *pData )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pBST );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pData );

    if ( pBST->size >= pBST->maxSize )
    {
        return NULL;
    }

    if ( NULL == pBST->root )
    {
        /* Create root node if no other node exists in BST */
        tIBinarySearchTree_Node* pRoot = CreateNode( pBST, pData );

        pBST->root = pRoot;

        return pRoot;
    }

    bool isLeft = false;
    tIBinarySearchTree_CompareResult compareResult = BST_COMPARE_EQ;
    tIBinarySearchTree_Node* cursor = pBST->root;
    tIBinarySearchTree_Node* prev   = NULL;

    while ( cursor != NULL )
    {
        compareResult = pBST->compareFun( pData, cursor->data );
        prev = cursor;

        if ( compareResult == BST_COMPARE_MAX )
        {
            /* pData is greater compared to cursor data,
             * should be inserted to right */
            isLeft = false;
            cursor = cursor->right;
        }
        else if ( compareResult == BST_COMPARE_MIN )
        {
            /* pData is smaller compared to cursor data,
             * should be inserted to left */
            isLeft = true;
            cursor = cursor->left;
        }
        else
        {
            /* pData already in BST, don't insert it again */
            return cursor;
        }
    }

    /* Create new node to be inserted in BST */
    tIBinarySearchTree_Node* pNewNode = CreateNode( pBST, pData );

    if ( isLeft )
    {
        /* Update left subtree with new node */
        prev->left = pNewNode;
    }
    else
    {
        /* Update right subtree with new node */
        prev->right = pNewNode;
    }

    return pNewNode;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
void IBinarySearchTree_Remove( tIBinarySearchTree *pBST, void *pData )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pBST );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pData );

    if ( NULL == pBST->root )
    {
        /* BST empty */
        return;
    }

    pBST->root = RemoveNode( pBST, pBST->root, pData );
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tIBinarySearchTree_Node* IBinarySearchTree_Search( tIBinarySearchTree *pBST, void *pData )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pBST );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pData );

    if ( NULL == pBST->root )
    {
        /* BST empty */
        return NULL;
    }

    tIBinarySearchTree_CompareResult compareResult = BST_COMPARE_EQ;
    tIBinarySearchTree_Node* cursor = pBST->root;

    while ( cursor != NULL )
    {
        compareResult = pBST->compareFun( pData, cursor->data );

        if ( compareResult == BST_COMPARE_MIN )
        {
            cursor = cursor->left;
        }
        else if ( compareResult == BST_COMPARE_MAX )
        {
            cursor = cursor->right;
        }
        else
        {
            /* Node containing data found in BST */
            return cursor;
        }
    }

    /* Did not find matching node in BST */
    return NULL;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tIBinarySearchTree_Node* IBinarySearchTree_Next( tIBinarySearchTree *pBST,
                                                 tIBinarySearchTree_Node* pRefNode )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pBST );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pRefNode );

    static tIBinarySearchTree_Node* pPreNode = NULL; /* Not used */
    static tIBinarySearchTree_Node* pSuccNode = NULL;

    if ( NULL == pBST->root )
    {
        /* BST empty */
        return NULL;
    }

    pPreNode = NULL;
    pSuccNode = NULL;

    FindInorderPreAndSuccNodes( pBST,
                                pBST->root,
                                pRefNode,
                                &pPreNode,
                                &pSuccNode );

    return pSuccNode;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
tIBinarySearchTree_Node* IBinarySearchTree_Previous( tIBinarySearchTree *pBST,
                                                     tIBinarySearchTree_Node* pRefNode )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pBST );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pRefNode );

    static tIBinarySearchTree_Node* pPreNode = NULL;
    static tIBinarySearchTree_Node* pSuccNode = NULL; /* Not used */

    if ( NULL == pBST->root )
    {
        /* BST empty */
        return NULL;
    }

    pPreNode = NULL;
    pSuccNode = NULL;

    FindInorderPreAndSuccNodes( pBST,
                                pBST->root,
                                pRefNode,
                                &pPreNode,
                                &pSuccNode );

    return pPreNode;
}

/*
 ------------------------------------------------------------------------------
 Private functions
 ------------------------------------------------------------------------------
 */

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static tIBinarySearchTree_Node* CreateNode( tIBinarySearchTree *pBST, void *pData )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pData );

    tIBinarySearchTree_Node node;

    node.data    = pData;
    node.left    = NULL;
    node.right   = NULL;
    node.isValid = true;

    /* Check if any invalid node exists,
     * replace at index if that's the case */
    for ( int i = 0; i < pBST->size; i++ )
    {
        if ( !pBST->nodeBuffer[ i ].isValid )
        {
            pBST->nodeBuffer[ i ] = node;
            tIBinarySearchTree_Node* newNode = &(pBST->nodeBuffer[ i ]);
            pBST->size++;

            return newNode;
        }
    }

    /* No invalid node exists,
     * create node at last position in buffer */
    pBST->nodeBuffer[ pBST->size ] = node;
    tIBinarySearchTree_Node* newNode = &(pBST->nodeBuffer[ pBST->size ]);
    pBST->size++;

    return newNode;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static tIBinarySearchTree_Node* RemoveNode( tIBinarySearchTree* pBST,
                                            tIBinarySearchTree_Node* pRoot,
                                            void* pData )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pBST );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pData );

    if ( NULL == pRoot )
    {
        return pRoot;
    }

    tIBinarySearchTree_CompareResult compareResult = pBST->compareFun( pData, pRoot->data );

    if ( compareResult == BST_COMPARE_MIN )
    {
        /* pData is smaller compared to cursor data,
         * continue search left subtree for node to be removed */
        pRoot->left = RemoveNode( pBST, pRoot->left, pData );
    }
    else if ( compareResult == BST_COMPARE_MAX )
    {
        /* pData is greater compared to cursor data,
         * continue search right subtree for node to be removed */
        pRoot->right = RemoveNode( pBST, pRoot->right, pData );
    }
    else
    {
        if ( NULL == pRoot->left )
        {
            tIBinarySearchTree_Node* tempNode = pRoot->right;
            InvalidateNode( pBST, pRoot );
            return tempNode;
        }
        else if ( NULL == pRoot->right )
        {
            tIBinarySearchTree_Node* tempNode = pRoot->left;
            InvalidateNode( pBST, pRoot );
            return tempNode;
        }
        /* Two children found (one in each subtree) */
        else
        {
            tIBinarySearchTree_Node *tempNode = FindMinimumNode( pRoot->right );
            pRoot->data = tempNode->data;
            pRoot->right = RemoveNode( pBST, pRoot->right, tempNode->data );
        }
    }

    return pRoot;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static void InvalidateNode( tIBinarySearchTree* pBST, tIBinarySearchTree_Node* pRoot )
{
    SOFTWARE_EXCEPTION_ASSERT( NULL != pBST );
    SOFTWARE_EXCEPTION_ASSERT( NULL != pRoot );

    /* Invalidate to indicate node as 'removed' */
    pRoot->isValid = false;

    /* Reduce BST size */
    pBST->size--;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static tIBinarySearchTree_Node* FindMinimumNode( tIBinarySearchTree_Node* pRoot )
{
    tIBinarySearchTree_Node* pCurrent = pRoot;

    /* Find leftmost leaf node (i.e. minimum node) */
    while ( pCurrent && NULL != pCurrent->left )
    {
        pCurrent = pCurrent->left;
    }

    return pCurrent;
}

/*
 ******************************************************************************
 * Function
 ******************************************************************************
 */
static void FindInorderPreAndSuccNodes( tIBinarySearchTree* pBST,
                                        tIBinarySearchTree_Node* pRoot,
                                        tIBinarySearchTree_Node* pRefNode,
                                        tIBinarySearchTree_Node** ppPreNode,
                                        tIBinarySearchTree_Node** ppSuccNode )
{
    /* Base case */
    if ( NULL == pRoot )
    {
        return;
    }

    tIBinarySearchTree_CompareResult compareResult = pBST->compareFun( pRoot->data, pRefNode->data );

    /* Check if reference node data is present at current root */
    if ( compareResult == BST_COMPARE_EQ )
    {
        /* Maximum value in left subtree is predecessor */
        if ( NULL != pRoot->left )
        {
            tIBinarySearchTree_Node* temp = pRoot->left;

            while ( temp->right )
            {
                temp = temp->right;
            }

            *ppPreNode = temp;
        }

        /* Minimum value in right subtree is successor */
        if ( NULL != pRoot->right )
        {
            tIBinarySearchTree_Node* temp = pRoot->right;

            while ( temp->left )
            {
                temp = temp->left;
            }

            *ppSuccNode = temp;
        }

        return;
    }

    /* If reference node's data is smaller than current root's data,
     * continue to search in left subtree */
    if ( compareResult == BST_COMPARE_MAX )
    {
        *ppSuccNode = pRoot;
        FindInorderPreAndSuccNodes( pBST, pRoot->left, pRefNode, ppPreNode, ppSuccNode );
    }
    /* Continue to search in right subtree */
    else
    {
        *ppPreNode = pRoot;
        FindInorderPreAndSuccNodes( pBST, pRoot->right, pRefNode, ppPreNode, ppSuccNode );
    }
}
