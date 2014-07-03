// SegaCD Data Extractor
// ---------------------
// Written by Valley Bell, 17 April 2014
// Improved on 18 April 2014 (LaserActive detection fixed)

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include "stdtype.h"


#ifndef _MSC_VER
#define _stricmp	strcasecomp
#endif


UINT8 KDecomp(UINT32 SrcSize, const UINT8* SrcData, UINT32* RetDstSize, UINT8** RetDstData, UINT8 Moduled);

int main(int argc, char* argv[]);
void OpenROMFile(const char* FileName);
void ExtractSCDData(void);

void DumpFile(const char* Title, const char* Ext, UINT32 DataLen, const UINT8* Data);
UINT32 ScanForData(UINT32 DataLen, const UINT8* Data, UINT32 StartPos);
static UINT16 ReadBE16(const UINT8* Data);
static UINT32 ReadBE32(const UINT8* Data);
static UINT32 ReadBE24(const UINT8* Data);
static UINT16 ReadLE16(const UINT8* Data);
static UINT16 Kosinski_GetSize(UINT32 MaxDataSize, const UINT8* Data, UINT32* RetDstSize);


UINT32 ROMLength;
UINT8* ROMData;
char FileBase[0x200];
char* FileTitle;
UINT8 ReadMode;

int main(int argc, char* argv[])
{
	int argbase;
	UINT8 RetVal;
	
	printf("SegaCD Data Extractor\n");
	printf("---------------------\n");
	if (argc < 2)
	{
		printf("Usage: SCBIOS_Extract.c [-ReadMode] BIOS.bin\n\n");
		return 0;
	}
	
	ReadMode = 0x00;
	argbase = 1;
	while(argbase < argc && argv[argbase][0] == '-')
	{
		if (! _stricmp(argv[argbase] + 1, "ReadMode"))
			ReadMode = 0x01;
		else
			break;
		argbase ++;
	}
	
	if (argc <= argbase + 0x00)
	{
		printf("Not enough arguments!\n");
		return 9;
	}
	
	OpenROMFile(argv[argbase]);
	if (! ROMLength)
		return 1;
	
	RetVal = 0;
	ExtractSCDData();
	
#ifdef _DEBUG
	getchar();
#endif
	printf("\n");
	
	return RetVal;
}

void OpenROMFile(const char* FileName)
{
	FILE* hFile;
	char* TempPnt;
	
	ROMLength = 0x00;
	
	hFile = fopen(FileName, "rb");
	if (hFile == NULL)
	{
		printf("Error opening file!\n");
		return;
	}
	
	fseek(hFile, 0, SEEK_END);
	ROMLength = ftell(hFile);
	
	fseek(hFile, 0, SEEK_SET);
	ROMData = (UINT8*)malloc(ROMLength);
	fread(ROMData, 0x01, ROMLength, hFile);
	
	fclose(hFile);
	
	strcpy(FileBase, FileName);
	TempPnt = strrchr(FileBase, '.');
	if (TempPnt != NULL)
		*TempPnt = 0x00;
	
	TempPnt = strrchr(FileBase, '\\');
	if (TempPnt == NULL)
		TempPnt = FileBase - 1;
	FileTitle = strrchr(FileBase, '/');
	if (FileTitle > TempPnt)
		TempPnt = FileTitle;
	
	TempPnt ++;
	FileTitle = TempPnt + 1;
	memmove(FileTitle, TempPnt, strlen(TempPnt) + 0x01);
	*TempPnt = '\0';
	printf("File name: %s\n", FileTitle);
	
	return;
}

