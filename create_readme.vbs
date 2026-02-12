Set objFSO = CreateObject("Scripting.FileSystemObject")
Set objFile = objFSO.CreateTextFile("e:\STM32\Three-channel controller_v4.0\README.md", True, False)
objFile.WriteLine "# 项目名称"
objFile.WriteLine "三通道切换箱控制系�?"
objFile.Close
WScript.Echo "File created with ANSI encoding"
