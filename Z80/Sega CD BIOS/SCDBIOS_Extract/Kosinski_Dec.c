/*-----------------------------------------------------------------------------*\
|																				|
|	Kosinski.dll: Compression / Decompression of data in Kosinski format		|
|	Copyright © 2002-2004 The KENS Project Development Team						|
|																				|
|	This library is free software; you can redistribute it and/or				|
|	modify it under the terms of the GNU Lesser General Public					|
|	License as published by the Free Software Foundation; either				|
|	version 2.1 of the License, or (at your option) any later version.			|
|																				|
|	This library is distributed in the hope that it will be useful,				|
|	but WITHOUT ANY WARRANTY; without even the implied warranty of				|
|	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU			|
|	Lesser General Public License for more details.								|
|																				|
|	You should have received a copy of the GNU Lesser General Public			|
|	License along with this library; if not, write to the Free Software			|
|	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA	|
|																				|
\*-----------------------------------------------------------------------------*/

// Note: Modified by Valley Bell to work with Memory Buffers instead of files.
#include <stdio.h>
#include <malloc.h>

typedef unsigned char	UINT8;
typedef signed char 	INT8;
typedef unsigned short	UINT16;
typedef signed short	INT16;
typedef unsigned int	UINT32;
typedef signed int		INT32;
typedef unsigned char	bool;

#define SUCCESS			0x00
#define ERROR_UNKNOWN	0x01

//-----------------------------------------------------------------------------------------------
// Name: KDecomp(UINT32 SrcSize, const UINT8* SrcData, UINT32* RetDstSize, UINT8** RetDstData, bool Moduled)
// Desc: Decompresses the data using the Kosinski compression format
//-----------------------------------------------------------------------------------------------
UINT8 KDecomp(UINT32 SrcSize, const UINT8* SrcData, UINT32* RetDstSize, UINT8** RetDstData, bool Moduled)
{
// Buffers and Buffer Infos
	UINT32 DstSize;
	UINT8* DstData;
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
	UINT32 Offset;
	UINT32 i;
	
// Moduled infos
	UINT32 FullSize = 0;
	
//------------------------------------------------------------------------------------------------
	
	if (RetDstSize == NULL || RetDstData == NULL)
		return ERROR_UNKNOWN;
	
	if (! *RetDstSize || *RetDstData == NULL)
	{
		if (*RetDstData != NULL)
			free(*RetDstData);
		DstSize = 0x10000;	// 64 KB should be more than enough
		DstData = (UINT8*)malloc(DstSize);
	}
	else
	{
		DstSize = *RetDstSize;
		DstData = *RetDstData;
	}
	SrcPos = 0x00;
	DstPos = 0x00;
	if (Moduled)
	{
		FullSize = (SrcData[SrcPos + 0] << 8) | SrcData[SrcPos + 1];
		SrcPos += 0x02;
	}
	
start:
	memcpy(&BITFIELD, &SrcData[SrcPos], 2);
	BITFIELD = (SrcData[SrcPos + 1] << 8) | SrcData[SrcPos + 0];
	SrcPos += 2;
	BFP = 0;
	
//------------------------------------------------------------------------------------------------
	while(SrcPos < SrcSize && DstPos < DstSize)
	{
		if (BITFIELD & (1 << BFP++))
			Bit = 1;
		else
			Bit = 0;
		if (BFP >= 16)
		{
			memcpy(&BITFIELD, &SrcData[SrcPos], 2);
			BITFIELD = (SrcData[SrcPos + 1] << 8) | SrcData[SrcPos + 0];
			SrcPos += 2;
			BFP = 0;
		}
//-- Direct Copy ---------------------------------------------------------------------------------
		if (Bit)
		{
			DstData[DstPos] = SrcData[SrcPos];
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
				memcpy(&BITFIELD, &SrcData[SrcPos], 2);
				BITFIELD = (SrcData[SrcPos + 1] << 8) | SrcData[SrcPos + 0];
				SrcPos += 2;
				BFP = 0;
			}
//-- Embedded / Separate -------------------------------------------------------------------------
			if (Bit)
			{
				Low = SrcData[SrcPos ++];
				High = SrcData[SrcPos ++];
				
				Count = High & 0x07;
				
				if (Count == 0)
				{
					Count = SrcData[SrcPos ++];
					if (Count == 0)
						goto end;
					if (Count == 1)
						continue;
				}
				else
				{
					Count ++;
				}
				
				Offset = 0xFFFFE000 | ((0xF8 & High) << 5) | Low;
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
					memcpy(&BITFIELD, &SrcData[SrcPos], 2);
					BITFIELD = (SrcData[SrcPos + 1] << 8) | SrcData[SrcPos + 0];
					SrcPos += 2;
					BFP = 0;
				}
				if (BITFIELD & (1 << BFP++))
					High = 1;
				else
					High = 0;
				if (BFP >= 16)
				{
					memcpy(&BITFIELD, &SrcData[SrcPos], 2);
					BITFIELD = (SrcData[SrcPos + 1] << 8) | SrcData[SrcPos + 0];
					SrcPos += 2;
					BFP = 0;
				}
				
				Count = Low * 2 + High + 1;
				
				Offset = 0xFFFFFF00 | SrcData[SrcPos ++];
			}
//-- Write to file for indirect copy -------------------------------------------------------------
			for (i = 0; i <= Count; i ++)
			{
				DstData[DstPos] = DstData[DstPos + Offset];
				DstPos ++;
			}
//------------------------------------------------------------------------------------------------
		}
	}
//------------------------------------------------------------------------------------------------
	
end:
	if (Moduled)
	{
		if (DstPos < FullSize)
		{
			do
			{
				Low = SrcData[SrcPos];
				SrcPos ++;
			} while(Low == 0);
			SrcPos --;
			goto start;
		}
	}
	DstSize = DstPos;
	DstData = (UINT8*)realloc(DstData, DstSize);	// free some memory
	
	*RetDstSize = DstSize;
	*RetDstData = DstData;
	return SUCCESS;
}