void ExtractSCDData(void)
{
	const UINT8 MAGIC_BYT_Z80[0x08] = {0x4D, 0xF9, 0x00, 0xA0, 0x00, 0x00, 0x4B, 0xF9};
	const UINT8 MAGIC_BYT_68KCD[0x08] = {0x43, 0xF9, 0x00, 0x02, 0x00, 0x00, 0x61, 0x00};
	
	UINT32 KosDecAddr;
	
	UINT32 CurPos;
	UINT32 SrcAddr;
	UINT32 DstAddr;
	UINT32 DataSize;
	UINT32 DataSizeUnc;
	UINT8* KosDecBuf;
	
	UINT8 LoopNum;
	UINT8 NameBuf[0x20];
	
	// 1. get Z80 data
	printf("Z80 Data:\n");
	LoopNum = 0x00;
	CurPos = ScanForData(0x08, MAGIC_BYT_Z80, 0x00);
	while(CurPos)
	{
		printf("    Loader code: %06X\n", CurPos);
		while(ROMData[CurPos + 0x01] == 0xF9)
		{
			// us_scd1_921011:
			//	001078	4DF9 00A0 0000	LEA		$A00000.l, A6
			//	00107E	4BF9 0001 4000	LEA		$014000.l, A5
			//	001084	3E3C 0CA5		MOVE.W	#$0CA5, D7
			//	001088	1CDD			MOVE.B	(A5)+, (A6)+
			//	00108A	51CF FFFC		DBRA	D7, $001088
			//	00108E	...				-- repeat --
			DstAddr = ReadBE24(&ROMData[CurPos + 0x00 + 0x02]) & 0xFFFF;
			SrcAddr = ReadBE24(&ROMData[CurPos + 0x06 + 0x02]);
			DataSize = ReadBE16(&ROMData[CurPos + 0x0C + 0x02]) + 1;
			
			printf("\tROM Ofs %06X, Z80 Ofs %04X, Size %04X\n",
					SrcAddr, DstAddr, DataSize);
			
			if (! LoopNum)
				sprintf(NameBuf, "Z80_%04X", DstAddr);
			else
				sprintf(NameBuf, "Z80%c_%04X", 'a' + LoopNum, DstAddr);
			DumpFile(NameBuf, "raw", DataSize, &ROMData[SrcAddr]);
			CurPos += 0x16;
		}
		
		CurPos = ScanForData(0x08, MAGIC_BYT_Z80, CurPos);
		LoopNum ++;
	}
	
	printf("CD 68k Data:\n");
	KosDecAddr = 0x00;
	LoopNum = 0x00;
	CurPos = ScanForData(0x08, MAGIC_BYT_68KCD, 0x06);
	while(CurPos)
	{
		CurPos -= 0x06;
		printf("    Loader code: %06X\n", CurPos);
		while(ROMData[CurPos + 0x01] == 0xF9)
		{
			// us_scd1_921011:
			//	00066E	41F9 0001 5800	LEA		$015800.l, A0
			//	000674	43F9 0002 0000	LEA		$020000.l, A1	; SegaCD 68k RAM
			//	00067A	6100 0286		BSR.W	$000902			; call Kosinski Decompression
			//	00067E	4A38 FE53		TST.B	$FE53.w		; [present only after the first block]
			//	000682	6704 			BEQ.S	$000688		;      --''--
			//	000684	...				...
			
			//	000688	41F9 0001 3400	LEA		$013400.l, A0
			//	00068E	43F9 0002 6000	LEA		$026000.l, A1	; SegaCD 68k RAM
			//	000694	6100 026C		BSR.W	$000902			; call Kosinski Decompression
			
			//	000698	41F9 0001 9800	LEA		$019800.l, A0
			//	...
			//
			// us_mld_930922: (LaserActive)
			//	00087C	41F9 0000 D500	LEA		$00D500.l, A0
			//	000882	43F9 0002 0000	LEA		$020000.l, A1	; SegaCD 68k RAM
			//	000888	6100 0190		BSR.W	$000A1A			; call Kosinski Decompression
			//	00088C	12FC 0000		MOVE.B	#0, (A1)+	; [present only after the first block]
			//	000890	B3FC 0002 5400	CMPA.L	$025400, A1	;      --''--
			//	000896	65F4			BLO.S	$00088C
			
			//	000898	41F9 0000 B900	LEA		$00B900.l, A0
			//	00089E	43F9 0002 6000	LEA		$026000.l, A1	; SegaCD 68k RAM
			//	0008A4	6100 0174		BSR.W	$000A1A			; call Kosinski Decompression
			SrcAddr = ReadBE24(&ROMData[CurPos + 0x00 + 0x02]);
			DstAddr = ReadBE24(&ROMData[CurPos + 0x06 + 0x02]) & 0x1FFFF;
			
			DataSizeUnc = CurPos + 0x0E + ReadBE16(&ROMData[CurPos + 0x0C + 0x02]);
			if (KosDecAddr != DataSizeUnc)
			{
				KosDecAddr = DataSizeUnc;
				printf("    Kosinski Decompression routine: %06X\n", KosDecAddr);
			}
			
			DataSize = Kosinski_GetSize(ROMLength - SrcAddr, &ROMData[SrcAddr], &DataSizeUnc);
			printf("\tROM Ofs %06X, CD Ofs %06X, Size %04X (cmp), %04X (uncmp)\n",
					SrcAddr, DstAddr, DataSize, DataSizeUnc);
			
			if (! LoopNum)
				sprintf(NameBuf, "CD_%05X", DstAddr);
			else
				sprintf(NameBuf, "CD%c_%05X", 'a' + LoopNum, DstAddr);
			DumpFile(NameBuf, "kos", DataSize, &ROMData[SrcAddr]);
			CurPos += 0x10;
			
			KosDecBuf = (UINT8*)malloc(DataSizeUnc);
			KDecomp(DataSize, &ROMData[SrcAddr], &DataSizeUnc, &KosDecBuf, 0);
			DumpFile(NameBuf, "raw", DataSizeUnc, KosDecBuf);
			free(KosDecBuf);	KosDecBuf  = NULL;
			
			if (ROMData[CurPos] == 0x12 && ROMData[CurPos + 0x0A] == 0x65)
			{
				CurPos += 0x04 + 0x06 + 0x02;
			}
			if (ROMData[CurPos] == 0x4A && ROMData[CurPos + 0x04] == 0x67)
			{
				CurPos += 0x04;
				CurPos += 0x02 + ROMData[CurPos + 0x01];
			}
		}
		
		CurPos = ScanForData(0x08, MAGIC_BYT_68KCD, CurPos);
		LoopNum ++;
	}
	
	return;
}

