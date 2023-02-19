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
    uint8_t buff[0];   /* 保存key和data的变长数组 */
} AVL_NODE_S;

/* avl树结构体定义 */
typedef struct tagAvlTree {
    AVL_NODE_S dumRoot;
    COMPARE_FUNC cmpFunc;
} AVL_TREE_S;

/* 子女类型，左子女或者右子女 */
typedef enum {
    AVL_LEFT = 0,
    AVL_RIGHT = 1
} AvlChildType;

/* key或data长度不超过32字节，则使用循环赋值的方式拷贝 */
#define AVL_BYTES_COPY_LEN 32

/* 创建avl树 */
AVL_TREE_S *AvlTreeCreate(COMPARE_FUNC cmpFunc)
{
    if (cmpFunc == NULL) {
        return NULL;
    }

    AVL_TREE_S *pAvlTree = (AVL_TREE_S *)malloc(sizeof(AVL_TREE_S));
    if (pAvlTree != NULL) {
        (void)memset(pAvlTree, 0, sizeof(AVL_TREE_S));
        pAvlTree->cmpFunc = cmpFunc; /* 设置为用户提供的比较函数 */
    }

    return pAvlTree;
}

static inline AvlBytesCopy(void *dst, void *src, uint32_t len)
{
    uint32_t i;
    uint8_t *curDst = (uint8_t *)dst;
    uint8_t *curSrc = (uint8_t *)src;

    for (i = 0; i < len; i++) {
        *curDst = *curSrc;
        curDst++;
        curSrc++;
    }
}

/* 生成avl树节点 */
AVL_NODE_S *AvlNodeAlloc(void *key, uint32_t keyLen, void *data, uint32_t dataLen)
{
    void *dst;
    AVL_NODE_S *pAvlNode;
    size_t nodeSize = sizeof(AVL_NODE_S) + (size_t)keyLen + (size_t)dataLen;

    /* 入参合法性由上层调用保证 */
    pAvlNode = (AVL_NODE_S *)malloc(nodeSize);
    if (pAvlNode == NULL) {
        return NULL;
    }

    pAvlNode->parent = NULL;
    pAvlNode->left = NULL;
    pAvlNode->right = NULL;
    pAvlNode->height = 1;
    pAvlNode->bFactor = 0;

    dst = (void *)(pAvlNode + 1);
    if (keyLen <= AVL_BYTES_COPY_LEN) {
        AvlBytesCopy(dst, key, keyLen);
    } else {
        (void)memcpy(dst, key, keyLen);
    }
    pAvlNode->key = dst;

    dst = (void *)((uint8_t *)dst + keyLen);
    if (dataLen < AVL_BYTES_COPY_LEN) {
        AvlBytesCopy(dst, data, dataLen);
    } else {
        (void)memcpy(dst, data, dataLen);
    }
    pAvlNode->data = dst;

    return pAvlNode;
}

/* 释放avl树节点内存 */
void AvlNodeFree(AVL_NODE_S *pAvlNode)
{
    /* 由上层调用保证pAvlNode非空 */
    free(pAvlNode);
}

/* 设置avl节点平衡因子和高度(指以本节点为根的子树的高度) */
static inline void AvlBalanceAndHeightSet(AVL_NODE_S *pAvlNode)
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
}

/* 左单旋 */
void AvlRotateLeft(AVL_NODE_S **ppRoot)
{
    AVL_NODE_S *pOldRoot = *ppRoot; 
    AVL_NODE_S *pNewRoot = pOldRoot->right;
    AVL_NODE_S *pGrand = pOldRoot->parent;

    /* 先修改旧根结点的右子树 */
    if (pNewRoot->left != NULL) {
        pNewRoot->left->parent = pOldRoot;
    }
    pOldRoot->right = pNewRoot->left;

    /* 链接新根结点的左边子树 */
    pNewRoot->left = pOldRoot;
    pOldRoot->parent = pNewRoot;

    /* 修改祖父结点的链接 */
    pNewRoot->parent = pGrand;
    *ppRoot = pNewRoot;

    /* 树高度和平衡因子更新 */
    AvlBalanceAndHeightSet(pOldRoot);
    AvlBalanceAndHeightSet(pNewRoot);
}

/* 右单旋 */
void AvlRotateRight(AVL_NODE_S **ppRoot)
{
    AVL_NODE_S *pOldRoot = *ppRoot; 
    AVL_NODE_S *pNewRoot = pOldRoot->left;
    AVL_NODE_S *pGrand = pOldRoot->parent;

    /* 先修改旧根结点的左子树 */
    if (pNewRoot->right != NULL) {
        pNewRoot->right->parent = pOldRoot;
    }
    pOldRoot->left = pNewRoot->right;

    /* 链接新根结点的右子树 */
    pNewRoot->right = pOldRoot;
    pOldRoot->parent = pNewRoot;

    /* 修改祖父结点的链接 */
    pNewRoot->parent = pGrand;
    *ppRoot = pNewRoot;

    /* 修改树高度和平衡因子 */
    AvlBalanceAndHeightSet(pOldRoot);
    AvlBalanceAndHeightSet(pNewRoot);
}

