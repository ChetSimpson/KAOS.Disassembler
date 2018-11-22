/*****************************************************************************
	disasm.c	- Main disassembler/trace functionality

	Copyright (C) 1996, 1997, 2014, 2018, by Chet Simpson.

	This file is distributed under the MIT License. See notice at the end
	of this file.

	--------------------------------------------------------------------------

	The disassembler was inspired by Didier Derny's original implementation.
	Although the implementation of that disassembler was discarded some
	concepts such as the table based call handlers remain.


	To Do:

		Add 6309 support
		Add support for RS-DOS .bin files
		Generate a text in a buffer instead of to a file for interactive mode
		Allow use of a memory description file
		Add $FFxx port definitions and support
		Change memory map to use bit table for possible use ON a CoCo
		Change disassembly process to read from file for possible use ON a CoCo

****************************************************************************/

#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "disasm.h"
#include "rof.h"
#include "genasm.h"

#define TRACE_RESET(x)				/* No longer used */
#define SET_TRACEINFO(offset, val)	traceInfo[offset] = val
#define SET_CODEINFO(offset, val)	codeInfo[offset] = val
#define SET_DATAINFO(offset, val)	dataInfo[offset] = val
#define SET_BSSINFO(offset, val)	bssInfo[offset] = val

#define GET_TRACEINFO(offset)		traceInfo[offset]
#define GET_CODEINFO(offset)		codeInfo[offset]
#define GET_DATAINFO(offset)		dataInfo[offset]
#define GET_BSSINFO(offset)			bssInfo[offset]

#define TRACE_DATA		0x00
#define TRACE_CODE		0x01


typedef enum
{
	LABCODE,		/* location is code, label is in code */
	LABDATA,		/* location is init data */
	LABBSS,			/* location is uninit data */
} LABTYPE;

typedef enum
{
	STATE_TRACE,		/* Trace state is trace */
	STATE_RETURN		/* Trace state is stop and return from pos */
} TSTATE;


typedef enum
{
	M_TEXT,			/* Data display mode is text (fcc) */
	M_BINARY		/* Data display mode is binary (fcb/fdb) */
} MODE;


static TSTATE	traceState = 0;		/* Current tracing state */
static u_int16	maxPC = 0;			/* Maximum code byte offset */
static int		disasmPass = 0;		/* Current disassembler pass */
static u_int16	xxPC = 0;			/* Current PC */
static OS9ROF	*rofFile = NULL;	/* Current ROF file */

/*
	The following tables could be better implemented as bit tables
	instead of taking up an entire byte. codeInfo, dataInfo, and bssInfo
	could probably be merged into a 2 bit value table reducing the
	size to 16k rather than a full 192k currently used. Of course this
	is only valid if you want to run the disassembler ON a coco although
	most .r files were no more than 1k in size so the extra work might
	not be worth it - just reduce the value of MAX_MEMORY.
*/
static u_char traceInfo[MAX_MEMORY];
static u_char codeInfo[MAX_MEMORY];
static u_char dataInfo[MAX_MEMORY];
static u_char bssInfo[MAX_MEMORY];


static int stackRegBits[8] =
{
	SREG_PC,
	SREG_U_S,
	SREG_Y,
	SREG_X,
	SREG_DP,
	SREG_B,
	SREG_A,
	SREG_CC
};

static const char *stackSRegTxt[8] =
{
	"pc", "u", "y", "x", "dp", "b", "a", "cc"
};

static const char *stackURegTxt[8] =
{
	"pc", "s", "y", "x", "dp", "b", "a", "cc"
};

static const char *Inter_Register[16] = 
{
	"d", "x", "y", "u", "s", "pc", "??", "??",
	"a", "b", "cc", "dp", "??", "??", "??", "??"
};

static const char *Indexed_Register[4] =
{
	"x", "y", "u", "s"
};

int postOpExtraBytes[32] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* 0x00 - 0x07 */
	0x01, 0x02, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00,	/* 0x08 - 0x0f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* 0x10 - 0x17 */
	0x01, 0x02, 0x00, 0x00, 0x01, 0x02, 0x00, 0x02	/* 0x18 - 0x1f */
};




/***********************************************************************
	Functionality for tracing opcodes to seperate code from data
***********************************************************************/
static u_int16 GetRelative(u_char *mem, u_int16 pc)
{
	int offset;
	int disp;
	
	offset = mem[pc + 1];
	if (offset < 127 )
	{
		disp   = pc + 2 + offset;
	}
	else
	{
		disp   = pc + 2 - (256 - offset);
	}

	return disp;
}

