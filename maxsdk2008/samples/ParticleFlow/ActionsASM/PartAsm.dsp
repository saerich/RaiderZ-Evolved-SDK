# Microsoft Developer Studio Project File - Name="PartAsm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PartAsm - Win32 Hybrid
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PartAsm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PartAsm.mak" CFG="PartAsm - Win32 Hybrid"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PartAsm - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PartAsm - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PartAsm - Win32 Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PartAsm - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PARTASM_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /O2 /I "..\..\maxsdk\include" /I "..\..\maxsdk\include\ParticleFlow" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PARTASM_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /dll /machine:I386 /out:"c:\3dsmax5\Plugins\ParticleAssembler.dlo" /libpath:"..\..\maxsdk\lib"

!ELSEIF  "$(CFG)" == "PartAsm - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PARTASM_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\maxsdk\include" /I "..\..\maxsdk\include\ParticleFlow" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PARTASM_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /dll /debug /machine:I386 /out:"c:\3dsmax5.1\StdPlugs\ParticleAssembler.dll" /pdbtype:sept /libpath:"..\..\maxsdk\lib"

!ELSEIF  "$(CFG)" == "PartAsm - Win32 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PartAsm___Win32_Hybrid"
# PROP BASE Intermediate_Dir "PartAsm___Win32_Hybrid"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PartAsm___Win32_Hybrid"
# PROP Intermediate_Dir "PartAsm___Win32_Hybrid"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\maxsdk\include" /I "..\..\maxsdk\include\ParticleFlow" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PARTASM_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "..\..\maxsdk\include" /I "..\..\maxsdk\include\ParticleFlow" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PARTASM_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /dll /debug /machine:I386 /out:"c:\3dsmax5.1\StdPlugs\PartAsm.dll" /pdbtype:sept /libpath:"..\..\maxsdk\lib"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /dll /debug /machine:I386 /out:"c:\3dsmax5\Plugins\ParticleAssembler.dlo" /pdbtype:sept /libpath:"..\..\maxsdk\lib"

!ENDIF 

# Begin Target

# Name "PartAsm - Win32 Release"
# Name "PartAsm - Win32 Debug"
# Name "PartAsm - Win32 Hybrid"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\PartAsm.cpp
# End Source File
# Begin Source File

SOURCE=.\PartAsm.def
# End Source File
# Begin Source File

SOURCE=.\PartAsm.rc
# End Source File
# Begin Source File

SOURCE=.\PartAsm_GlobalFunctions.cpp
# End Source File
# Begin Source File

SOURCE=.\PartAsm_GlobalVariables.cpp
# End Source File
# Begin Source File

SOURCE=.\PartAsm_SysUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\PartAsm_GlobalFunctions.h
# End Source File
# Begin Source File

SOURCE=.\PartAsm_GlobalVariables.h
# End Source File
# Begin Source File

SOURCE=.\PartAsm_SysUtil.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ASMOperator_ActiveIcon.bmp
# End Source File
# Begin Source File

SOURCE=.\ASMOperator_DepotIcon.bmp
# End Source File
# Begin Source File

SOURCE=.\ASMOperator_InactiveIcon.bmp
# End Source File
# Begin Source File

SOURCE=.\Display_ActiveIcon.bmp
# End Source File
# Begin Source File

SOURCE=.\Display_DepotIcon.bmp
# End Source File
# Begin Source File

SOURCE=.\Display_InactiveIcon.bmp
# End Source File
# Begin Source File

SOURCE=.\Operator_DepotIconMask.bmp
# End Source File
# Begin Source File

SOURCE=.\Test_ActiveIcon.bmp
# End Source File
# Begin Source File

SOURCE=.\Test_DepotIcon.bmp
# End Source File
# Begin Source File

SOURCE=.\Test_DepotIconMask.bmp
# End Source File
# Begin Source File

SOURCE=.\Test_FalseIcon.bmp
# End Source File
# Begin Source File

SOURCE=.\Test_TrueIcon.bmp
# End Source File
# End Group
# Begin Group "Channels"

# PROP Default_Filter "*.h *.cpp"
# Begin Source File

SOURCE=.\IParticleChannelMult.h
# End Source File
# Begin Source File

SOURCE=.\ParticleChannelDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleChannelDesc.h
# End Source File
# Begin Source File

SOURCE=.\ParticleChannelMult.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleChannelMult.h
# End Source File
# End Group
# Begin Group "Operators"

