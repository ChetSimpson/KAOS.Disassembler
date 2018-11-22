/*****************************************************************************
	genasm.h	- Function prototypes for assembler generation calls

	Copyright (C) 1996, 1997, 2014, 2018, by Chet Simpson.

	This file is distributed under the MIT License. See notice at the end
	of this file.

*****************************************************************************/
#ifndef GENASM_H
#define GENASM_H

#include "util.h"
#include "rof.h"

#define MAX_TABS	12
#define TAB_SIZE	4

void SetAsmOutputMode(BOOL mode);

char *GetReferenceLabel(OS9ROF *rfile, u_int16 label);

int GenAsm(FILE *outFile, const char *fmt, ...);
int GenAsmOp(FILE *outFile, const char *op, const char *fmt, ...);

typedef int (*GENLABEL)(FILE *outFile, u_int16 location);

void GenAsmTabs(FILE *outFile, int lsize);
void GenAsmAddress(FILE *outFile, u_int16 address);
int GenAsmLabelCode(FILE *outFile, u_int16 label);
int GenAsmLabelCodeData(FILE *outFile, u_int16 label);
int GenAsmLabelData(FILE *outFile, u_int16 label);
int GenAsmLabelBSS(FILE *outFile, u_int16 label);


#endif	/* GENASM_H */



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