static int GetRelativeLong(u_char *mem, u_int16 pc)
{
	int offset;
	int disp;
	
	offset = mem[pc + 1] * 256 + mem[pc + 2];
	if (offset < 32767 )
	{
		disp   = pc + 3 + offset;
	}
	else
	{
		disp   = pc + 3 - (65536 - offset);
	}

	return disp;
}


/*************************************************************************** 
	Initiates a trace into a branch. Works until it gets a return
***************************************************************************/
static void TraceBranch(u_char *mem, u_int16 pc)
{
	if(pc >= maxPC)
	{
		return;
	}

	/*
		Prevent us from processing a block of code that has already
		been done
	*/
	if(TRACE_DATA == GET_TRACEINFO(pc))
	{
		int bytesUsed;

		/* Set the new pc */
		while(STATE_TRACE == traceState && pc < maxPC)
		{
			u_char code = mem[pc];

			bytesUsed = optable[code].traceFunc(mem, &optable[code], code, pc);
			if(bytesUsed < 1)
			{
				pc = -bytesUsed;
			}
			else
			{
				pc += bytesUsed;
			}
		}

		/* Reset the return state */
		traceState = STATE_TRACE;
	}
}


/*************************************************************************** 
***************************************************************************/
static void EnterTrace(u_char *mem, u_int16 pc)
{
	traceState = STATE_TRACE;

	TraceBranch(mem, pc);
}


/*************************************************************************** 
***************************************************************************/
int TraceGeneric(u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	int i;

	for(i = 0; i < op->byteCount; i++)
	{
		SET_TRACEINFO(pc + i, TRACE_CODE);
	}

	return op->byteCount;

}


/*************************************************************************** 
***************************************************************************/
int TraceGenericJmp(u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	return TraceGeneric(mem, op, code, pc);
}



/*************************************************************************** 
***************************************************************************/
int TracePage10(u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	SET_TRACEINFO(pc++, TRACE_CODE);
	code = mem[pc];
	return optable10[code].traceFunc(mem, &optable10[code], code, pc) + 1;
}


int TracePage11(u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	SET_TRACEINFO(pc++, TRACE_CODE);
	code = mem[pc];
	return optable11[code].traceFunc(mem, &optable11[code], code, pc) + 1;
}


int TraceIndexed(u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	int postbyte;
	int extrabytes;

	
	extrabytes = 0;
	postbyte = mem[pc + 1];

	if(0 != (postbyte & 0x80))
	{
		extrabytes = postOpExtraBytes[postbyte & 0x1f];
	}

	TraceGeneric(mem, op, code, pc);

	return op->byteCount + extrabytes;
}


int TraceRelative(u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	int used;

	used = TraceGeneric(mem, op, code, pc);

	TraceBranch(mem, GetRelative(mem, pc));

	return used;
}


int TraceRelativeLong(u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	int used;

	used = TraceGeneric(mem, op, code, pc);

	TraceBranch(mem, (u_int16)GetRelativeLong(mem, pc));

	return used;
}


int TraceRelativeJump(u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	u_int16 newPC;
	int bytesUsed;

	bytesUsed = TraceGeneric(mem, op, code, pc);

	newPC = GetRelative(mem, pc);

	/* If the new PC already has something, don't go there! */
	if(TRACE_DATA != GET_TRACEINFO(newPC))
	{
		return bytesUsed;
	}

	return -newPC;
}


int TraceRelativeJumpLong(u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	u_int16 newPC;
	int bytesUsed;

	bytesUsed = TraceGeneric(mem, op, code, pc);

	newPC = GetRelativeLong(mem, pc);

	/* If the new PC already has something, don't go there! */
	if(TRACE_DATA != GET_TRACEINFO(newPC))
	{
		return bytesUsed;
	}

	return -newPC;
}


int TraceReturn(u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	traceState = STATE_RETURN;

	return TraceGeneric(mem, op, code, pc);
}


int TracePullStack(u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	if(mem[pc + 1] & SREG_PC)
	{
		traceState = STATE_RETURN;
	}

	return TraceGeneric(mem, op, code, pc);
}



