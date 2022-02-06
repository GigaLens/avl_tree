#ifndef AVL_TREE_H
#define AVL_TREE_H

#define AVL_OK 0  /* avl操作成功 */
#define AVL_ERR 1 /* avl操作失败 */

/* 节点比较函数 */
typedef int (*COMPARE_FUNC)(const void *key1, const void *key2);

/* avl树类型声明 */
typedef struct tagAvlTree AVL_TREE_S;

/* 创建avl树 */
AVL_TREE_S *AvlTreeCreate(COMPARE_FUNC cmpFunc);

/* 插入节点 */
int AvlTreeInsert(AVL_TREE_S *pAvlTree, void *key, unsigned int keySize, void *data,
    unsigned int dataSize);

/* 删除节点 */
int AvlTreeRemove(AVL_TREE_S *pAvlTree, void *key);

/* 查找节点 */
int AvlTreeGetData(AVL_TREE_S *pAvlTree, void *key, void *dataBuff, unsigned int buffSize);

/* 销毁avl树 */
void AvlTreeDestroy(AVL_TREE_S *pAvlTree);

/* 获取avl树的高度 */
unsigned int AvlTreeGetHeight(AVL_TREE_S *pAvlTree);

#endif
