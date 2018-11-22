/*****************************************************************************
	util.h	- Common definitions

	Copyright (C) 1996, 1997, 2014, 2018, by Chet Simpson.

	This file is distributed under the MIT License. See notice at the end
	of this file.

*****************************************************************************/
#ifndef UTIL_H
#define  UTIL_H

typedef enum
{
	FALSE,
	TRUE
} BOOL;

typedef unsigned char u_char;
typedef unsigned long u_int32;
typedef unsigned short u_int16;
typedef short int16;


typedef struct _Node Node;
typedef struct _List List;

void ListInit(List **list);
void *ListAddTail(List *list, void *data);
u_int32 ListGetSize(List *list);
Node *ListGetHead(List *list);

void *NodeGetData(Node *node);
Node *NodeGetNext(List *list, Node *node);

#endif	/* UTIL_H */



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
