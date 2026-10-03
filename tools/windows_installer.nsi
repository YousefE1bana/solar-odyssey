Unicode true
!include "MUI2.nsh"
Name "Solar Odyssey ${VERSION}"
OutFile "${OUTPUT}"
InstallDir "$LOCALAPPDATA\Programs\Solar Odyssey"
RequestExecutionLevel user
SetCompressor /SOLID lzma
Icon "${STAGE}\icon.ico"
UninstallIcon "${STAGE}\icon.ico"
VIProductVersion "${VERSION}.0"
VIAddVersionKey "ProductName" "Solar Odyssey"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "FileDescription" "Solar Odyssey Windows Installer"
VIAddVersionKey "LegalCopyright" "Copyright (c) 2025-2026 Yousef Osama"
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${STAGE}\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"
Section "Solar Odyssey"
 SetShellVarContext current
 SetOutPath "$INSTDIR"
 ; Refresh only the dedicated resources of an existing Solar Odyssey install.
 IfFileExists "$INSTDIR\SolarOdyssey.exe" 0 resources_ready
 RMDir /r "$INSTDIR\Textures"
 RMDir /r "$INSTDIR\shaders"
 RMDir /r "$INSTDIR\assets"
 resources_ready:
 File /r "${STAGE}\*.*"
 WriteUninstaller "$INSTDIR\Uninstall.exe"
 CreateDirectory "$SMPROGRAMS\Solar Odyssey"
 CreateShortCut "$SMPROGRAMS\Solar Odyssey\Solar Odyssey.lnk" "$INSTDIR\SolarOdyssey.exe" "" "$INSTDIR\icon.ico"
 CreateShortCut "$SMPROGRAMS\Solar Odyssey\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
 CreateShortCut "$DESKTOP\Solar Odyssey.lnk" "$INSTDIR\SolarOdyssey.exe" "" "$INSTDIR\icon.ico"
 WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\SolarOdyssey" "DisplayName" "Solar Odyssey"
 WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\SolarOdyssey" "DisplayVersion" "${VERSION}"
 WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\SolarOdyssey" "Publisher" "Yousef Osama"
 WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\SolarOdyssey" "InstallLocation" "$INSTDIR"
 WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\SolarOdyssey" "DisplayIcon" "$INSTDIR\SolarOdyssey.exe"
 WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\SolarOdyssey" "UninstallString" '$\"$INSTDIR\Uninstall.exe$\"'
 WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\SolarOdyssey" "NoModify" 1
 WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\SolarOdyssey" "NoRepair" 1
SectionEnd
Section "Uninstall"
 SetShellVarContext current
 Delete "$DESKTOP\Solar Odyssey.lnk"
 Delete "$SMPROGRAMS\Solar Odyssey\Solar Odyssey.lnk"
 Delete "$SMPROGRAMS\Solar Odyssey\Uninstall.lnk"
 RMDir "$SMPROGRAMS\Solar Odyssey"
 DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\SolarOdyssey"
 ; Owned runtime directories only. Player state lives outside the installation.
 RMDir /r "$INSTDIR\Textures"
 RMDir /r "$INSTDIR\shaders"
 RMDir /r "$INSTDIR\assets"
 !include "${OWNED_DLLS}"
 Delete "$INSTDIR\SolarOdyssey.exe"
 Delete "$INSTDIR\icon.png"
 Delete "$INSTDIR\icon.ico"
 Delete "$INSTDIR\LICENSE"
 Delete "$INSTDIR\THIRD_PARTY_NOTICES.md"
 Delete "$INSTDIR\PLAYER_GUIDE.md"
 Delete "$INSTDIR\Uninstall.exe"
 RMDir "$INSTDIR"
SectionEnd
