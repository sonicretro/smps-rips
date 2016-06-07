Attribute VB_Name = "Module1"
Option Explicit

Sub Main()

    Const PATH = "D:\VStudio-Programme\VC2010\SMPSPlay\SMPS_Lib\preSMPS\Super League\"
    Const DACCnt = &H12
    Dim FileSize() As Long
    Dim FileOfs() As Long
    Dim CurFile As Long
    Dim TempArr() As Byte
    Dim Z80Len As Integer
    Dim Z80Spd As Byte
    Dim DACPath As String
    
    Open PATH & "Super League (W) (May 1989).bin" For Binary Access Read As #1
    Open PATH & "DAC_Voice_W.ini" For Append As #4
        ReDim FileOfs(0 To DACCnt - 1)
        ReDim FileSize(0 To DACCnt - 1)
        
        Seek #1, 1 + &H7818&
        For CurFile = 0 To DACCnt - 1
            FileSize(CurFile) = ReadBE16() - &H3
        Next CurFile
        For CurFile = 0 To DACCnt - 1
            FileOfs(CurFile) = ReadBE32()
        Next CurFile
        
        For CurFile = 0 To DACCnt - 1
            ReDim TempArr(0 To FileSize(CurFile) - 1)
            Seek #1, 1 + FileOfs(CurFile)
            Get #1, , Z80Len
            Get #1, , Z80Spd
            
            DACPath = "DAC_Voice\Voice_" & HexOut(CurFile) & "_W.bin"
            Open PATH & DACPath For Binary Access Write As #2
                Get #1, , TempArr()
                Put #2, 1, TempArr()
            Close #2
            Print #4, "[" & HexOut(CurFile) & "]"
            If FileSize(CurFile) = Z80Len + 1 Then
                ' nothing
            ElseIf FileSize(CurFile) = Z80Len Then
                Print #4, "; Z80 sound length: " & HexOut2(Z80Len) & " (correct)"
            Else
                Print #4, "; Z80 sound length: " & HexOut2(Z80Len) & ", should be " & HexOut2(FileSize(CurFile))
            End If
            Print #4, "Compr = DPCM"
            Print #4, "File = " & DACPath
            Print #4, "Rate = 0x" & HexOut(Z80Spd)
            Print #4, ""
        Next CurFile
    Close #4
    Close #1

End Sub

Private Function ReadBE32() As Long

    Dim TempArr(0 To 3) As Byte
    
    Get #1, , TempArr()
    ReadBE32 = (TempArr(0) * &H1000000) Or (TempArr(1) * &H10000) Or (TempArr(2) * &H100&) Or (TempArr(3) * &H1&)

End Function

Private Function ReadBE16() As Long

    Dim TempArr(0 To 1) As Byte
    
    Get #1, , TempArr()
    ReadBE16 = (TempArr(0) * &H100&) Or (TempArr(1) * &H1&)

End Function

Private Function HexOut(ByVal Val As Byte) As String

    HexOut = Right$("0" & Hex$(Val), 2)

End Function

Private Function HexOut2(ByVal Val As Long) As String

    HexOut2 = Right$("000" & Hex$(Val), 4)

End Function