/*************************************************************************** 
***************************************************************************/
void TraceObjectCode(OS9ROF *rfile)
{
	Node *node;
	int traced;

	traced = 0;
	maxPC = rfile->sizeObjectCode;

	/* Reset the disassembler trace info */
	memset(traceInfo, TRACE_DATA, MAX_MEMORY);

	/* Get the head of the list */
	node = ListGetHead(rfile->refList);

	/* While there are global references, trace them */
	while(NULL != node)
	{
		Reference *ref;

		ref = NodeGetData(node);
		if(ref->type == REF_GLOBAL)
		{
			EnterTrace(rfile->objectCode, ref->offset);
			traced++;
		}

		node = NodeGetNext(rfile->refList, node);
	}

	/* FIXME - should we start from the execution point? */
	/* If no globals were traced, start from the beginning */
	if(0 == traced)
	{
		EnterTrace(rfile->objectCode, 0);
	}
}


static u_int16 getWord(u_char *mem, u_int16 index)
{
	return((mem[index] << 8) | mem[index + 1]);
}

/*

  Generating a base label

	for BSS:
		Pointed to by a global reference
		Pointed to by a local reference

	for DATA:
		Pointed to by a global reference
		Pointed to by a local reference

	for CODE:
		Pointer to by a global reference
		Pointed to by a local reference
*/
static Reference *FindDataRef(OS9ROF *rfile, u_int16 addr)
{
	Node *node;
	Reference *ref;

	/* Get the first node of the reference list */
	node = ListGetHead(rfile->refList);

	while(NULL != node)
	{
		/* Get the node data */
		ref = NodeGetData(node);

		/* if it's the in code, the right offset, and right type, check it */
		if(REF_LOCAL == ref->type && addr == ref->offset)
		{
			if(0 == (ref->flag & CODLOC))
			{
				/*
					Local references can target code, data, or bss so we need
					to check them all
				*/
				/*if(0 == (ref->flag & CODENT) && 0 != (ref->flag & INIENT))*/
				{
					return ref;
				}
			}
		}

		node = NodeGetNext(rfile->refList, node);
	}

	return NULL;
}

/* Returns if it should output a reference to something else */
static void GenBaseLabel(FILE *outFile, OS9ROF *rfile, LABTYPE labelType, u_int16 label)
{
	Node *node;
	Reference *ref;
	int length;
	BOOL labelGenerated;

	if(1 == disasmPass)
	{
		return;
	}

	labelGenerated = FALSE;

	node = ListGetHead(rfile->refList);

	while(NULL != node)
	{

		ref = NodeGetData(node);

		if(label == ref->offset)
		{

			switch(labelType)
			{
			case LABCODE:
				/* Break out if this is isn't a global ref */
				if(REF_GLOBAL != ref->type)
				{
					break;
				}

				if(0 != (ref->flag & CODENT))
				{
					length = GenAsm(outFile, "%s:", ref->symbol);
					labelGenerated = TRUE;
				}

				break;

			case LABDATA:

				/* Only global references are handled here */
				if(REF_GLOBAL != ref->type)
				{
					break;
				}
				
				/* Make sure we are referencing initializeddata */
				if(0 != (ref->flag & CODENT) && 0 == (ref->flag & INIENT))
				{
					break;
				}

				length = GenAsm(outFile, "%s:", ref->symbol);
				labelGenerated = TRUE;
				break;

			case LABBSS:
				length = GenAsmLabelBSS(outFile, label);
				labelGenerated = TRUE;
				break;

			default:
				assert(0);
			}


			if(TRUE == labelGenerated)
			{
				break;
			}
		}

		node = NodeGetNext(rfile->refList, node);
	}


	if(FALSE == labelGenerated)
	{
		length = 0;

		switch(labelType)
		{
		case LABCODE:			/* location is code, label is in code */
			if(TRUE == GET_CODEINFO(label))
			{
				if(TRUE == GET_TRACEINFO(label))
				{
					length = GenAsmLabelCode(outFile, label);
					labelGenerated = TRUE;
				}
				else
				{
					length = GenAsmLabelCodeData(outFile, label);
					labelGenerated = TRUE;
				}
			}
			break;
		case LABDATA:			/* location is init data */
			if(TRUE == GET_DATAINFO(label))
			{
				length = GenAsmLabelData(outFile, label);
				labelGenerated = TRUE;
			}
			break;
		case LABBSS:			/* location is uninit data */
			if(TRUE == GET_BSSINFO(label))
			{
				length = GenAsmLabelBSS(outFile, label);
				labelGenerated = TRUE;
			}
			break;
		default:
			assert(0);
		}
	}

	GenAsmTabs(outFile, length);
}



