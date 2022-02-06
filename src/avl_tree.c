#include "avl_tree.h"
#include <string.h>
#include <stdlib.h>

/* avl树节点定义 */
typedef struct tagAvlNode {
    struct tagAvlNode *parent;  /* 父结点指针 */
    struct tagAvlNode *left;    /* 左子女 */
    struct tagAvlNode *right;   /* 右子女 */
    int height;        /* 当前子树高度 */
    int bFactor;       /* 当前子树的平衡因子 */
    void *key;         /* 数据键值 */
    void *data;        /* 存储的数据 */
} AVL_NODE_S;

/* avl树结构体定义 */
typedef struct tagAvlTree {
    AVL_NODE_S *root;
    COMPARE_FUNC cmpFunc;
} AVL_TREE_S;

/* 创建avl树 */
AVL_TREE_S *AvlTreeCreate(COMPARE_FUNC cmpFunc)
{
    if (cmpFunc == NULL) {
        return NULL;
    }

    AVL_TREE_S *pAvlTree = (AVL_TREE_S *)malloc(sizeof(AVL_TREE_S));
    if (pAvlTree != NULL) {
        pAvlTree->root = NULL;
        pAvlTree->cmpFunc = cmpFunc; /* 设置为用户提供的比较函数 */
    }

    return pAvlTree;
}

/* 生成avl树节点 */
AVL_NODE_S *AvlNodeAlloc(void *key, unsigned int keyLen, void *data, unsigned int dataLen)
{
    /* 入参的合法性由上层调用保证 */
    void *pKey = malloc(keyLen);
    if (pKey == NULL) {
        return NULL;
    }

    void *pData = malloc(dataLen);
    if (pData == NULL) {
        free(pKey);
        return NULL;
    }

    AVL_NODE_S *pAvlNode = (AVL_NODE_S *)malloc(sizeof(AVL_NODE_S));
    if (pAvlNode == NULL) {
        free(pKey);
        free(pData);
        return NULL;
    }

    pAvlNode->parent = NULL;
    pAvlNode->left = NULL;
    pAvlNode->right = NULL;
    pAvlNode->height = 1;
    pAvlNode->bFactor = 0;

    (void)memcpy(pKey, key, keyLen);
    (void)memcpy(pData, data, dataLen);
    pAvlNode->key = pKey;
    pAvlNode->data = pData;

    return pAvlNode;
}

/* 释放avl树节点内存 */
void AvlNodeFree(AVL_NODE_S *pAvlNode)
{
    free(pAvlNode->key);
    free(pAvlNode->data);
    free(pAvlNode);

    return;
}

/* 设置avl节点平衡因子和高度(指以本节点为根的子树的高度) */
void AvlBalanceAndHeightSet(AVL_NODE_S *pAvlNode)
{
    int leftHeight = 0;
    int rightHeight = 0;

    if (pAvlNode->left != NULL) {
        leftHeight = pAvlNode->left->height;
    }

    if (pAvlNode->right != NULL) {
        rightHeight = pAvlNode->right->height;
    }

    pAvlNode->height = 1 + ((leftHeight > rightHeight) ? leftHeight : rightHeight);
    pAvlNode->bFactor = rightHeight - leftHeight;

    return;
}

/* 左单旋 */
void AvlRotateLeft(AVL_NODE_S **ppHead)
{
    AVL_NODE_S *pHeadPre = *ppHead;
    AVL_NODE_S *pHeadNew = pHeadPre->right;

    if (pHeadNew->left != NULL) {
        pHeadNew->left->parent = pHeadPre;
    }
    pHeadPre->right = pHeadNew->left;

    pHeadNew->parent = pHeadPre->parent;
    pHeadPre->parent = pHeadNew;
    pHeadNew->left = pHeadPre;

    AvlBalanceAndHeightSet(pHeadPre);
    AvlBalanceAndHeightSet(pHeadNew);

    *ppHead = pHeadNew;

    return;
}

/* 右单旋 */
void AvlRotateRight(AVL_NODE_S **ppHead)
{
    AVL_NODE_S *pHeadPre = *ppHead;
    AVL_NODE_S *pHeadNew = pHeadPre->left;

    if (pHeadNew->right != NULL) {
        pHeadNew->right->parent = pHeadPre;
    }

    pHeadPre->left = pHeadNew->right;

    pHeadNew->parent = pHeadPre->parent;
    pHeadPre->parent = pHeadNew;
    pHeadNew->right = pHeadPre;

    AvlBalanceAndHeightSet(pHeadPre);
    AvlBalanceAndHeightSet(pHeadNew);

    *ppHead = pHeadNew;

    return;
}