/* 先左后右双旋转 */
void AvlRotateLeftRight(AVL_NODE_S **ppRoot)
{
    AVL_NODE_S *pRoot = *ppRoot;

    /* 先左单旋左子树 */
    AvlRotateLeft(&(pRoot->left));

    /* 再右单旋当前树 */
    AvlRotateRight(ppRoot);
}

/* 先右后左双旋转 */
void AvlRotateRightLeft(AVL_NODE_S **ppRoot)
{
    AVL_NODE_S *pRoot = *ppRoot;

    /* 先右单旋右子树 */
    AvlRotateRight(&(pRoot->right));

    /* 再左单旋当前树 */
    AvlRotateLeft(ppRoot);
}

/* 单旋转 */
void AvlRotateSingle(AVL_NODE_S **ppHead, int bFactor)
{
    if (bFactor > 0) {
        AvlRotateLeft(ppHead);
    } else {
        AvlRotateRight(ppHead);
    }
}

/* 双旋转 */
void AvlRotateDual(AVL_NODE_S **ppHead, int bFactor)
{
    if (bFactor > 0) {
        AvlRotateLeftRight(ppHead);
    } else {
        AvlRotateRightLeft(ppHead);
    }
}

/* 旋转平衡操作 */
AVL_NODE_S *AvlRotateBalance(AVL_NODE_S *pCur, int bFactor, int cbFactor)
{
    AVL_NODE_S **ppCur;

    /* 使用dumy root结点，上层调用保证pCur->parent不为空 */
    if (pCur->parent->left == pCur) {
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
void AvlTreeInsertBalance(AVL_NODE_S *pNode, AVL_NODE_S *pDumRoot)
{
    int bFactor;
    AVL_NODE_S *pCur;
    AVL_NODE_S *pChild;

    /* 上层调用保证pNode一定不为空 */
    pChild = pNode;
    pCur = pNode->parent;

    /* 当前结点非dumRoot则继续回溯 */
    while (pCur != pDumRoot) {
        AvlBalanceAndHeightSet(pCur);
        bFactor = pCur->bFactor;
        if (bFactor == 0) {
            /* 无需平衡处理, 直接退出 */
            break;
        }

        if ((bFactor > 1) || (bFactor < (-1))) {
            /* 旋转平衡处理 */
            pCur = AvlRotateBalance(pCur, bFactor, pChild->bFactor);
        }

        pChild = pCur;
        pCur = pCur->parent;
    }
}

/* 插入节点 */
int AvlTreeInsert(AVL_TREE_S *pAvlTree, void *key, uint32_t keySize, void *data,
    uint32_t dataSize)
{
    /* 检查入参合法性 */
    /* 应该还需要检查比较函数是否为空 */
    if ((pAvlTree == NULL) || (pAvlTree->cmpFunc == NULL) || (key == NULL) || (keySize == 0) ||
        (data == NULL) || (dataSize == 0)) {
        return AVL_ERR;
    }

    AVL_NODE_S *pNewNode = AvlNodeAlloc(key, keySize, data, dataSize);
    if (pNewNode == NULL) {
        return AVL_ERR;
    }

    /* 树为空 */
    if (pAvlTree->dumRoot.left == NULL) {
        pAvlTree->dumRoot.left = pNewNode;
        pNewNode->parent = &pAvlTree->dumRoot;
        return AVL_OK;
    }

    int cmpRet;
    AVL_NODE_S *pParent;
    AVL_NODE_S *pNode = pAvlTree->dumRoot.left;
    COMPARE_FUNC pfnCmp = pAvlTree->cmpFunc;
    while (pNode != NULL) {
        cmpRet = pfnCmp(key, pNode->key);
        if (cmpRet == 0) {
            /* 结点存在则直接返回 */
            /* 释放内存 */
            AvlNodeFree(pNewNode);
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

    AvlTreeInsertBalance(pNewNode, &pAvlTree->dumRoot);
    return AVL_OK;
}

/* 查找节点 */
AVL_NODE_S *AvlNodeFind(AVL_TREE_S *pAvlTree, void *key)
{
    int cmpRet;
    COMPARE_FUNC cmpFunc = pAvlTree->cmpFunc;
    AVL_NODE_S *pAvlNode = pAvlTree->dumRoot.left;

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
void AvlTreeRemoveBalance(AVL_NODE_S *pNode, AVL_NODE_S *pDumRoot)
{
    AVL_NODE_S *pCur = pNode;
    AVL_NODE_S *pChild;
    int preHeight;
    int bFactor;

    while (pCur != pDumRoot) {
        preHeight = pCur->height;
        AvlBalanceAndHeightSet(pCur);
        if (pCur->height == preHeight) {
            /* 高度未变化, 无需再向上回溯 */
            break;
        }

        bFactor = pCur->bFactor;
        if ((bFactor > 1) || (bFactor < (-1))) {
            /* 此处pChild一定不为空 */
            pChild = AvlGetHigherChild(pCur);
            pCur = AvlRotateBalance(pCur, bFactor, pChild->bFactor);
        }
        pCur = pCur->parent;
    }
}

void AvlNodeTransplant(AVL_NODE_S *pNode, AVL_NODE_S **ppParent,
    AvlChildType *pChildType, AVL_NODE_S **ppNewChild)
{
    AvlChildType childType = *pChildType;
    AVL_NODE_S *pPreNode;
    AVL_NODE_S *pNewChild;
    AVL_NODE_S *pParent = *ppParent;
    AVL_NODE_S *pNewParent;
    AVL_NODE_S **ppChild;

    if (childType == AVL_LEFT) {
        ppChild = &pParent->left;
    } else {
        ppChild = &pParent->right;
    }

    /* 上层调用保证此处pPreNode一定不为空 */
    pPreNode = AvlPreNodeFind(pNode);
    pNewChild = pPreNode->left;

    pPreNode->right = pNode->right;
    /* 上层调用保证此处pNode的右子女一定不为空 */
    pNode->right->parent = pPreNode;

    /* 直接前驱是pNode的左子女 */
    if (pPreNode == pNode->left) {
        childType = AVL_LEFT;
        pNewParent = pPreNode;
    } else {
        pPreNode->left = pNode->left;
        /* 上层调用保证此处pNode的左子女一定不为空 */
        pNode->left->parent = pPreNode;
        childType = AVL_RIGHT;
        pNewParent = pPreNode->parent;
    }

    /* 复制高度和平衡因子 */
    pPreNode->height = pNode->height;
    pPreNode->bFactor = pNode->bFactor;

    pPreNode->parent = pParent;
    *ppChild = pPreNode;
    
    *ppParent = pNewParent;
    *pChildType = childType;
    *ppNewChild = pNewChild;
}

int AvlTreeRemove(AVL_TREE_S *pAvlTree, void *key)
{
    AVL_NODE_S *pAvlNode;
    AVL_NODE_S *pPreNode;
    AVL_NODE_S *pParent;
    AVL_NODE_S *pNewChild;
    AvlChildType childType;

    /* 参数合法性校验 */
    if ((pAvlTree == NULL) || (pAvlTree->cmpFunc == NULL) || (key == NULL)) {
        return AVL_ERR;
    }

    /* 先查找节点是否存在 */
    pAvlNode = AvlNodeFind(pAvlTree, key);
    if (pAvlNode == NULL) {
        /* 找不到节点直接返回失败 */
        return AVL_ERR;
    }

    /* dumRoot的使用保证此处pParent一定不为空 */
    pParent = pAvlNode->parent;
    if (pParent->left == pAvlNode) {
        childType = AVL_LEFT;
    } else {
        childType = AVL_RIGHT;
    }

    /* 判断是否有两个子女，如果有两个子女，需要使用直接前驱替换 */
    if (pAvlNode->left == NULL) {
        pNewChild = pAvlNode->right;
    } else if (pAvlNode->right == NULL) {
        pNewChild = pAvlNode->left;
    } else {
        /* 存在两个子女，将pAvlNode替换为直接前驱，并在原直接前驱处链接 */
        AvlNodeTransplant(pAvlNode, &pParent, &childType, &pNewChild);
    }

    /* 重新链接 */
    if (pNewChild != NULL) {
        pNewChild->parent = pParent;
    }
    if (childType == AVL_LEFT) {
        pParent->left = pNewChild;
    } else {
        pParent->right = pNewChild;
    }

    /* 释放结点 */
    AvlNodeFree(pAvlNode);

    /* 删除节点后再平衡 */
    AvlTreeRemoveBalance(pParent, &pAvlTree->dumRoot);

    return AVL_OK;
}

/* 查找节点 */
int AvlTreeGetData(AVL_TREE_S *pAvlTree, void *key, void *dataBuff, uint32_t buffSize)
{
    AVL_NODE_S *pAvlNode = AvlNodeFind(pAvlTree, key);
    if (pAvlNode == NULL) {
        return AVL_ERR;
    }

    if (buffSize <= AVL_BYTES_COPY_LEN) {
        AvlBytesCopy(dataBuff, pAvlNode->data, buffSize);
    } else {
        (void)memcpy(dataBuff, pAvlNode->data, buffSize);
    }

    return AVL_OK;
}

/* 销毁avl树 */
void AvlTreeDestroy(AVL_TREE_S *pAvlTree)
{
    return;
}

/* 获取avl树高度 */
uint32_t AvlTreeGetHeight(AVL_TREE_S *pAvlTree)
{
    if ((pAvlTree == NULL) || (pAvlTree->dumRoot.left == NULL)) {
        return 0;
    }

    return (uint32_t)pAvlTree->dumRoot.left->height;
}