static void GenCodeLabel(FILE *outFile, OS9ROF *rfile, u_int16 pc, u_int16 label)
{
	if(pc > maxPC) 	/* It's outside of the code, just gen a label */
	{
		GenAsmAddress(outFile, label);
	}
	else
	{
		Node *node;
		Reference *ref;

		/* Get the first node of the reference list */
		node = ListGetHead(rfile->refList);

		while(NULL != node)
		{
			/* Get the node data */
			ref = NodeGetData(node);

			/* if it's the in code, the right offset, and right type, check it */
			if(pc == ref->offset && 0 != (ref->flag & CODLOC))
			{

				if(REF_LOCAL == ref->type)
				{
					/*
						Local references can target code, data, or bss so we need
						to check them all
					*/
					if(ref->flag & CODENT)
					{
						GenAsmLabelCode(outFile, label);
					}
					else
					{
						if(ref->flag & INIENT)
						{
							GenAsmLabelData(outFile, label);
						}
						else
						{
							GenAsmLabelBSS(outFile, label);
						}
					}
				}
				else
				{
					GenAsm(outFile, ref->symbol);
				}

				return;
			}

			node = NodeGetNext(rfile->refList, node);
		}

		/* Set that this code location as labeled/accessed */
		SET_CODEINFO(label, TRUE);
		
		/* Generate the proper code label */
		if(TRUE == GET_TRACEINFO(label))
		{
			GenAsmLabelCode(outFile, label);
		}
		else
		{
			GenAsmLabelCodeData(outFile, label);
		}

	}
}



/*************************************************************************** 

***************************************************************************/
static const char *IndexRegister(int postbyte)
{
	return Indexed_Register[ (postbyte>>5) & 0x03];
}



/*
	Searchs all reference data for locations which are
	accessed and have labels.
*/
static const char *GetGlobalSymbol(u_int16 pc, u_char mask)
{
	Reference *ref;

	ref = GetReference(rofFile, REF_GLOBAL, pc, TRUE, FALSE);
	if(NULL != ref)
	{
		return ref->symbol;
	}

	return NULL;
}


/*************************************************************************** 

***************************************************************************/
int DisasmPage10(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	pc++;
	code = mem[pc];
	return optable10[code].disasmFunc(outFile, mem, &optable10[code], code, pc) + 1;
}


int DisasmPage11(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	pc++;
	code = mem[pc];
	return optable11[code].disasmFunc(outFile, mem, &optable11[code], code, pc) + 1;
}


int DisasmIllegal(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	GenAsmOp(outFile, "nop", NULL);
	GenAsm(outFile, "* [%02X] Illegal instruction", mem[pc]);
	return op->byteCount;
}


int DisasmDirect(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	int offset;
	
	offset = mem[pc+1];
	GenAsmOp(outFile, op->opName, "$%02x", offset);
	return op->byteCount;
}



/* FIXME
	Immediate mode should check for accessing bss and data segments
	as well as inlined data in the code segment. PRobably safe to assume
	that if the location is labeled in the ROF data and/or is data in the
	code segment that we can access it as labeled. Either way, a special
	comment should be added
*/
int DisasmImmediate(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	int offset;
	
	offset = mem[pc + 1];
	GenAsmOp(outFile, op->opName, "#$%02x", offset);
	return op->byteCount;
}


int DisasmImmediateLong(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	int offset;
	
	offset = mem[pc + 1] * 256 + mem[pc + 2];
	GenAsmOp(outFile, op->opName, "#$%04x", offset);
	return op->byteCount;
}


int DisasmInherent(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	GenAsmOp(outFile, op->opName, NULL);
	return op->byteCount;
}


