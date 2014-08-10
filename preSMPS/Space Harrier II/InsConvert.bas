Attribute VB_Name = "Module1"
Option Explicit

Private Type INS_TBL
    Data(&H0 To &H19) As Byte
End Type

Sub Main()

    Const PATH = "D:\VStudio-Programme\VC2010\SMPSPlay\SMPS_Lib\preSMPS\Space Harrier II\"
    Const RegMapStr = "B0 B4 30 38 34 3C 40 48 44 4C 50 58 54 5C 60 68 64 6C 70 78 74 7C 80 88 84 8C"
    Dim RegMap(&H0 To &HFF) As Byte
    Dim InData() As Byte
    Dim PtrList() As Integer
    Const InsCount As Byte = &HAA / 2
    Dim CurIns As Byte
    Dim CurReg As Byte
    Dim InsTable() As INS_TBL
    
    For CurReg = &H0 To &H7F
        RegMap(CurReg) = &HFF
        RegMap(CurReg Or &H80) = &HFF
    Next CurReg
    For CurReg = 0 To Len(RegMapStr) \ 3
        CurIns = CByte("&H" & Mid$(RegMapStr, 1 + CurReg * 3, 2))
        RegMap(CurIns) = CurReg
    Next CurReg
    
    ReDim InsTable(&H0 To InsCount - 1)
    ReDim PtrList(&H0 To InsCount - 1)
    ReDim InData(&H0 To InsCount * 2 - 1)
    Open PATH & "Z80Bank.bin" For Binary Access Read As #1
        Seek #1, 1 + &H83A
        Get #1, , InData()
        For CurIns = &H0 To InsCount - 1
            PtrList(CurIns) = InData(CurIns * 2 + &H0) Or (InData(CurIns * 2 + &H1) And &H7F) * &H100
        Next CurIns
        
        ReDim InData(&H0 To &H1)
        For CurIns = &H0 To InsCount - 1
            Seek #1, 1 + PtrList(CurIns)
            For CurReg = &H0 To &H19
                InsTable(CurIns).Data(CurReg) = &HAA
            Next CurReg
            Do
                Get #1, , InData()
                If InData(0) = &H83 Then Exit Do
                If InData(0) = &HBC Then InData(0) = &HB4
                CurReg = RegMap(InData(0))
                If CurReg = &HFF Then Stop
                InsTable(CurIns).Data(CurReg) = InData(1)
            Loop
        Next CurIns
    Close #1
    
    Open PATH & "GblInsSet.bin" For Binary Access Write As #2
        Put #2, 1, InsTable()
    Close #2

End Sub