void DumpFile(const char* Title, const char* Ext, UINT32 DataLen, const UINT8* Data)
{
	FILE* hFile;
	char FileName[0x200];
	
	if (ReadMode)
		return;
	
	if (Ext == NULL)
		Ext = "bin";
	sprintf(FileName, "%s%s_%s.%s", FileBase, FileTitle, Title, Ext);
	hFile = fopen(FileName, "wb");
	if (hFile == NULL)
	{
		printf("Error opening file %s!\n", FileName + strlen(FileBase));
		return;
	}
	
	fwrite(Data, 0x01, DataLen, hFile);
	
	fclose(hFile);
	
	return;
}


UINT32 ScanForData(UINT32 DataLen, const UINT8* Data, UINT32 StartPos)
{
	UINT32 CurPos;
	
	//printf("Trying to search for Music Pointer List ...");
	for (CurPos = StartPos; CurPos < ROMLength - DataLen; CurPos ++)
	{
		if (! memcmp(ROMData + CurPos, Data, DataLen))
		{
			//printf("  found at %06X\n", CurPos);
			return CurPos;
		}
	}
	//printf("  not found.\n");
	
	return 0x00;
}


static UINT16 ReadBE16(const UINT8* Data)
{
	return (Data[0x00] << 8) | (Data[0x01] << 0);
}

static UINT32 ReadBE32(const UINT8* Data)
{
	return	(Data[0x00] << 24) | (Data[0x01] << 16) |
			(Data[0x02] <<  8) | (Data[0x03] <<  0);
}

static UINT32 ReadBE24(const UINT8* Data)
{
	return	(Data[0x01] << 16) |
			(Data[0x02] <<  8) | (Data[0x03] <<  0);
}

static UINT16 ReadLE16(const UINT8* Data)
{
	return (Data[0x01] << 8) | (Data[0x00] << 0);
}



static UINT16 Kosinski_GetSize(UINT32 MaxDataSize, const UINT8* Data, UINT32* RetDstSize)
{
	UINT32 SrcPos;
	UINT32 DstPos;
	
// Bitfield Infos
	UINT16 BITFIELD;
	UINT8 BFP;
	UINT8 Bit;
	
// R/W infos
	UINT8 Low, High;
	
// Count and Offest
	UINT32 Count;
	//UINT32 Offset;
	//UINT32 i;
	
	SrcPos = 0x00;
	DstPos = 0x00;
	
	BITFIELD = ReadLE16(&Data[SrcPos]);
	SrcPos += 2;
	BFP = 0;
	while(SrcPos < MaxDataSize)
	{
		if (BITFIELD & (1 << BFP++))
			Bit = 1;
		else
			Bit = 0;
		if (BFP >= 16)
		{
			BITFIELD = ReadLE16(&Data[SrcPos]);
			SrcPos += 2;
			BFP = 0;
		}
//-- Direct Copy ---------------------------------------------------------------------------------
		if (Bit)
		{
			SrcPos ++;	DstPos ++;
		}
		else
		{
			if (BITFIELD & (1 << BFP++))
				Bit = 1;
			else
				Bit = 0;
			if (BFP>=16)
			{
				BITFIELD = ReadLE16(&Data[SrcPos]);
				SrcPos += 2;
				BFP = 0;
			}
//-- Embedded / Separate -------------------------------------------------------------------------
			if (Bit)
			{
				Low = Data[SrcPos ++];
				High = Data[SrcPos ++];
				
				Count = High & 0x07;
				
				if (Count == 0)
				{
					Count = Data[SrcPos ++];
					if (Count == 0)
						break;
					if (Count == 1)
						continue;
				}
				else
				{
					Count ++;
				}
				
				//Offset = 0xFFFFE000 | ((0xF8 & High) << 5) | Low;
			}
//-- Inline --------------------------------------------------------------------------------------
			else
			{
				if (BITFIELD & (1 << BFP++))
					Low = 1;
				else
					Low = 0;
				if (BFP >= 16)
				{
					BITFIELD = ReadLE16(&Data[SrcPos]);
					SrcPos += 2;
					BFP = 0;
				}
				if (BITFIELD & (1 << BFP++))
					High = 1;
				else
					High = 0;
				if (BFP >= 16)
				{
					BITFIELD = ReadLE16(&Data[SrcPos]);
					SrcPos += 2;
					BFP = 0;
				}
				
				Count = Low * 2 + High + 1;
				
				//Offset = 0xFFFFFF00 | Data[SrcPos ++];
				SrcPos ++;
			}
//-- Write to file for indirect copy -------------------------------------------------------------
			DstPos += Count + 1;
//------------------------------------------------------------------------------------------------
		}
	}
//------------------------------------------------------------------------------------------------
	
	if (RetDstSize != NULL)
		*RetDstSize = DstPos;
	return SrcPos;
}
