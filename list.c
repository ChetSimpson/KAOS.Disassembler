/*****************************************************************************
	list.c	- A simple linked list

	Copyright (C) 1996, 1997, 2014, 2018, by Chet Simpson.

	This file is distributed under the MIT License. See notice at the end
	of this file.

*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "util.h"

struct _Node
{
	struct _Node	*next;
	struct _Node	*prev;
	void			*data;
};

struct _List
{
	Node	*head;
	Node	*tail;
	u_int32	size;
};


void ListInit(List **listRet)
{
	List *list;

	list = malloc(sizeof(List));
	if(NULL != list)
	{
		list->head = NULL;
		list->tail = NULL;
		list->size = 0;
	}

	*listRet = list;
}

void *ListAddTail(List *list, void *data)
{
	Node *node;

	node = malloc(sizeof(Node));
	if(NULL != node)
	{
		if(NULL == list->head)
		{
			list->head = node;
			node->prev = NULL;
		}
		else
		{
			node->prev = list->tail;
			list->tail->next = node;
		}

		node->next = NULL;
		node->data = data;
		list->tail = node;
		list->size++;
	}

	return node;
}

u_int32 ListGetSize(List *list)
{
	return list->size;
}

Node *ListGetHead(List *list)
{
	return list->head;
}


void *NodeGetData(Node *node)
{
	return node->data;
}


Node *NodeGetNext(List *list, Node *node)
{
	if(NULL == node)
	{
		node = list->head;
	}
	else
	{
		node = node->next;
	}

	return node;
}



/*****************************************************************************
	
	Copyright (C) 1996, 1997, 2014, 2018, by Chet Simpson
	
	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:
	
	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
	
*****************************************************************************/
