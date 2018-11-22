/*****************************************************************************
	roflib.c	- Utilitie functions for reading OS-9 .r (ROF) files

	Copyright (C) 1996, 1997, 2014, 2018, by Chet Simpson.

	This file is distributed under the MIT License. See notice at the end
	of this file.

*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <assert.h>
#include "rof.h"

char *fname;

static void GetOffset(FILE *in, OS9ROF *rfile, fpos_t *retPos)
{
	fpos_t pos;
	fgetpos(in, &pos);

	*retPos = pos - rfile->offsetBase;
}

/*************************************************************************** 

***************************************************************************/
static void hexadump(FILE *outfile, u_char *s, int length)
{
	int x = 0;
	int loc = 0;

	while(length--)
	{
		if(x == 0)
		{
			fprintf(outfile, "%04X:  ", loc);
		}
		fprintf(outfile, "%02X ", *(s++));

		x++;

		if(x == 16)
		{
			x = 0;
			fprintf(outfile, "\n");
		}
	}

	fprintf(outfile, "\n");

}


/*************************************************************************** 
***************************************************************************/
static int ReadString(FILE *in, char *s)
{
	int count = 0;

	while(TRUE)
	{
		*s = getc(in);
		if(0 == *s)
		{
			break;
		}
		s++;
		count++;

		if(SYMLEN == count)
		{
			error("Long symbol name encountered");
			break;
		}
	}

	if(ferror(in))
	{
		error("Error reading string from file!\n");
	}

	return (count);
}

Reference *GetReference(OS9ROF *rfile, REFTYPE type, u_int16 location, BOOL code, BOOL init)
{
	Node *node;

	node = ListGetHead(rfile->refList);

	while(NULL != node)
	{
		Reference *ref;

		ref = NodeGetData(node);

		if(type == ref->type && location == ref->offset)
		{

			// FIXME - not sure if this is the correct method to do this
			if(TRUE == code && 0 != (ref->flag & (CODLOC | CODENT)))
			{
				return ref;
			}


			if(FALSE == code && 0 == (ref->flag & (CODLOC | CODENT)))
			{

				if(TRUE == init && 0 == (ref->flag & INIENT))
				{
					return ref;
				}

				if(FALSE == init && 0 != (ref->flag & INIENT))
				{
					return ref;
				}
			}
		}

		node = NodeGetNext(rfile->refList, node);
	}

	return NULL;
}

/*************************************************************************** 
	Globals references (exported symbols)

	word		reference count (rcount)

	---References---
	???			Null terminated string
	byte		flag
	word		offset
***************************************************************************/
Reference *AddReference(OS9ROF *rfile, REFTYPE type, char *sym, u_char flag, u_int16 offset)
{
	Reference *ref;
	u_char mask;

	switch(type)
	{
	case REF_GLOBAL:
		mask = MASK_GLOBAL;
		break;
	case REF_EXTERNAL:
		mask = MASK_EXTERNAL;
		break;
	case REF_LOCAL:
		mask = MASK_LOCAL;
		break;
	default:
		assert(0);
	}

	if(0 != (flag & ~mask))
	{
		fprintf(stderr, "Reference invalid!\n");
	}

	ref = malloc(sizeof(Reference));
	if(NULL != ref)
	{
		ref->type = type;
		ref->symbol = sym;
		ref->flag = flag;
		ref->offset = offset;
		ListAddTail(rfile->refList, ref);
	}

	return ref;
}


static int LoadGlobals(FILE *in, OS9ROF *rfile)
{
	int i;
	char symbuf[SYMLEN+1];
	char *symbol;
	u_int16	count;
	u_char flag;
	u_int16 offset;

	/* Get the number of globals */
	read16(in, &count);

	if(0 != count)
	{
		/* Allocate the globals */
		for(i = 0; i < count; i++)
		{
			int length;
			/* Read the global entry */
			length = ReadString(in, symbuf);
			read8(in, &flag);
			read16(in, &offset);

			/* Copy the symbol name over */
			symbol = malloc(length + 1);
			strcpy(symbol, symbuf);

			AddReference(rfile, REF_GLOBAL, symbol, flag, offset);
		}
	}

	return 0;
}


/*************************************************************************** 
	Local references

	word		reference count (rcount)

	----reference data = rcount * the following---
	byte		flag
	word		offset
***************************************************************************/
static int LoadLocalRefs(FILE *in, OS9ROF *rfile)
{
	u_int16 count;
	u_char flag;
	u_int16 offset;

	GetOffset(in, rfile, &rfile->offsetLocRefs);

	read16(in, &count);

	if(0 != count)
	{
		int i;
		for(i = 0; i < count; i++)
		{
			read8(in, &flag);
			read16(in, &offset);

			AddReference(rfile, REF_LOCAL, NULL, flag, offset);
		}
	}

	return 0;
}