int DisasmIndexed(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	int postbyte;
	char *s;
	int extrabytes;
	int disp;
	int address;
	int offset;
	
	extrabytes = 0;
	postbyte = mem[pc + 1];

	if(0 == (postbyte & 0x80))
	{
		disp = postbyte & 0x1f;
		if ((postbyte & 0x10) == 0x10)
		{
			s = NEG_STRING;
			disp = 0x20 - disp;
		}
		else
		{
			s = POS_STRING;
		}

		/* FIXME - look for local symbol here */		
		GenAsmOp(outFile, op->opName, "%s$%02x,%s", s, disp,IndexRegister(postbyte));
		
	}
	else
	{
		BOOL indirect;

		/* Get the number of extra bytes for this postop */
		extrabytes = postOpExtraBytes[postbyte & 0x1f];

		/* Get the indirect flag */
		indirect = (postbyte & PB_INDIRECT) ? TRUE : FALSE;

		/* Generate the operand */
		GenAsmOp(outFile, op->opName, NULL);

		/* Check for indirect addressing */
		if(TRUE == indirect)
		{
			GenAsm(outFile, "[");
		}

		/* Process the postop */
		switch(postbyte & 0x0f)
		{
		case IDX_INCREG:
			GenAsm(outFile, ",%s+", IndexRegister(postbyte));
			if(TRUE == indirect)
			{
				GenAsm(outFile, "\t* Invalid indexing mode");
			}
			break;
			
		case IDX_INCREG2:
			GenAsm(outFile, ",%s++", IndexRegister(postbyte));
			break;

		case IDX_DECREG:
			GenAsm(outFile, ",-%s", IndexRegister(postbyte));
			if(TRUE == indirect)
			{
				GenAsm(outFile, "\t* Invalid indexing mode");
			}
			break;
			
		case IDX_DECREG2:
			GenAsm(outFile, ",--%s", IndexRegister(postbyte));
			break;
			
		case IDX_OFFSET_0:
			GenAsm(outFile, ",%s", IndexRegister(postbyte));
			break;
			
		case IDX_OFFSET_B:
			GenAsm(outFile, "b,%s", IndexRegister(postbyte));
			break;
			
		case IDX_OFFSET_A:
			GenAsm(outFile, "a,%s", IndexRegister(postbyte));
			break;
			
		case IDX_ILLEGAL1:
			break;
			
		case IDX_OFFSET_BYTE:
			offset = mem[pc+2];
			if (offset < 128)
			{
				s = POS_STRING;
			}
			else
			{
				s = NEG_STRING;
				offset = 0x0100 - offset;
			}
			GenAsm(outFile, "%s$%02x,%s", s, offset, IndexRegister(postbyte));
			break;
			
		case IDX_OFFSET_WORD:
			offset = mem[pc + 2] * 256 + mem[pc + 3];

			if (offset < 32768)
			{
				s = POS_STRING;
			}
			else
			{
				s = NEG_STRING;
				offset = 0xffff - offset + 1;
			}
			GenAsm(outFile, "%s", s);
			GenCodeLabel(outFile, rofFile, (u_int16)(pc + 2), (u_int16)offset);
			GenAsm(outFile, ",%s", IndexRegister(postbyte));
			break;
			
		case IDX_ILLEGAL2:
			GenAsm(outFile, "\t* Invalid indexing mode");
			break;
			
			
		case IDX_OFFSET_D:
			GenAsm(outFile, "d,%s", IndexRegister(postbyte));
			break;
			
		case IDX_OFFSET_PCR1:
			offset = (mem[pc + 2] + pc + 3) & 0xFFFF;
			GenAsm(outFile, "<$%02x,pcr", offset);
			break;
			
		case IDX_OFFSET_PCR2:
			offset = (mem[pc + 2] * 256 + mem[pc + 3] + pc + 4) & 0xFFFF;
			GenAsm(outFile, ">");
			GenCodeLabel(outFile, rofFile, (u_int16)(pc + 2), (u_int16)offset);
			GenAsm(outFile, ",pcr");
			break;

		case IDX_ILLEGAL3:
			GenAsm(outFile, "\t* Invalid indexing mode");
			break;
			
		case IDX_INDIRECT:
			address = mem[pc + 2] * 256 + mem[pc + 3];
			GenAsm(outFile, "$%4X", address);
			if(FALSE == indirect)
			{
				GenAsm(outFile, "\t* Invalid indexing mode");
			}
			break;
		}

		if(TRUE == indirect)
		{
			GenAsm(outFile, "]");
		}

	}

	return op->byteCount + extrabytes;
}


int DisasmExtended(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	u_int16 offset;
	
	offset = mem[pc + 1] * 256 + mem[pc + 2];

	GenAsmOp(outFile, op->opName, NULL);

	pc++;

	GenCodeLabel(outFile, rofFile, pc, offset);

	return op->byteCount;
}


int DisasmRelative(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	int offset;
	u_int16 disp;
	
	offset = mem[pc + 1];
	if (offset < 127 )
	{
		disp   = pc + 2 + offset;
	}
	else
	{
		disp   = pc + 2 - (256 - offset);
	}

	GenAsmOp(outFile, op->opName, NULL);
	GenCodeLabel(outFile, rofFile, ++pc, disp);

	return op->byteCount;
}


