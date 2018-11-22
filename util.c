/*****************************************************************************
	util.c	- Stuff that doesn't fit anywhere else

	Copyright (C) 1996, 1997, 2014, 2018, by Chet Simpson.

	This file is distributed under the MIT License. See notice at the end
	of this file.

*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "rof.h"



/*************************************************************************** 

***************************************************************************/
void error(const char *fmt, ...)
{
	va_list start;
	
	va_start(start, fmt);
	
	fprintf(stderr, "kdisasm: ");
	vfprintf(stderr, fmt, start);
	putc('\n',stderr);
	exit(1);
}


void ferr(const char *s)
{
	error("error reading '%s'",s);
}


/*************************************************************************** 

***************************************************************************/
void read8(FILE *file, u_char *val)
{
	*val = fgetc(file);
}


void read16(FILE *file, u_int16 *retVal)
{
	u_int16 val;

	val = (fgetc(file) << 8);
	val |= fgetc(file);

	*retVal = val;
}


void read32(FILE *file, u_int32 *retVal)
{
	u_int32 val;

	val = (fgetc(file) << 24);
	val |= (fgetc(file) << 16);
	val |= (fgetc(file) << 8);
	val |= fgetc(file);

	*retVal = val;
}




/*************************************************************************** 

***************************************************************************/
static void ftext(FILE *outFile, u_char flag, int16 ref, REFTYPE type)
{
	fprintf(outFile, "[%02x : %04X] ", flag, ref);

	if(REF_GLOBAL != type)
	/*if(flag & LOCMASK)*/
	{
		fprintf(outFile, flag & F_BYTE ? "byte" : "word");
		fprintf(outFile, " ");

		if(flag & CODLOC)
		{
			fprintf(outFile, "in code");
		}
		else
		{
			fprintf(outFile, "in ");
			if(flag & DIRLOC)
			{
				fprintf(outFile, "direct page");
			}
			fprintf(outFile, "data");
		}

		fprintf(outFile, " ");

		if(flag & (F_NEGATE | F_RELATIVE))
		{
			fprintf(outFile, "(");
			
			switch(flag & (F_NEGATE  | F_RELATIVE))
			{
			case F_NEGATE:
				fprintf(outFile, "negative");
				break;
			case F_RELATIVE:
				fprintf(outFile, "offset pcr");
				break;
			case F_NEGATE  | F_RELATIVE:
				fprintf(outFile, "negate offset to pcr");
				break;
			}
			fprintf(outFile, ") ");
		}
	}

	if(flag & CODENT)
	{
		fprintf(outFile, "references code");
	}

	//CODENT	2	The reference refers to code
	//DIRENT	1	The reference refers to direct-page data
	//INIENT	0	The reference refers to initialized data

	if(REF_LOCAL == type)
	{
		fprintf(outFile, "referencing ");

		if(flag & DIRENT)
		{
			fprintf(outFile, "direct page ");
		}

		if(flag & CODENT)
		{
			fprintf(outFile, "code ");
		}
		else
		{
			if(flag & INIENT)
			{
				fprintf(outFile, "data ");
			}
			else
			{
				fprintf(outFile, "bss ");
			}
		}
	}
/*
04	00000100	in non-dp data/word - to code
02	00000001	in non-dp data/word - to non-dp data
00	00000000	in non-dp data/word - to non-dp bss

	if(REF_LOCAL == type)
	{
		fprintf(outFile, "referencing ");

		if(flag & (DIRENT | INIENT))
		{
			fprintf(outFile, flag & INIENT ? "data" : "bss");

			if(flag & DIRENT)
			{
				fprintf(outFile, " in direct page");
			}
		}
		else if(flag & CODENT)
		{
			fprintf(outFile, "code");
		} else
		{
		}
	}
*/
}


static void DumpReferences(FILE *outFile, OS9ROF *rfile, REFTYPE type)
{
	void *node;
	int count;

	count = 0;
	node = ListGetHead(rfile->refList);
	while(NULL != node)
	{
		Reference *ref;

		ref = NodeGetData(node);

		if(type == ref->type)
		{
			count++;
		}
		node = NodeGetNext(rfile->refList, node);
	}

	if(0 == count)
	{
		return;
	}

	switch(type)
	{
	case REF_GLOBAL:
		fprintf(outFile, "* Global");
		break;
	case REF_EXTERNAL:
		fprintf(outFile, "* External");
		break;
	case REF_LOCAL:
		fprintf(outFile, "* Local");
		break;
	default:
		assert(0);
	}

	fprintf(outFile, " references:\n");

	node = ListGetHead(rfile->refList);
	while(NULL != node)
	{
		Reference *ref;

		ref = NodeGetData(node);

		if(type == ref->type)
		{
			fprintf(outFile, "*  ");
			if(NULL != ref->symbol)
			{
				fprintf(outFile, "%9s:", ref->symbol);
			}
			else
			{
				fprintf(outFile, "          ");
			}

			fprintf(outFile, "  ");
			ftext(outFile, ref->flag, ref->offset, type);
			fprintf(outFile, "\n");
		}

		node = NodeGetNext(rfile->refList, node);
	}
	fprintf(outFile, "*\n");
}

void DumpROFInfo(FILE *outFile, OS9ROF *rof)
{
	/* Print header information */
	fprintf(outFile, "************************************************************\n");
	fprintf(outFile, "* Module name: %s\t", rof->name);
	fprintf(outFile, "*   TyLa/RvAt: %02x/%02x\n", rof->typeLanguage >> 8, rof->typeLanguage);
	fprintf(outFile, "*   Asm valid: %s\n", rof->asmVaild ? "No" : "Yes");
	fprintf(outFile, "* Create date: %02d/%02d/%04d %02d:%02d\n", rof->creationDate[1],
													  rof->creationDate[2],
													  rof->creationDate[0] + 1900,
													  rof->creationDate[3],
													  rof->creationDate[4]);
	fprintf(outFile, "*     Edition: %2d\n",rof->edition);
	fprintf(outFile, "*     Section: Init Uninit\n");
	fprintf(outFile, "*        Code: %04x\n",rof->sizeObjectCode);
	fprintf(outFile, "*          DP: %04x %04x\n",rof->sizeInitDPData, rof->sizeUninitDPData);
	fprintf(outFile, "*        Data: %04x %04x\n",rof->sizeInitData, rof->sizeUninitData);
	fprintf(outFile, "*       Stack: %04x\n",rof->sizeStack);
	fprintf(outFile, "* Entry point: %04x\n",rof->execEntry);
	fprintf(outFile, "************************************************************\n");

	fprintf(outFile, "*\n");
	DumpReferences(outFile, rof, REF_GLOBAL);
	DumpReferences(outFile, rof, REF_EXTERNAL);
	DumpReferences(outFile, rof, REF_LOCAL);
	fprintf(outFile, "************************************************************\n");
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
