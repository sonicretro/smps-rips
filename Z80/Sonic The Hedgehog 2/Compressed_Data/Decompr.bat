set path="D:\Eigene Dateien\Downloads\ROMs\KensSharp"
for %%f in (*.sax) do KensSharp.exe -d sax %%f %%~nf.sm2
pause