/* 先左后右双旋转 */
void AvlRotateLeftRight(AVL_NODE_S **ppHead)
{
    AVL_NODE_S *pHeadNew = (*ppHead)->left->right;
    AVL_NODE_S *pLeftNew = (*ppHead)->left;
    AVL_NODE_S *pRightNew = *ppHead;
    
    if (pHeadNew->right != NULL) {
        pHeadNew->right->parent = pRightNew;
    }
    pRightNew->left = pHeadNew->right;

    if (pHeadNew->left != NULL) {
        pHeadNew->left->parent = pLeftNew;
    }
    pLeftNew->right = pHeadNew->left;

    pHeadNew->parent = pRightNew->parent;

    pLeftNew->parent = pHeadNew;
    pHeadNew->left = pLeftNew;

    pRightNew->parent = pHeadNew;
    pHeadNew->right = pRightNew;

    AvlBalanceAndHeightSet(pLeftNew);
    AvlBalanceAndHeightSet(pRightNew);
    AvlBalanceAndHeightSet(pHeadNew);

    *ppHead = pHeadNew;

    return;
}

/* 先右后左双旋转 */
void AvlRotateRightLeft(AVL_NODE_S **ppHead)
{
    AVL_NODE_S *pHeadNew = (*ppHead)->right->left;
    AVL_NODE_S *pLeftNew = *ppHead;
    AVL_NODE_S *pRightNew = (*ppHead)->right;
    
    if (pHeadNew->left != NULL) {
        pHeadNew->left->parent = pLeftNew;
    }
    pLeftNew->right = pHeadNew->left;
    
    if (pHeadNew->right != NULL) {
        pHeadNew->right->parent = pRightNew;
    }
    pRightNew->left = pHeadNew->right;

    pHeadNew->parent = pLeftNew->parent;

    pLeftNew->parent = pHeadNew;
    pHeadNew->left = pLeftNew;

    pRightNew->parent = pHeadNew;
    pHeadNew->right = pRightNew;

    AvlBalanceAndHeightSet(pLeftNew);
    AvlBalanceAndHeightSet(pRightNew);
    AvlBalanceAndHeightSet(pHeadNew);

    *ppHead = pHeadNew;

    return;
}

/* 单旋转 */
void AvlRotateSingle(AVL_NODE_S **ppHead, int bFactor)
{
    if (bFactor > 0) {
        return AvlRotateLeft(ppHead);
    } else {
        return AvlRotateRight(ppHead);
    }
}

/* 双旋转 */
void AvlRotateDual(AVL_NODE_S **ppHead, int bFactor)
{
    if (bFactor > 0) {
        return AvlRotateLeftRight(ppHead);
    } else {
        return AvlRotateRightLeft(ppHead);
    }
}

/* 旋转平衡操作 */
AVL_NODE_S *AvlRotateBalance(AVL_NODE_S *pCur, int bFactor, int cbFactor, AVL_NODE_S **ppRoot)
{
    AVL_NODE_S **ppCur;

    if (pCur->parent == NULL) {
        ppCur = ppRoot;
    } else if (pCur->parent->left == pCur) {
        ppCur = &(pCur->parent->left);
    } else {
        ppCur = &(pCur->parent->right);
    }

    if (bFactor * cbFactor >= 0) {
        /* 单旋转 */
        AvlRotateSingle(ppCur, bFactor);
    } else {
        /* 双旋转 */
        AvlRotateDual(ppCur, bFactor);
    }

    return *ppCur;
}

/* avl插入节点后的平衡操作 */
void AvlTreeInsertBalance(AVL_NODE_S **ppRoot, AVL_NODE_S *pNode)
{
    int bFactor;
    AVL_NODE_S *pCur;
    AVL_NODE_S *pChild;

    if (pNode == NULL) {
        return;
    }

    pChild = pNode;
    pCur = pNode->parent;

    while (pCur != NULL) {
        AvlBalanceAndHeightSet(pCur);
        bFactor = pCur->bFactor;
        if (bFactor == 0) {
            /* 无需平衡处理, 直接退出 */
            break;
        }

        if ((bFactor > 1) || (bFactor < -1)) {
            /* 旋转平衡处理 */
            pCur = AvlRotateBalance(pCur, bFactor, pChild->bFactor, ppRoot);
        }

        pChild = pCur;
        pCur = pCur->parent;
    }

    return;
}