# PROP Default_Filter "*.h *.cpp"
# Begin Group "Display"

# PROP Default_Filter "*.h *.cpp"
# Begin Source File

SOURCE=.\PAOperatorDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorDisplay.h
# End Source File
# Begin Source File

SOURCE=.\PAOperatorDisplay_ParamBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorDisplay_ParamBlock.h
# End Source File
# Begin Source File

SOURCE=.\PAOperatorDisplayDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorDisplayDesc.h
# End Source File
# End Group
# Begin Group "Value"

# PROP Default_Filter "*.h *.cpp"
# Begin Source File

SOURCE=.\PAOperatorValue.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorValue.h
# End Source File
# Begin Source File

SOURCE=.\PAOperatorValue_ParamBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorValue_ParamBlock.h
# End Source File
# Begin Source File

SOURCE=.\PAOperatorValueDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorValueDesc.h
# End Source File
# End Group
# Begin Group "Copy"

# PROP Default_Filter "*.h *.cpp"
# Begin Source File

SOURCE=.\PAOperatorCopy.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorCopy.h
# End Source File
# Begin Source File

SOURCE=.\PAOperatorCopy_ParamBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorCopy_ParamBlock.h
# End Source File
# Begin Source File

SOURCE=.\PAOperatorCopyDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorCopyDesc.h
# End Source File
# End Group
# Begin Group "Function"

# PROP Default_Filter "*.h *.cpp"
# Begin Source File

SOURCE=.\PAOperatorFunction.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorFunction.h
# End Source File
# Begin Source File

SOURCE=.\PAOperatorFunction_ParamBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorFunction_ParamBlock.h
# End Source File
# Begin Source File

SOURCE=.\PAOperatorFunctionDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorFunctionDesc.h
# End Source File
# End Group
# Begin Group "Random"

# PROP Default_Filter "*.h *.cpp"
# Begin Source File

SOURCE=.\PAOperatorRandom.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorRandom.h
# End Source File
# Begin Source File

SOURCE=.\PAOperatorRandom_ParamBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorRandom_ParamBlock.h
# End Source File
# Begin Source File

SOURCE=.\PAOperatorRandomDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorRandomDesc.h
# End Source File
# End Group
# Begin Group "Condition"

# PROP Default_Filter "*.h *.cpp"
# Begin Source File

SOURCE=.\PAOperatorCondition.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorCondition.h
# End Source File
# Begin Source File

SOURCE=.\PAOperatorCondition_ParamBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorCondition_ParamBlock.h
# End Source File
# Begin Source File

SOURCE=.\PAOperatorConditionDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorConditionDesc.h
# End Source File
# End Group
# Begin Group "Object"

# PROP Default_Filter "*.h *.cpp"
# Begin Source File

SOURCE=.\PAOperatorObject.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorObject.h
# End Source File
# Begin Source File

SOURCE=.\PAOperatorObject_ParamBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorObject_ParamBlock.h
# End Source File
# Begin Source File

SOURCE=.\PAOperatorObjectDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\PAOperatorObjectDesc.h
# End Source File
# End Group
# End Group
# Begin Group "Tests"

# PROP Default_Filter "*.h *.cpp"
# Begin Group "Test"

# PROP Default_Filter "*.h *.cpp"
# Begin Source File

SOURCE=.\PATest.cpp
# End Source File
# Begin Source File

SOURCE=.\PATest.h
# End Source File
# Begin Source File

SOURCE=.\PATest_ParamBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\PATest_ParamBlock.h
# End Source File
# Begin Source File

SOURCE=.\PATestDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\PATestDesc.h
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=..\..\maxsdk\lib\mesh.lib
# End Source File
# Begin Source File

SOURCE=..\..\maxsdk\lib\geom.lib
# End Source File
# Begin Source File

SOURCE=..\..\maxsdk\lib\gfx.lib
# End Source File
# Begin Source File

SOURCE=..\..\maxsdk\lib\maxutil.lib
# End Source File
# Begin Source File

SOURCE=..\..\maxsdk\lib\Paramblk2.lib
# End Source File
# Begin Source File

SOURCE=..\..\maxsdk\lib\ParticleFlow.lib
# End Source File
# Begin Source File

SOURCE=..\..\maxsdk\lib\core.lib
# End Source File
# End Target
# End Project
