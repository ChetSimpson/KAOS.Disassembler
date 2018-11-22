/*****************************************************************************
	genasm.c	- Portion of the disassembler that generated assembler output

	Copyright (C) 1996, 1997, 2014, 2018, by Chet Simpson.

	This file is distributed under the MIT License. See notice at the end
	of this file.

*****************************************************************************/
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "genasm.h"
#include "rof.h"



static BOOL outputAsm = FALSE;


/*************************************************************************** 
***************************************************************************/
void SetAsmOutputMode(BOOL mode)
{
	outputAsm = mode;
}

void GenAsmTabs(FILE *outFile, int lsize)
{
	while(lsize < MAX_TABS)
	{
		GenAsm(outFile, "\t");
		lsize += TAB_SIZE;
	}
}

/*************************************************************************** 
***************************************************************************/
/*
	Searchs all reference data for locations which are
	accessed and have labels.
*/
char *GetReferenceLabel(OS9ROF *rfile, u_int16 label)
{
	Reference *ref;

	if(NULL == rfile)
	{
		return NULL;
	}

	/* Check for a global symbol */
	ref = GetReference(rfile, REF_GLOBAL, label, TRUE, FALSE);
	if(NULL != ref)
	{
		return ref->symbol;
	}

	/* Check for a reference */
	ref = GetReference(rfile, REF_EXTERNAL, label, TRUE, FALSE);
	if(NULL != ref)
	{
		return ref->symbol;
	}

	return NULL;
}


/*************************************************************************** 

***************************************************************************/
int GenAsm(FILE *outFile, const char *fmt, ...)
{
	if(TRUE == outputAsm)
	{
		va_list list;
		int size;

		va_start(list, fmt);
		size = vfprintf(outFile, fmt, list);
		va_end(list);

		return size;
	}
	return 0;
}


/*************************************************************************** 
***************************************************************************/
int GenAsmOp(FILE *outFile, const char *op, const char *fmt, ...)
{
	int length = 0;

	if(TRUE == outputAsm)
	{
		if(NULL == op)
		{
			op = "????";
		}

		length += GenAsm(outFile, "%s", op);

		GenAsmTabs(outFile, length);

		if(NULL != fmt)
		{
			va_list list;

			va_start(list, fmt);
			length += vfprintf(outFile, fmt, list);
			va_end(list);
		}
	}

	return length;
}


/*************************************************************************** 
***************************************************************************/
int GenAsmLabelCode(FILE *outFile, u_int16 label)
{
	return GenAsm(outFile, "L%04X", label);
}

int GenAsmLabelCodeData(FILE *outFile, u_int16 label)
{
	return GenAsm(outFile, "D%04X", label);
}

/*************************************************************************** 
***************************************************************************/
int GenAsmLabelData(FILE *outFile, u_int16 label)
{
	return GenAsm(outFile, "I%04X", label);
}


/*************************************************************************** 
***************************************************************************/
int GenAsmLabelBSS(FILE *outFile, u_int16 label)
{
	return GenAsm(outFile, "U%04X", label);
}


/*************************************************************************** 
***************************************************************************/
void GenAsmAddress(FILE *outFile, u_int16 address)
{
	GenAsm(outFile, "$%04", address);
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
