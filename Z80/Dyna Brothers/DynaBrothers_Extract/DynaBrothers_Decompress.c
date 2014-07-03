// Dyna Brothers Decompressor
// --------------------------
// Written by Valley Bell on 19th April 2014
// Decompression Type 02 added on 23th April 2014

/*
Decompression routine locations:
	01	009858
	02	0098F0
	03	009996

Offsets with example data:
	01	00158A (03B3 -> 0558), 0FA360 (015D -> 02C0)
	02	020828 (05A8 -> 0A00), 020DD0 (082C -> 1000)
	03	0FCF32 (0FA2 -> 1E2A), 000792 (0DF6 -> 0EF4)
*/

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

void Decompress13(UINT16 MaxInSize, const UINT8* InData, UINT16* RetInSize,
				  UINT16* RetOutSize, UINT8** RetOutData);
void Decompress2(UINT16 MaxInSize, const UINT8* InData, UINT16* RetInSize,
				 UINT16* RetOutSize, UINT8** RetOutData);


UINT32 ROMLength;
UINT8* ROMData;
char FileBase[0x200];
char* FileTitle;

int main(int argc, char* argv[])
{
	int argbase;
	UINT16 FileCount;
	UINT32 BaseOfs;
	
	printf("Dyna Brothers Decompressor\n");
	printf("--------------------------\n");
	if (argc < 4)
	{
		printf("Usage: dynabdec.exe ROM.bin FileCount Offset\n");
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
	UINT8 ComprType;
	FILE* hFile;
	
	ComprType = ROMData[Offset + 0x02];
	if (ComprType < 0x01 || ComprType > 0x03)
	{
		printf("Bad compression type: %02X!\n", ComprType);
		return;
	}
	
	sprintf(FileName, "%s%s_%02X.%s", FileBase, FileTitle, FileID, "bin");
	
	hFile = fopen(FileName, "wb");
	if (hFile == NULL)
	{
		printf("Error opening file!\n");
		return;
	}
	
	if (ComprType & 0x01)
		Decompress13(0x8000, &ROMData[Offset], &InSize, &OutSize, &OutData);
	else
		Decompress2(0x8000, &ROMData[Offset], &InSize, &OutSize, &OutData);
	printf("File %02X (Compr %02X): 0x%04X bytes -> %04X bytes\n", FileID, ComprType, InSize, OutSize);
	
	fwrite(OutData, 0x01, OutSize, hFile);
	
	fclose(hFile);
	free(OutData);
	
	return;
}

static UINT16 ReadBE16(const UINT8* Data)
{
	return (Data[0x00] << 8) | (Data[0x01] << 0);
}


void Decompress13(UINT16 MaxInSize, const UINT8* InData, UINT16* RetInSize,
				  UINT16* RetOutSize, UINT8** RetOutData)
{
	UINT16 OutSize;		// register D7
	UINT8* OutData;
	UINT8 ComprType;
	UINT8 CtrlByte;		// register D0 - LZ control byte
	UINT16 InPos;		// register A5
	UINT16 OutPos;		// register A4
	UINT16 BytCopy;		// register D2 [0099BA] / D1 [0099E8] - bytes to copy
	UINT16 Offset;		// register D1
	
	InPos = 0x00;
	OutSize = ReadBE16(&InData[InPos]);
	// Note: Incorrectly compressed files can write beyond the file boundary,
	//       so I'm increasing the buffer size by the amount of bytes it writes at once.
	//       Example: Dyna Brothers 1: SMPS Z80 sound driver (size 0EF4, decompresses 0EF5 bytes)
	OutData = (UINT8*)malloc(OutSize + 0x80);
	InPos += 0x02;
	
	ComprType = InData[InPos];	// Compression Type (can be 01, 02, 03)
	InPos ++;
	
	OutPos = 0x00;
	while(OutPos < OutSize)
	{
		if (InPos >= MaxInSize)
		{
			printf("Input data too small!!\n");
			goto Error;
		}
		
		// 009874/0099B2
		CtrlByte = InData[InPos];	InPos ++;
		if (CtrlByte < 0x80)
		{
			// 00987C/0099BA - direct copy input->output
			BytCopy = CtrlByte + 1;
			memcpy(&OutData[OutPos], &InData[InPos], BytCopy);
			InPos += BytCopy;	OutPos += BytCopy;
		}
		else
		{
			if (ComprType == 0x01)
			{
				// 0098AA - reuse previously generated data (8-bit window)
				BytCopy = (CtrlByte & 0x7F) + 1;
				Offset = InData[InPos];
				InPos ++;
			}
			else //if (ComprType == 0x03)
			{
				// 0099E8 - reuse previously generated data (8/16-bit window)
				BytCopy = (CtrlByte & 0x3F) + 1;
				if (CtrlByte & 0x40)
				{
					Offset = ReadBE16(&InData[InPos]);
					InPos += 0x02;
				}
				else
				{
					Offset = InData[InPos];
					InPos ++;
				}
			}
			Offset ++;
			if (Offset > OutPos)
			{
				printf("Copying from beyond start of file! (OutPos: %04X, Offset -%04X)\n",
						OutPos, Offset);
				goto Error;
			}
			
			// I can't use memcpy/memmove, because the buffers can (and will) overlap.
			// In that case, it is supposed to overwrite data and copy it then.
			//memcpy(&OutData[OutPos], &OutData[OutPos - Offset], BytCopy);
			//OutPos += BytCopy;
			while(BytCopy)
			{
				OutData[OutPos] = OutData[OutPos - Offset];
				OutPos ++;	BytCopy --;
			}
		}
	}
	if (OutPos != OutSize)
	{
		printf("Warning! Size is %04X, but decompressed %04X bytes!\n", OutSize, OutPos);
		OutSize = OutPos;
	}
	
	if (RetInSize != NULL)	// this one is optional
		*RetInSize = InPos;
	*RetOutSize = OutSize;
	*RetOutData = OutData;
	return;

Error:
	printf("Decompression failure at 0x%04X!\n", InPos);
	*RetOutData = NULL;
	free(OutData);
	
	return;
}

static UINT16 Lookup_00997A[7][2] =
{	{0, 0x00},
	{3, 0x01},
	{4, 0x09},
	{5, 0x19},
	{6, 0x40},
	{7, 0x80},
	{3, 0x39}};

void Decompress2(UINT16 MaxInSize, const UINT8* InData, UINT16* RetInSize,
				 UINT16* RetOutSize, UINT8** RetOutData)
{
	UINT16 OutSize;		// register D7
	UINT8* OutData;
	UINT8 ComprType;
	
	UINT8 BitCount;		// register D6
	UINT16 BitMask;		// register D5 (actually UINT8, but we can't check the Carry bit, which is 0x100 here)
	UINT16 InPos_Base;	// register A0
	UINT16 InPos_Data;	// register D0
	UINT16 InPos_Huff;	// register A1
	UINT16 OutPos;		// register A2
	UINT8 TblIdx;		// register D0
	UINT16 ByteCount;	// register D1
	
	InPos_Base = 0x00;
	OutSize = ReadBE16(&InData[InPos_Base]);	// The ROM actually treats 0 as 0x10000.
	OutData = (UINT8*)malloc(OutSize);
	InPos_Base += 0x02;
	
	ComprType = InData[InPos_Base];	// Compression Type (can be 01, 02, 03)
	InPos_Base ++;
	
	InPos_Huff = InData[InPos_Base];
	InPos_Base ++;
	InPos_Huff += InPos_Base;
	
	BitCount = 0x00;
	BitMask = InData[InPos_Huff];
	InPos_Huff ++;
	for (OutPos = 0x00; OutPos < OutSize; OutPos ++)
	{
		// 009914
		TblIdx = 0;
		do
		{
			if (BitCount == 8)
			{
				if (InPos_Huff >= MaxInSize)
				{
					printf("Input data too small!!\n");
					goto Error;
				}
				
				// 00991E
				BitMask = InData[InPos_Huff];
				InPos_Huff ++;
				BitCount = 0;
			}
			
			// 009928
			BitCount ++;
			BitMask <<= 1;
			if (BitMask & 0x100)
				TblIdx ++;
		} while(BitMask & 0x100);
		
		// 009934
		InPos_Data = 0;
		ByteCount = Lookup_00997A[TblIdx][0];
		while(ByteCount)
		{
			if (BitCount == 8)
			{
				if (InPos_Huff >= MaxInSize)
				{
					printf("Input data too small!!\n");
					goto Error;
				}
				
				// 00995E
				BitMask = InData[InPos_Huff];
				InPos_Huff ++;
				BitCount = 0;
			}
			
			// 009954
			BitCount ++;
			BitMask <<= 1;
			InPos_Data <<= 1;
			if (BitMask & 0x100)
				InPos_Data |= 0x01;
			ByteCount --;
		}
		InPos_Data += Lookup_00997A[TblIdx][1];
		OutData[OutPos] = InData[InPos_Base + InPos_Data];
	}
	
	if (RetInSize != NULL)	// this one is optional
		*RetInSize = InPos_Huff;
	*RetOutSize = OutSize;
	*RetOutData = OutData;
	return;

Error:
	printf("Decompression failure at 0x%04X!\n", InPos_Huff);
	*RetOutData = NULL;
	free(OutData);
	
	return;
}