int DisasmRelativeLong(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	int offset;
	int disp;
	
	offset = mem[pc + 1] * 256 + mem[pc + 2];
	if (offset < 32767 )
	{
		disp   = pc + 3 + offset;
	}
	else
	{
		disp   = pc + 3 - (65536 - offset);
	}

	GenAsmOp(outFile, op->opName, NULL);
	GenCodeLabel(outFile, rofFile, ++pc, (u_int16)disp);

	return op->byteCount;
}


int DisasmRegToRegOp(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	int postbyte;
	
	postbyte = mem[pc + 1];
	
	GenAsmOp(outFile, op->opName, "%s,%s", Inter_Register[postbyte>>4], Inter_Register[postbyte & 0x0F]);
	
	return op->byteCount;
}



int DisasmStackOp(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc, const char **stackRegs)
{
	int postbyte;
	int i;
	int emitCount;
	
	postbyte = mem[pc + 1];	
	emitCount = 0;

	GenAsmOp(outFile, op->opName, NULL);
	
	for(i = 0; i < 8; i++)
	{
		if ((postbyte & stackRegBits[i]) != 0)
		{
			if (emitCount != 0)
			{
				GenAsm(outFile, ",");
			}
			GenAsm(outFile, stackRegs[i]);
			emitCount++;
		}
	}
	return op->byteCount;
}


int DisasmSystemStackOp(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	return DisasmStackOp(outFile, mem, op, code, pc, stackSRegTxt);
}


int DisasmUserStackOp(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	return DisasmStackOp(outFile, mem, op, code, pc, stackURegTxt);
}


int DisasmOS9SysCall(FILE *outFile, u_char *mem, Opcode *op, u_char code, u_int16 pc)
{
	int post;
	
	post = mem[pc + 1];
	
	GenAsmOp(outFile, op->opName, NULL);

	if(post < MAX_OS9CALLS && NULL != os9calls[post].callString)
	{
		GenAsm(outFile, "%s\t\t* [$%02X] %s", os9calls[post].callString, post, os9calls[post].callDesc);
	}
	else
	{
		GenAsm(outFile, "$%02x\t\t* Unknown OS-9 system call", post);
	}

	return op->byteCount;
}




/*************************************************************************** 

***************************************************************************/
static void DumpData(FILE *outFile,
					 u_char *mem,
					 u_int16 pc,
					 u_int16 toDump,
					 LABTYPE type)
{
	MODE	mode;
	MODE	lastMode;
	int		count;
	int		totalOut;
	BOOL	genLabel;
	Reference *ref;

	count = 0;
	totalOut = 0;
	mode = M_BINARY;
	lastMode = M_BINARY;
	ref = NULL;

	/*
		As long as we are within the code and the location we are processing is
		data, keep processing.
	*/
	while(toDump > 0)
	{
		lastMode = mode;

		/* Check for mode */
		if('"' != mem[pc] && mem[pc] >= 0x20 && mem[pc] <= 0x7f)
		{

			if(M_TEXT != mode)
			{
				/* Check the next character and switch modes ONLY if it is */
				if(1 != toDump && mem[pc + 1] >= 0x20 && mem[pc + 1] <= 0x7f)
				{
					count = 0;
					mode = M_TEXT;
				}
			}

		}
		else
		{

			if(M_BINARY != mode)
			{
				count = 0;
				mode = M_BINARY;
			}
		}

		/* Check for using a reference to renerate a target ref */
		if(LABDATA == type)
		{
			ref = FindDataRef(rofFile, pc);
			/* If we find a reference, make the mode binary and reset the count */
			if(NULL != ref)
			{
				count = 0;
				mode = M_BINARY;
			}
		}

		switch(type)
		{
		case LABDATA:
			genLabel = GET_DATAINFO(pc);
			break;
		case LABCODE:
			genLabel = GET_CODEINFO(pc);
			break;
		default:
			assert(0);
			break;
		}

		if(TRUE == genLabel)
		{
			count = 0;
		}

		/* If this is the beginning of the data line, do stuff */
		if(0 == count)
		{

			if(0 != totalOut)
			{
				/* If in text mode generate the terminator */
				if(M_TEXT == lastMode)
				{
					GenAsm(outFile, "\"");
				}

				GenAsm(outFile, "\n");
			}

			GenAsm(outFile, "%04X:\t", pc);

			GenBaseLabel(outFile, rofFile, type, pc);

			/* Output the data type contained */
			if(M_TEXT == mode)
			{
				assert(ref == NULL);
				GenAsm(outFile, "fcc\t\"");
			}
			else
			{
				if(NULL == ref)
				{
					GenAsm(outFile, "fcb\t");
				}
				else
				{
					u_int16 addr;

					addr = getWord(rofFile->initData, pc);

					GenAsm(outFile, "fdb\t");

					if(ref->flag & CODENT)
					{
						GenAsmLabelCode(outFile, addr);
					}
					else
					{
						if(0 != (ref->flag & INIENT))
						{
							GenAsmLabelData(outFile, addr);
						}
						else
						{
							GenAsmLabelBSS(outFile, addr);
						}
					}

					count = 0;
					totalOut += 2;
					pc += 2;
					toDump -= 2;
					continue;
				}
			}
		}

		if(M_TEXT == mode)
		{
			GenAsm(outFile, "%c", mem[pc]);
		}
		else
		{
			if(0 != count)
			{
				GenAsm(outFile, ",");
			}
			GenAsm(outFile, "$%02x", mem[pc]);
		}

		count++;

		if(M_TEXT == mode)
		{
			if(count >= MAX_FCCLENGTH)
			{
				count = 0;
			}
		}
		else
		{
			if(count >= MAX_FCBLENGTH)
			{
				count = 0;
			}
		}

		totalOut++;
		pc++;
		toDump--;
	}

	if(0 != totalOut)
	{
		if(M_TEXT == mode)
		{
			GenAsm(outFile, "\"");
		}
		GenAsm(outFile, "\n");
	}
}


