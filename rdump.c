/*****************************************************************************
	rdump.c	- Mainline routines

	Copyright (C) 1996, 1997, 2014, 2018, by Chet Simpson.

	This file is distributed under the MIT License. See notice at the end
	of this file.

*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <io.h>
#include "rof.h"
#include "disasm.h"

#define MAXSOURCE	20
static const char *snames[MAXSOURCE];
static const char *fname = NULL;
static int16 scount = 0;
static BOOL gflag = FALSE;
static BOOL rflag = FALSE;
static BOOL oflag = FALSE;
static BOOL iflag = FALSE;
static FILE *in = NULL;



/*
	header
	globals
	object code
	dp data
	data
	external references
	local references
*/
void DisassembleROF()
{
	int16 count;

	if(scount == 0)
	{
		return;
	}

	for(count = 0; count < scount; count++)
	{
		int rofCount;

		fname = snames[count];

		in = fopen(fname,"rb");

		if(in == NULL)
		{
			fprintf(stderr, "can't open '%s'",fname);
			return;
		}

		rofCount = 0;
		while(TRUE)
		{
			OS9ROF *rfile;
			int result;
			FILE *outFile;


			result = LoadROF(in, &rfile, fname);
			if(0 != result)
			{
				if(0 == rofCount)
				{
					error("'%s' does not contain any relocatable object files", fname);
				}

				break;
			}
			fprintf(stderr, "Disassembling %s\n", rfile->name);

			/* Go trace the code */
			TraceObjectCode(rfile);

			if(TRUE == iflag)
			{
				char fname[256];
				strcpy(fname, rfile->name);
				strcat(fname, ".asm");
				outFile = fopen(fname, "w");
			}
			else
			{
				outFile = stdout;
			}
			if(TRUE == rflag)
			{
				DumpROFInfo(outFile, rfile);
			}
			DisasmObjectCode(outFile, rfile);
			FreeROF(rfile);

			if(outFile != stdout)
			{
				fclose(outFile);
			}

			rofCount++;
		}
		fclose(in);
	}
}


void help(void)
{

	fprintf(stderr, "kdisasm:  prints formatted dump of .r and .l files\n");
	fprintf(stderr, "usage: kdisasm [opts] <file>[ <file>] [opts]\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "-g - add global definition info\n");
	fprintf(stderr, "-r - dump ROF information\n");
	fprintf(stderr, "-o - add reference and local offset info\n");
	fprintf(stderr, "-a - all of the above\n");
}

/* symbol table types */
/* symbol definition/reference type/location */
int main(int argc, char **argv)
{
	int i;
	
	for(i = 1;  i < argc; i++)
	{
		const char *p;

		p = argv[i];

		if(0 == stricmp(p, "-help"))
		{
			help();
			exit(0);
		}

		if('-' == *p)
		{
			p++;

			while(*p)
			{
				switch(*p)
				{
				case 'i': iflag = TRUE; break;
				case 'g': gflag = TRUE; break;
				case 'r': rflag = TRUE; break;
				case 'o': oflag = TRUE; break;
				default:
					error("unknown option -%c",*p);
				}
				p++;

			}
		}
		else
		{
			if(scount==MAXSOURCE)
			{
				error("too many source files");
			}
			snames[scount++]= p;
		}
	}

	DisassembleROF();
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