/* 插入节点 */
int AvlTreeInsert(AVL_TREE_S *pAvlTree, void *key, unsigned int keySize, void *data,
    unsigned int dataSize)
{
    /* 检查入参合法性 */
    /* 应该还需要检查比较函数是否为空 */
    if ((pAvlTree == NULL) || (key == NULL) || (keySize == 0) || (data == NULL) ||
        (dataSize == 0)) {
        return AVL_ERR;
    }

    AVL_NODE_S *pNewNode = AvlNodeAlloc(key, keySize, data, dataSize);
    if (pNewNode == NULL) {
        return AVL_ERR;
    }

    /* 树为空 */
    if (pAvlTree->root == NULL) {
        pAvlTree->root = pNewNode;
        return AVL_OK;
    }

    int cmpRet;
    AVL_NODE_S *pParent;
    AVL_NODE_S *pNode = pAvlTree->root;
    COMPARE_FUNC pfnCmp = pAvlTree->cmpFunc;
    while (pNode != NULL) {
        cmpRet = pfnCmp(key, pNode->key);
        if (cmpRet == 0) {
            /* 结点存在则直接返回 */
            return AVL_OK;
        }

        pParent = pNode;
        if (cmpRet < 0) {
            pNode = pNode->left;
        } else {
            pNode = pNode->right;
        }
    }

    if (cmpRet < 0) {
        pParent->left = pNewNode;
    } else {
        pParent->right = pNewNode;
    }
    pNewNode->parent = pParent;

    AvlTreeInsertBalance(&pAvlTree->root, pNewNode);

    return AVL_OK;
}

/* 查找节点 */
AVL_NODE_S *AvlNodeFind(AVL_TREE_S *pAvlTree, void *key)
{
    int cmpRet;
    COMPARE_FUNC cmpFunc = pAvlTree->cmpFunc;
    AVL_NODE_S *pAvlNode = pAvlTree->root;

    while (pAvlNode != NULL) {
        cmpRet = cmpFunc(key, pAvlNode->key);
        if (cmpRet == 0) {
            break;
        }
        if (cmpRet < 0) {
            pAvlNode = pAvlNode->left;
        } else {
            pAvlNode = pAvlNode->right;
        }
    }

    return pAvlNode;
}

/* 查找直接前驱 */
AVL_NODE_S *AvlPreNodeFind(AVL_NODE_S *pAvlNode)
{
    AVL_NODE_S *pPreNode = pAvlNode->left;
    while (pPreNode != NULL) {
        pPreNode = pPreNode->right;
    }

    return pPreNode;
}

/* 替换avl树节点 */
void AvlNodeReplace(AVL_NODE_S *pAvlNodeOld, AVL_NODE_S *pAvlNodeNew, AVL_NODE_S **ppRoot)
{
    AVL_NODE_S *pParent;

    pAvlNodeNew->parent = pAvlNodeOld->parent;
    pAvlNodeNew->left = pAvlNodeOld->left;
    pAvlNodeNew->right = pAvlNodeOld->right;

    if (pAvlNodeOld->left != NULL) {
        pAvlNodeOld->left->parent = pAvlNodeNew;
    }

    if (pAvlNodeOld->right != NULL) {
        pAvlNodeOld->right->parent = pAvlNodeNew;
    }

    pParent = pAvlNodeOld->parent;
    if (pParent != NULL) {
        if (pParent->left == pAvlNodeOld) {
            pParent->left = pAvlNodeNew;
        } else {
            pParent->right = pAvlNodeNew;
        }
    } else {
        /* 父节点为空说明为根节点 */
        *ppRoot = pAvlNodeNew;
    }

    return;
}

