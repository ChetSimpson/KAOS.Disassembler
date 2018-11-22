/*****************************************************************************
	os9calls.c	- Table of OS-9 SWI calls

	Copyright (C) 1996, 1997, 2014, 2018, by Chet Simpson.

	This file is distributed under the MIT License. See notice at the end
	of this file.

*****************************************************************************/
#include "disasm.h"

OS9Call os9calls[MAX_OS9CALLS] =
{	
	{"F$Link",		"Link to Module"},
	{"F$Load",		"Load Module from File"},
	{"F$UnLink",	"Unlink Module"},
	{"F$Fork",		"Start New Process"},
	{"F$Wait",		"Wait for Child Process to Die"},
	{"F$Chain",		"Chain Process to New Module"},
	{"F$Exit",		"Terminate Process"},
	{"F$Mem",		"Set Memory Size"},
	{"F$Send",		"Send Signal to Process"},
	{"F$Icpt",		"Set Signal Intercept"},
	{"F$Sleep",		"Suspend Process"},
	{"F$SSpd",		"Suspend Process"},
	{"F$ID",		"Return Process ID"},
	{"F$SPrior",	"Set Process Priority"},
	{"F$SSWI",		"Set Software Interrupt"},
	{"F$PErr",		"Print Error"},

	{"F$PrsNam",	"Parse Pathlist Name"},
	{"F$CmpNam",	"Compare Two Names"},
	{"F$SchBit",	"Search Bit Map"},
	{"F$AllBit",	"Allocate in Bit Map"},
	{"F$DelBit",	"Deallocate in Bit Map"},
	{"F$Time",		"Get Current Time"},
	{"F$STime",		"Set Current Time"},
	{"F$CRC",		"Generate CRC"},
	{"F$GPrDsc",	"get Process Descriptor copy"},
	{"F$GBlkMp",	"get System Block Map copy"},
	{"F$GModDr",	"get Module Directory copy"},
	{"F$CpyMem",	"Copy External Memory"},
	{"F$SUser",		"Set User ID number"},
	{"F$UnLoad",	"Unlink Module by name"},
	{"F$Alarm",		"Color Computer Alarm Call"},
	{NULL,			NULL}, /* reserved */

	{NULL,			NULL}, /* reserved */
	{"F$NMLink",	"Color Computer NonMapping Link"},
	{"F$NMLoad",	"Color Computer NonMapping Load"},
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{"F$VIRQ",		"Install/Delete Virtual IRQ"},
	{"F$SRqMem",	"System Memory Request"},
	{"F$SRtMem",	"System Memory Return"},
	{"F$IRQ",		"Enter IRQ Polling Table"},
	{"F$IOQu",		"Enter I/O Queue"},
	{"F$AProc",		"Enter Active Process Queue"},
	{"F$NProc",		"Start Next Process"},
	{"F$VModul",	"Validate Module"},
	{"F$Find64",	"Find Process/Path Descriptor"},

	{"F$All64",		"Allocate Process/Path Descriptor"},
	{"F$Ret64",		"Return Process/Path Descriptor"},
	{"F$SSvc",		"Service Request Table Initialization"},
	{"F$IODel",		"Delete I/O Module"},
	{"F$SLink",		"System Link"},
	{"F$Boot",		"Bootstrap System"},
	{"F$BtMem",		"Bootstrap Memory Request"},
	{"F$GProcP",	"Get Process ptr"},
	{"F$Move",		"Move Data (low bound first)"},
	{"F$AllRAM",	"Allocate RAM blocks"},
	{"F$AllImg",	"Allocate Image RAM blocks"},
	{"F$DelImg",	"Deallocate Image RAM blocks"},
	{"F$SetImg",	"Set Process DAT Image"},
	{"F$FreeLB",	"Get Free Low Block"},
	{"F$FreeHB",	"Get Free High Block"},
	{"F$AllTsk",	"Allocate Process Task number"},

	{"F$DelTsk",	"Deallocate Process Task number"},
	{"F$SetTsk",	"Set Process Task DAT registers"},
	{"F$ResTsk",	"Reserve Task number"},
	{"F$RelTsk",	"Release Task number"},
	{"F$DATLog",	"Convert DAT Block/Offset to Logical"},
	{"F$DATTmp",	"Make temporary DAT image (Obsolete)"},
	{"F$LDAXY",		"Load A [X,[Y]]"},
	{"F$LDAXYP",	"Load A [X+,[Y]]"},
	{"F$LDDDXY",	"Load D [D+X,[Y]]"},
	{"F$LDABX",		"Load A from 0,X in task B"},
	{"F$STABX",		"Store A at 0,X in task B"},
	{"F$AllPrc",	"Allocate Process Descriptor"},
	{"F$DelPrc",	"Deallocate Process Descriptor"},
	{"F$ELink",		"Link using Module Directory Entry"},
	{"F$FModul",	"Find Module Directory Entry"},
	{"F$MapBlk",	"Map Specific Block"},

	{"F$ClrBlk",	"Clear Specific Block"},
	{"F$DelRAM",	"Deallocate RAM blocks"},
	{"F$GCMDir",	"Pack module directory"},
	{"F$AlHRam",	"Allocate HIGH RAM Blocks"},
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */

	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */

	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */
	{NULL,			NULL}, /* reserved */

	{"I$Attach",	"Attach I/O Device"},
	{"I$Detach",	"Detach I/O Device"},
	{"I$Dup",		"Duplicate Path"},
	{"I$Create",	"Create New File"},
	{"I$Open",		"Open Existing File"},
	{"I$MakDir",	"Make Directory File"},
	{"I$ChgDir",	"Change Default Directory"},
	{"I$Delete",	"Delete File"},
	{"I$Seek",		"Change Current Position"},
	{"I$Read",		"Read Data"},
	{"I$Write",		"Write Data"},
	{"I$ReadLn",	"Read Line of ASCII Data"},
	{"I$WritLn",	"Write Line of ASCII Data"},
	{"I$GetStt",	"Get Path Status"},
	{"I$SetStt",	"Set Path Status"},
	{"I$Close",		"Close Path"},
	{"I$DeletX",	"Delete from current exec dir"}
};



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
