#pragma once

#include "stdlib.h"
#include "stdint.h"

#define SUCCESS 1
#define FAIL    0

#define MIN_ORDER 4
#define MAX_ORDER 34
#define ORDERTOSIZE(i) (1 << (i)) /*������, �������� 2 � ������� i */

struct buddy_allocator_t_
{
	size_t memSize; /* ������ ����� ����������*/
	uint8_t maxOrder; /* ������������ ������� */
	void * memStart; /* ����� ���� */
	void * memPool[MAX_ORDER + 1]; /* ������ ������ ��������� ������ �� ��������. �� ����� ��� ��� */
};

typedef struct buddy_allocator_t_ buddy_allocator_t;

struct node_t /* ���� ��������, ������� ����� ������������ */
{
	buddy_allocator_t * buddy_alloc;
	struct node_t * next;
};

typedef struct node_t node;

node  * buddyList; /* ������� ���� ������ */

				   /**
				   * ������� ���������
				   * @param memory_size 
				   * @return ����� ���������
				   */
buddy_allocator_t *constructor(size_t raw_memory_size);

/**
* ���������� ���������
* @param buddy_allocator
*/
void buddy_allocator_destroy(buddy_allocator_t *buddy_allocator);

/**
* ���������� 
* @param buddy_allocator 
* @param size ������ ������ ��� ���������
* @param order �������(����� ������ ��� ��������� ������������ �����)
* @return ��������� �� �������������� ����, ��� @a NULL 
*/
void *alloc(buddy_allocator_t *buddy_allocator, size_t size, uint8_t * order);

/**
* ����������� ���� .
* ���� @a ptr  @a NULL, ������ �� ������
* @param buddy_allocator 
* @param ptr ������ ��� �����������
* @param order �������
*/
void buddy_allocator_free(buddy_allocator_t *buddy_allocator, void *ptr, uint8_t order);

int initBuddyAlloc(buddy_allocator_t *buddy_allocator);
void * alloc_buddy(buddy_allocator_t *buddy_allocator, size_t size, uint8_t * order);
void * findBuddy(buddy_allocator_t * buddyMem, void * block, uint8_t order);