/*************************************************************************** 

***************************************************************************/
static void DisasmDumpData(FILE *outFile, OS9ROF *rfile)
{
	u_int16 loc;

	// Dump the unitialized data
	if(0 != rfile->sizeUninitData)
	{
		loc = 0;
		GenAsm(outFile, "*\n* Uninitialized data ($%04X)\n*\n", rfile->sizeUninitData);
		GenAsm(outFile, "\tvsect\n");

		while(loc < rfile->sizeUninitData)
		{

			if(TRUE == GET_BSSINFO(loc))
			{
				u_int16 size = 0;
				u_int16 label = loc;
			
				do
				{
					size++;
					loc++;
				}
				while(loc < rfile->sizeUninitData && FALSE == GET_BSSINFO(loc));


				GenAsm(outFile, "%04X:\t", label);
				GenBaseLabel(outFile, rfile, LABBSS, label);
				GenAsm(outFile, "rmb\t%$%x\n", size);
			}
			else
			{
				loc++;
			}
		}
		GenAsm(outFile, "\tendsect\n*\n");
	}

	// Dump the initialized data
	if(0 != rfile->sizeInitData)
	{
		GenAsm(outFile, "*\n* Initialized data ($%04x)\n*\n", rfile->sizeInitData);
		GenAsm(outFile, "\tvsect\n");
		DumpData(outFile, rfile->initData, 0, rfile->sizeInitData, LABDATA);
		GenAsm(outFile, "\tendsect\n");
		GenAsm(outFile, "*\n*\n");
	}

}


/*************************************************************************** 

***************************************************************************/
static int DisasmDecode(FILE *outFile, u_char *mem)
{
	if(xxPC < maxPC)
	{
		u_char	code;

		code = mem[xxPC];

		if(TRACE_DATA != GET_TRACEINFO(xxPC))
		{
			// Print the location
			GenAsm(outFile, "%04X:\t", xxPC);

			/* Generate a base label for this location */
			GenBaseLabel(outFile, rofFile, LABCODE, xxPC);

			/* Go process the opcode */
			xxPC += optable[code].disasmFunc(outFile, mem, &optable[code], code, xxPC);

			GenAsm(outFile, "\n");
		}
		else
		{
			u_int16 pc;
			u_int16 count;

			GenAsm(outFile, "*\n");

			pc = xxPC;

			count = 0;
			while(TRACE_DATA == GET_TRACEINFO(xxPC) && xxPC < maxPC)
			{
				count++;
				xxPC++;
			}

			DumpData(outFile, mem, pc, count, LABCODE);
			GenAsm(outFile, "*\n");
		}
	}

	return xxPC >= maxPC ? -1 : 0;
}


/*************************************************************************** 

***************************************************************************/
static void DisasmSetPass(FILE *outFile, int pass)
{
	assert(pass > 0 && pass < 3);
	xxPC = 0;
	disasmPass = pass;

	if(2 == pass)
	{
		SetAsmOutputMode(TRUE);
		GenAsm(outFile, "\n*\n*\t%s\n*\n*\n", rofFile->name);
	}
	else
	{
		SetAsmOutputMode(FALSE);
	}
}


