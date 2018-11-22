/*****************************************************************************
	rof.h	- Definitions for .r (ROF) file handling

	Copyright (C) 1996, 1997, 2014, 2018, by Chet Simpson.

	This file is distributed under the MIT License. See notice at the end
	of this file.

*****************************************************************************/
#ifndef ROF_H
#define ROF_H

#include <stdio.h>
#include "util.h"

#define F_RELATIVE	0x80			/* adjustment - relative reference */
#define F_NEGATE	0x40			/* adjustment - negate on resolution */
#define CODLOC		0x20			/* location flag - 0 = data | 1 = code */
#define DIRLOC		0x10			/* location - global/direct flag */
#define F_BYTE		0x08			/* location - two/one byte size flag */
#define CODENT		0x04			/* type - 0 - to data | 1 - to code */
#define DIRENT		0x02			/* data - global/direct flag */
#define INIENT		0x01			/* data - clear/init. data flag */

#define MASK_GLOBAL		(F_RELATIVE | F_NEGATE | CODENT | DIRENT | INIENT)
#define MASK_EXTERNAL	(F_RELATIVE | CODLOC)
#define MASK_LOCAL		(CODLOC | F_BYTE | CODENT | DIRENT | INIENT)

/* misc. constants */
#define ROFSYNC     0x62CD2387
#define SYMLEN      64		/* Length of symbols */
#define MAXNAME     16		/* length of module name */


typedef enum
{
	REF_GLOBAL,
	REF_LOCAL,
	REF_EXTERNAL
} REFTYPE;


typedef struct
{
	REFTYPE		type;
	char		*symbol;	/* Symbol if present */
	u_char		flag;		/* type/location flag */
	u_int16		offset;		/* Offset into the object code */
} Reference;



typedef struct
{
	char			name[256];	/* Name of the OS9ROF */
	char			*filename;	/* Name of the file containing the ROF */

	/* Offset information */
	fpos_t			hdrSize;			/* Size of the header */
	fpos_t			offsetBase;			/* Position the rof starts at */
	fpos_t			offsetCode;			/* Position the code starts at */
	fpos_t			offsetDPData;		/* Position the Direct Page data starts at */
	fpos_t			offsetData;			/* Position the ?initialized? data starts at */
	fpos_t			offsetGlobRefs;		/* Offset to global references */
	fpos_t			offsetExtRefs;		/* Offset to external references */
	fpos_t			offsetLocRefs;		/* Offset to local references */

	/* Object code */
	u_int16			sizeObjectCode;		/* h_ocode - size of the code block */
	u_char			*objectCode;		/* object code */

	/* Initialized data */
	u_int16			sizeUninitData;		/* h_glbl */
	u_int16			sizeInitData;		/* h_data */
	u_char			*initData;			/* initialized data */

	/* Initialized direct page data */
	u_int16			sizeUninitDPData;	/* h_dglbl - Size of uninitialized direct page data */
	u_int16			sizeInitDPData;		/* h_ddata - size of initialized direct page data */
	u_char			*initDataDP;		/* initialized direct page data */

	/* Misc */
	u_int16			typeLanguage;		/* Type language */
	u_char			asmVaild;			/* Assembler valid? */
	u_char			creationDate[5];	/* Date of creation */
	u_char			edition;			/* Edition */
	u_int16			sizeStack;			/* Size of stack space */
	u_int16			execEntry;			/* Execution entry point */

	List			*refList;
} OS9ROF;



void error(const char *fmt, ...);
void ferr(const char *s);
void read8(FILE *file, u_char *val);
void read16(FILE *file, u_int16 *retVal);
void read32(FILE *file, u_int32 *retVal);

int LoadROF(FILE *in, OS9ROF **retROF, const char *filename);
void FreeROF(OS9ROF *rfile);
void DumpROFInfo(FILE *outFile, OS9ROF *rof);
Reference *GetReference(OS9ROF *rfile, REFTYPE type, u_int16 location, BOOL code, BOOL init);


#endif	/* ROF_H */



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
