/*****************************************************************************
	disasm.h	- CPU definitions for disassembler/tracer

	Copyright (C) 1996, 1997, 2014, 2018, by Chet Simpson.

	This file is distributed under the MIT License. See notice at the end
	of this file.

*****************************************************************************/
#ifndef DISASM_H
#define DISASM_H

#include "rof.h"

#define MAX_MEMORY			65536

#define	POS_STRING			""
#define	NEG_STRING			"-"

#define SREG_PC				0x80
#define SREG_U_S			0x40
#define SREG_Y				0x20
#define SREG_X				0x10
#define SREG_DP				0x08
#define SREG_B				0x04
#define SREG_A				0x02
#define SREG_CC				0x01

#define IDX_INCREG			0x00
#define IDX_INCREG2			0x01
#define IDX_DECREG			0x02
#define IDX_DECREG2			0x03
#define IDX_OFFSET_0		0x04
#define IDX_OFFSET_B		0x05
#define IDX_OFFSET_A		0x06
#define IDX_ILLEGAL1		0x07
#define IDX_OFFSET_BYTE		0x08
#define IDX_OFFSET_WORD		0x09
#define IDX_ILLEGAL2		0x0a
#define IDX_OFFSET_D		0x0b
#define IDX_OFFSET_PCR1		0x0c
#define IDX_OFFSET_PCR2		0x0d
#define IDX_ILLEGAL3		0x0e
#define IDX_INDIRECT		0x0f

#define PB_INDIRECT			0x10	/* Indirect postop */

#define MAX_FCCLENGTH		32		/* Max length of an FCC string */
#define MAX_FCBLENGTH		8		/* Max number of bytes to include on a single FCB line */
#define MAX_OS9CALLS		0x91	/* Maximum number of OS-9 calls in the call table */


typedef int (*DISASM)(FILE *outFile, u_char *mem, struct _Opcode *op, u_char code, u_int16 pc);
typedef int (*TRACE)(u_char *mem, struct _Opcode *op, u_char code, u_int16 pc);

typedef struct _Opcode
{
	char		*opName;	/* Name of the operation */
	int			byteCount;	/* Number of bytes for the operation */
	DISASM		disasmFunc;	/* Call for disassembler operation */
	TRACE		traceFunc;	/* Call for trace operation */
	u_char		opValue;	/* Test value (unused) */
} Opcode;

typedef struct
{
	char	*callString;	/* Actual OS-9 call */
	char	*callDesc;		/* Description of the OS-9 call */
} OS9Call;


extern Opcode optable[];
extern Opcode optable10[];
extern Opcode optable11[];
extern OS9Call os9calls[MAX_OS9CALLS];

/* Disassbmler callback functions */
int	DisasmIllegal(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmDirect(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmPage10(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmPage11(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmImmediate(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmImmediateLong(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmInherent(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmIndexed(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmExtended(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmRelative(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmRelativeLong(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmRegToRegOp(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmSystemStackOp(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmUserStackOp(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmPage10(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmPage11(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	DisasmOS9SysCall(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc);

/* Code trace callback functions */
int	TraceReturn(u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	TraceGeneric(u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	TracePage10(u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	TracePage11(u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	TraceIndexed(u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	TraceRelative(u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	TraceRelativeLong(u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	TracePage10(u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	TracePage11(u_char *mem, Opcode *op, u_char code, u_int16 pc);
int	TracePullStack(u_char *mem, Opcode *op, u_char code, u_int16 pc);
int TraceRelativeJump(u_char *mem, Opcode *op, u_char code, u_int16 pc);
int TraceRelativeJumpLong(u_char *mem, Opcode *op, u_char code, u_int16 pc);

/* Trace and disassembler interface calls */
void TraceObjectCode(OS9ROF *rfile);
void DisasmObjectCode(FILE *outFile, OS9ROF *rfile);

#endif	/* DISASM_H */



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