/* 删除最多只有一个子女的avl树节点 */
void AvlEdgeNodeRemove(AVL_NODE_S *pAvlNode, AVL_NODE_S **ppRoot)
{
    AVL_NODE_S *pChild;
    AVL_NODE_S *pParent = pAvlNode->parent;

    if (pAvlNode->left != NULL) {
        pChild = pAvlNode->left;
    } else {
        pChild = pAvlNode->right;
    }

    if (pChild != NULL) {
        pChild->parent = pParent;
    }

    if (pParent != NULL) {
        if (pParent->left == pAvlNode) {
            pParent->left = pChild;
        } else {
            pParent->right = pChild;
        }
    } else {
        /* 父节点为空说明为根节点 */
        *ppRoot = pChild;
    }

    return;
}

/* 获取高度更高的子树节点 */
AVL_NODE_S *AvlGetHigherChild(AVL_NODE_S *pNode)
{
    int leftHeight = 0;
    int rightHeight = 0;

    if (pNode->left != NULL) {
        leftHeight = pNode->left->height;
    }

    if (pNode->right != NULL) {
        rightHeight = pNode->right->height;
    }

    return (leftHeight > rightHeight) ? (pNode->left) : (pNode->right);
}

/* 删除节点后再平衡 */
void AvlTreeRemoveBalance(AVL_NODE_S *pNode, AVL_NODE_S **ppRoot)
{
    AVL_NODE_S *pCur = pNode;
    AVL_NODE_S *pChild;
    int preHeight;
    int bFactor;

    while (pCur != NULL) {
        preHeight = pCur->height;
        AvlBalanceAndHeightSet(pCur);
        if (pCur->height == preHeight) {
            /* 高度未变化, 无需再向上回溯 */
            break;
        }

        bFactor = pCur->bFactor;
        if ((bFactor > 1) || (bFactor < -1)) {
            /* 此处pChild一定不为空 */
            pChild = AvlGetHigherChild(pCur);
            pCur = AvlRotateBalance(pCur, bFactor, pChild->bFactor, ppRoot);
        }
        pCur = pCur->parent;
    }

    return;
}

/* 删除节点 */
int AvlTreeRemove(AVL_TREE_S *pAvlTree, void *key)
{
    AVL_NODE_S *pAvlNode;
    AVL_NODE_S *pPreNode;
    AVL_NODE_S *pParent;

    /* 参数合法性校验 */
    if ((pAvlTree == NULL) || (key == NULL)) {
        return AVL_ERR;
    }

    /* 先查找节点是否存在 */
    pAvlNode = AvlNodeFind(pAvlTree, key);
    if (pAvlNode == NULL) {
        /* 找不到节点直接返回失败 */
        return AVL_ERR;
    }

    /* 是否具有两个子女，如果有两个子女，使用直接前驱替换 */
    if ((pAvlNode->left != NULL) && (pAvlNode->right != NULL)) {
        /* 查找直接前驱 */
        pPreNode = AvlPreNodeFind(pAvlNode);

        /* 摘除直接前驱节点, pPreNode及其父节点一定不为空 */
        pParent = pPreNode->parent;
        pParent->right = pPreNode->left;
        if (pPreNode->left != NULL) {
            pPreNode->left->parent = pParent;
        }

        /* 使用直接前驱替换 */
        AvlNodeReplace(pAvlNode, pPreNode, &pAvlTree->root);
        AvlBalanceAndHeightSet(pPreNode);
    } else {
        pParent = pAvlNode->parent;
        AvlEdgeNodeRemove(pAvlNode, &pAvlTree->root);
    }

    /* 删除节点 */
    AvlNodeFree(pAvlNode);

    /* 删除节点后再平衡 */
    AvlTreeRemoveBalance(pParent, &pAvlTree->root);

    return AVL_OK;
}

/* 查找节点 */
int AvlTreeGetData(AVL_TREE_S *pAvlTree, void *key, void *dataBuff, unsigned int buffSize)
{
    AVL_NODE_S *pAvlNode = AvlNodeFind(pAvlTree, key);
    if (pAvlNode == NULL) {
        return AVL_ERR;
    }

    (void)memcpy(dataBuff, pAvlNode->data, buffSize);
    return AVL_OK;
}

/* 销毁avl树 */
void AvlTreeDestroy(AVL_TREE_S *pAvlTree)
{
    return;
}

/* 获取avl树高度 */
unsigned int AvlTreeGetHeight(AVL_TREE_S *pAvlTree)
{
    if ((pAvlTree == NULL) || (pAvlTree->root == NULL)) {
        return 0;
    }

    return (unsigned int)pAvlTree->root->height;
}