/*************************************************************************** 
	external references

	word		reference count

	---References---
	word		symbol references (rcount)
	???			Null terminated string
	----reference data = rcount * the following---
	byte		flag
	word		offset
***************************************************************************/
static int LoadExtRefs(FILE *in, OS9ROF *rfile)
{
	char sym[SYMLEN+1];
	char *symbol;
	u_int16 count;
	u_int16 offset;
	u_char flag;

	GetOffset(in, rfile, &rfile->offsetExtRefs);

	/* Get the total number of external references */
	read16(in, &count);

	/* If we have references load them */
	if(0 != count)
	{
		int i;

		/* Load the references */
		for(i = 0; i < count; i++)
		{
			int length;
			u_int16 subcount;

			/* Read the string */
			length = ReadString(in, sym);
			symbol = malloc(length + 1);
			strcpy(symbol, sym);

			/* Read the count */
			read16(in, &subcount);

			if(subcount != 0)
			{
				int x;
				for(x = 0; x < subcount; x++)
				{
					read8(in, &flag);
					read16(in, &offset);
					AddReference(rfile, REF_EXTERNAL, symbol, flag, offset);
				}
			}
		}
	}

	return 0;
}


/*************************************************************************** 
	Load object code
***************************************************************************/
int LoadCode(FILE *in, OS9ROF *rfile)
{
	GetOffset(in, rfile, &rfile->offsetCode);

	if(0 != rfile->sizeObjectCode)
	{
		size_t result;
		rfile->objectCode = malloc(rfile->sizeObjectCode);

		result = fread(rfile->objectCode, 1, rfile->sizeObjectCode, in);
		if(result != rfile->sizeObjectCode)
		{
			return -1;
		}
	}

	return 0;
}


/*************************************************************************** 
	Load the OS9ROF header

	dword		Signature
	word		Type and language
	byte		Assembler valid?
	byte		month of creation
	byte		day of creation
	byte		year of creation (base of 1900)
	byte		hour of creation
	byte		minute of creation
	byte		Edition
	byte		Unused
	word		Size of uninitialized data
	word		Size of uninitialized DP data
	word		Size of initialized data
	word		Size of initialized DP data
	word		Size of object code
	word		Size of stack space
	word		Execution entry point
***************************************************************************/
static int ReadROFHeader(FILE *in, OS9ROF *rfile)
{
	int length;
	u_int32	sync;

	/* Get the location in the file */
	fgetpos(in, &rfile->offsetBase);

	/* Check for end of file */
	length = filelength(fileno(in));
	if(rfile->offsetBase >= length)
	{
		return -1;
	}

	/* Read and check the sync bytes */
	read32(in, &sync);
	if(sync != ROFSYNC)
	{
		return -1;
	}

	/* Read the rest of the header */
	read16(in, &rfile->typeLanguage);
	read8(in, &rfile->asmVaild);
	fread(rfile->creationDate, 5, 1, in);
	read8(in, &rfile->edition);
	fgetc(in);	/* Skip the unused byte */
	read16(in, &rfile->sizeUninitData);
	read16(in, &rfile->sizeUninitDPData);
	read16(in, &rfile->sizeInitData);
	read16(in, &rfile->sizeInitDPData);
	read16(in, &rfile->sizeObjectCode);
	read16(in, &rfile->sizeStack);
	read16(in, &rfile->execEntry);
	ReadString(in, rfile->name);

	return 0;
}



/*************************************************************************** 
	Free an allocated OS9ROF structure
***************************************************************************/
void FreeROF(OS9ROF *rfile)
{
	if(NULL == rfile->objectCode)
	{
		free(rfile->objectCode);
		rfile->objectCode = NULL;
	}
}


/*************************************************************************** 
	Load a reolcatable object section from an opened file
***************************************************************************/
int LoadROF(FILE *in, OS9ROF **retROF, const char *filename)
{
	int result;
	OS9ROF *rfile;

	*retROF = NULL;

	/* Allocate the struct */
	rfile = calloc(1, sizeof(OS9ROF));

	ListInit(&rfile->refList);

	/* Save the filename */
	rfile->filename = malloc(strlen(filename) + 1);
	strcpy(rfile->filename, filename);

	/* Read the header */
	result = ReadROFHeader(in, rfile);
	if(0 == result)
	{
		result = LoadGlobals(in, rfile);
	}

	if(0 == result)
	{
		result = LoadCode(in, rfile);
	}

	if(0 == result && 0 != rfile->sizeInitDPData)
	{
		GetOffset(in, rfile, &rfile->offsetDPData);

		/* Load direct page data */
		rfile->initDataDP = malloc(rfile->sizeInitDPData);
		fread(rfile->initDataDP, rfile->sizeInitDPData, 1, in);
	}

	if(0 == result && 0 != rfile->sizeInitData)
	{
		GetOffset(in, rfile, &rfile->offsetData);

		rfile->initData = malloc(rfile->sizeInitData);
		fread(rfile->initData, rfile->sizeInitData, 1, in);
	}


	if(0 == result)
	{
		/* Load the references */
		result = LoadExtRefs(in, rfile);
	}

	if(0 == result)
	{
		result = LoadLocalRefs(in, rfile);
	}

	GetOffset(in, rfile, &rfile->hdrSize);

	if(0 != result)
	{
		FreeROF(rfile);
	}
	else
	{
		*retROF = rfile;
	}

	return result;
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
