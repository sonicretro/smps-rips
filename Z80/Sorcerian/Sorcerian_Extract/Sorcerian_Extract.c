// Sorcerian Extractor and Decompressor
// ------------------------------------
// Written by Valley Bell on 28th March 2014

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>	// for NULL
#include <malloc.h>
#include <string.h>

#include "stdtype.h"


int main(int argc, char* argv[]);
void OpenROMFile(const char* FileName);
void ExtractFile(UINT32 Offset, UINT16 FileID);
static UINT16 ReadBE16(const UINT8* Data);

void DecompressLZSS(const UINT8* InData, UINT16* RetOutSize, UINT8** RetOutData);


UINT32 ROMLength;
UINT8* ROMData;
char FileBase[0x200];
char* FileTitle;

int main(int argc, char* argv[])
{
	int argbase;
	UINT16 FileCount;
	UINT32 BaseOfs;
	
	printf("Sorcerian Extractor\n");
	printf("-------------------\n");
	if (argc < 4)
	{
		printf("Usage: sorcdec.exe ROM.bin FileCount Offset\n");
		printf("FileCount ==  0 - single file mode\n");
		printf("FileCount >=  1 - multi file mode (offset is 16-bit pointer list)\n");
		printf("FileCount == -1 - multi file mode, file count autodetection\n");
		return 0;
	}
	
	argbase = 1;
	/*if (argc <= argbase + 0x00)
	{
		printf("Not enough arguments!\n");
		return 9;
	}*/
	
	OpenROMFile(argv[argbase + 0]);
	if (! ROMLength)
		return 1;
	
	FileCount = (UINT16)strtol(argv[argbase + 1], NULL, 0);
	BaseOfs = (UINT32)strtoul(argv[argbase + 2], NULL, 0);
	
	if (! FileCount)
	{
		ExtractFile(BaseOfs, 0x00);
	}
	else
	{
		UINT16 CurFile;
		UINT16 FilePtr;
		UINT32 CurPos;
		
		if (FileCount == 0xFFFF)
		{
			UINT16 MaxOfs;
			
			MaxOfs = 0xFFFF;
			CurPos = BaseOfs;
			for (CurFile = 0x00; CurFile < FileCount; CurFile ++, CurPos += 0x02)
			{
				if (CurPos >= BaseOfs + MaxOfs)
					break;
				FilePtr = ReadBE16(&ROMData[CurPos]);
				if (FilePtr < MaxOfs)
					MaxOfs = FilePtr;
			}
			FileCount = CurFile;
			printf("List contains 0x%02hX files.\n", FileCount);
		}
		
		CurPos = BaseOfs;
		for (CurFile = 0x00; CurFile < FileCount; CurFile ++, CurPos += 0x02)
		{
			FilePtr = ReadBE16(&ROMData[CurPos]);
			ExtractFile(BaseOfs + FilePtr, CurFile);
		}
	}
	
	free(ROMData);
	
#ifdef _DEBUG
	getchar();
#endif
	
	return 0;
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
	
	return;
}

void ExtractFile(UINT32 Offset, UINT16 FileID)
{
	char FileName[0x200];
	UINT16 InSize;
	UINT16 OutSize;
	UINT8* OutData;
	FILE* hFile;
	
	sprintf(FileName, "%s%s_%02X.%s", FileBase, FileTitle, FileID, "bin");
	
	hFile = fopen(FileName, "wb");
	if (hFile == NULL)
	{
		printf("Error opening file!\n");
		return;
	}
	
	InSize = ReadBE16(&ROMData[Offset]);
	printf("File %02X: 0x%04X bytes -> ", FileID, InSize);
	
	DecompressLZSS(&ROMData[Offset], &OutSize, &OutData);
	if (OutData != NULL)
		printf("%04X bytes\n", OutSize);
	
	fwrite(OutData, 0x01, OutSize, hFile);
	
	fclose(hFile);
	free(OutData);
	
	return;
}

static UINT16 ReadBE16(const UINT8* Data)
{
	return (Data[0x00] << 8) | (Data[0x01] << 0);
}


void DecompressLZSS(const UINT8* InData, UINT16* RetOutSize, UINT8** RetOutData)
{
	UINT8 StackBuf[0x1000];	// register A1
	UINT16 InSize;
	UINT16 OutSize;
	UINT8* OutData;
	UINT16 InPos;		// register A0
	UINT16 OutPos;		// register A4
	UINT16 RemBytes;	// register D0
	UINT16 StackPos;	// register D1
	UINT16 RemBits;		// register D2
	UINT16 BitBuf;		// register D3
	UINT16 CopyPos;		// register D4
	UINT16 CopyBytes;	// register D5
	
	memset(StackBuf, 0x00, 0x1000);	// Note: offical LZSS lib. fills with spaces
	
	InPos = 0x00;
	OutPos = 0x00;
	
	InSize = ReadBE16(&InData[InPos]) + 1;
	OutSize = 0x4000;	// 16 KB should be enough
	OutData = (UINT8*)malloc(OutSize);
	InPos += 0x02;
	
	RemBytes = InSize;
	StackPos = 0x1000 - 18;
	RemBits = 0;
	do
	{
		// 0074B8
		if (! RemBits)
		{
			// 0074BC
			RemBits = 8;
			BitBuf = InData[InPos];
			InPos ++;	RemBytes --;
		}
		RemBits --;
		
		BitBuf <<= 1;
		if (BitBuf & 0x100)	// actually checks for the Carry bit of an 8-bit shift
		{
			if (OutPos >= OutSize)
				goto Error;
			// 0074C6
			OutData[OutPos] = InData[InPos];
			StackBuf[StackPos] = InData[InPos];
			InPos ++;	OutPos ++;
			StackPos ++;	StackPos &= 0xFFF;
		}
		else
		{
			// 0074D4
			CopyPos = ReadBE16(&InData[InPos]);
			InPos += 0x02;
			
			CopyBytes = (CopyPos & 0x0F) + 3;
			CopyPos >>= 4;
			do
			{
				if (OutPos >= OutSize)
					goto Error;
				// 0074E4
				OutData[OutPos] = StackBuf[CopyPos];
				StackBuf[StackPos] = StackBuf[CopyPos];
				OutPos ++;
				CopyPos ++;		CopyPos &= 0xFFF;
				StackPos ++;	StackPos &= 0xFFF;
				CopyBytes --;
			} while(CopyBytes);
			RemBytes --;
		}
		// 0074FE
		RemBytes --;
	} while(RemBytes);
	OutSize = OutPos;
	
	*RetOutSize = OutSize;
	*RetOutData = OutData;
	return;

Error:
	printf("Decompression failure at 0x%04X!\n", InPos);
	*RetOutData = NULL;
	
	return;
}