/*************************************************************************** 

***************************************************************************/
static void DisasmReset(FILE *outFile, OS9ROF *rof, int max)
{
	Node *node;

	rofFile = rof;
	maxPC = max;

	memset(codeInfo, 0, MAX_MEMORY);
	memset(dataInfo, 0, MAX_MEMORY);
	memset(bssInfo, 0, MAX_MEMORY);

	/* Set up reference information */
	node = ListGetHead(rof->refList);
	while(NULL != node)
	{
		Reference *ref;

		ref = NodeGetData(node);

		switch(ref->type)
		{
		case REF_GLOBAL:
			/*
				Global references gives an absolute address of the target.
				These are used to set either code, data, or bss information
				with the correct information to indicate that the specific
				target location is labeled.
			*/

			if(0 != (ref->flag & CODENT))
			{
				SET_CODEINFO(ref->offset, TRUE);
			}
			else
			{
				/* Check for references to data or bss*/
				if((ref->flag & INIENT))
				{	/* data */
					SET_DATAINFO(ref->offset, TRUE);
				}
				else
				{	/* bss */
					SET_BSSINFO(ref->offset, TRUE);
				}
			}
			break;

		case REF_EXTERNAL:
			/*
				External references really do not label any targets
				as labeled so we probably shouldn't process it
			*/
			if(0 != (ref->flag & CODLOC))
			{	/* code */
				TRACE_RESET(("extern code: %04X = %s\n", ref->offset, ref->symbol));
			}
			else
			{	/* data */
				TRACE_RESET(("extern data: %04X = %s\n", ref->offset, ref->symbol));
			}
			break;

		case REF_LOCAL:
			if(0 != (ref->flag & CODLOC))
			{	/* code */
				u_int16 address = getWord(rof->objectCode, ref->offset);

				TRACE_RESET(("local code: at %04X points to ", ref->offset));

				if(0 != (ref->flag & CODENT))
				{
					TRACE_RESET(("code"));
					SET_CODEINFO(address, TRUE);
				}
				else
				{
					/* Check for references to data */
					if(0 != (ref->flag & INIENT))
					{	/* data */
						TRACE_RESET(("data"));
						SET_DATAINFO(address, TRUE);
					}
					else 
					{ /* bss */
						TRACE_RESET(("bss"));
						SET_BSSINFO(address, TRUE);
					}
				}
				TRACE_RESET((" at %04X\n", address));
			}
			else
			{	/* data */
				/* Check for references to code */
				u_int16 address = getWord(rof->initData, ref->offset);

				TRACE_RESET(("local data: at %04X points to ", ref->offset));

				if(0 != (ref->flag & CODLOC))
				{
					TRACE_RESET(("code at %04X\n", ref->offset));
					SET_CODEINFO(address, TRUE);
				}
				else
				{
					/* Check for references to data */
					if(ref->flag & INIENT)
					{
						TRACE_RESET(("data"));
						SET_DATAINFO(address, TRUE);
					}
					else
					{ /* bss */
						TRACE_RESET(("bss"));
						SET_BSSINFO(address, TRUE);
					}
				}

				TRACE_RESET((" at %04X\n", address));
			}
			break;

		default:
			assert(0);
		}
	
		node = NodeGetNext(rof->refList, node);
	}

	DisasmSetPass(outFile, 1);
}


/*************************************************************************** 

***************************************************************************/
void DisasmObjectCode(FILE *outFile, OS9ROF *rfile)
{
	/* Reset the disassembler */
	DisasmReset(outFile, rfile, rfile->sizeObjectCode);

	/* Disassemble on the first pass */
	while(0 == DisasmDecode(outFile, rfile->objectCode));

	/* Set to the second pass */
	DisasmSetPass(outFile, 2);


	/* Output the preamble stuff */
	GenAsm(outFile, "*\n");
	GenAsm(outFile, "\tpsect %s\n", rfile->name);
	GenAsm(outFile, "*\n*\n\tvsect\n");

	/* Dump the data */
	DisasmDumpData(outFile, rfile);

	/* Disassemble the file */
	while(0 == DisasmDecode(outFile, rfile->objectCode));

	/* Output the end of the assembler file */
	GenAsm(outFile, "*\n\tendsect\n");
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
