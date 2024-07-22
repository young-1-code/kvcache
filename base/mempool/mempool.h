/**
 * Copyright (c) 2024 young. All rights reserved. 
 * @author young, email:ovsuun63x11@gmail.com 
 */

#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

#include <stdio.h>
#include <stdint.h>

#define ITEM_MEM_NUM 128 /* 128种规格 8 16 24 ...*/
#define LIST_MEM_NUM 16  /* 默认一条链表有16个节点 */
#define ALIGN_SIZE    8
#define ALIGN(bytes) ((bytes+ALIGN_SIZE-1) & ~(ALIGN_SIZE-1))
#define LIST_INDEX(bytes) ((bytes+ALIGN_SIZE-1)/ALIGN_SIZE-1)

typedef struct chunklist_t
{
    struct chunklist_t *next;
}chunklist_t;

union freemem_t
{
    union freemem_t *link;
    uint8_t p[1];
};

typedef struct 
{
    chunklist_t *head;  /* 链表存放从系统申请的大块内存 */
    union freemem_t* volatile mem[ITEM_MEM_NUM]; /* 空闲链表 */
    void* (*alloc)(void* pthis, size_t size); /* 从内存池申请内存 */
    void (*free)(void* pthis, void* p); /* 释放内存 */
    void (*try_compress)(void *pthis);  /* 尝试向系统释放内存 */
}mempool_t;

/**
 * @brief 创建并初始化内存池
 * @return 返回内存池句柄
*/
mempool_t* mempool_create(void);

/**
 * @brief 销毁内存池
 * @param pool 传入内存池句柄
*/
void mempool_destroy(mempool_t *pool);


void* kvcache_molloc(size_t size);
void kvcache_free(void *m);

#endif // !__MEMPOOL_H__
