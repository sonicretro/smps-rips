Const FILE_PATH As String = "D:\VStudio-Programme\VC2010\SMPSPlay\SMPS_Lib\preSMPS\Rent A Hero\"

Private Function GetPtr(ByRef Data() As Byte, ByVal Base As Long) As Long

    GetPtr = Base + Data(&H0) * &H100 + Data(&H1) * &H1

End Function

Private Sub Form_Load()

    Const TEMPO_LIST As Long = &HEEFF6
    Const MUS_PTRS As Long = &HEF290
    Const DRV_BASE As Long = &HEF330
    Dim CurSong As Long
    Dim PtrA As Long
    Dim PtrB As Long
    Dim TempArr() As Byte
    Dim FileSize As Long
    Dim FileData() As Byte
    
    ReDim TempArr(0 To 1)
    Open FILE_PATH & "Rent A Hero (J) [!].bin" For Binary Access Read As #1
        For CurSong = &H0 To &H19
            Get #1, 1 + MUS_PTRS + CurSong * 2, TempArr()
            PtrA = GetPtr(TempArr(), DRV_BASE)
            Get #1, , TempArr()
            PtrB = GetPtr(TempArr(), DRV_BASE)
            
            FileSize = PtrB - PtrA
            If FileSize < 0 Then FileSize = &H7FFF
            
            ReDim FileData(&H0 To FileSize - 1)
            Open FILE_PATH & Right$("0" & Hex$(CurSong + 1), 2) & ".bin" For Binary Access Write As #2
                Get #1, 1 + MUS_PTRS + CurSong * 2, TempArr()
                Put #2, , TempArr()
                
                Get #1, 1 + TEMPO_LIST + CurSong, TempArr(0)
                Get #1, 1 + PtrA, FileData()
                
                Put #2, , TempArr(0)
                Put #2, , FileData()
            Close #2
        Next CurSong
    Close #1

End Sub
