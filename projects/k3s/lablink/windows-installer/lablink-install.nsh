  ; ---- the service gucefLabLink, set up by install.ps1 of this package ----
  ${IfNot} ${RunningX64}
    MessageBox MB_OK|MB_ICONSTOP "gucefLabLink needs a 64-bit Windows." /SD IDOK
    SetErrorLevel 3
    Abort
  ${EndIf}
  SetRegView 64

  ; /SECRET=<lab secret> on the command line links the machine without approval on the cluster; it goes to the script
  ; in an environment variable, not on the command line of a program
  ${GetParameters} $R0
  ${GetOptions} $R0 "/SECRET=" $R1
  ${If} $R1 != ""
    System::Call 'Kernel32::SetEnvironmentVariable(t "GUCEF_LAB_SECRET", t r11)i'
  ${EndIf}

  ; a setup program that runs again on a machine with the service upgrades it: the settings stay
  StrCpy $R2 ""
  ReadRegStr $R3 HKLM "SYSTEM\CurrentControlSet\Services\gucefLabLink" "ImagePath"
  ${If} $R3 != ""
    StrCpy $R2 " -Upgrade"
  ${EndIf}

  ; the 32-bit setup program starts the 64-bit PowerShell, so that %ProgramFiles% is the same for the script
  DetailPrint "Setting up the service gucefLabLink"
  ${DisableX64FSRedirection}
  nsExec::ExecToLog '"$SYSDIR\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "$INSTDIR\scripts\install.ps1" -Prebuilt "$INSTDIR\bin" -SkipTests$R2'
  Pop $R4
  ${EnableX64FSRedirection}
  System::Call 'Kernel32::SetEnvironmentVariable(t "GUCEF_LAB_SECRET", n)i'

  ${If} $R4 != "0"
    MessageBox MB_OK|MB_ICONSTOP "The service gucefLabLink could not be set up (exit code $R4). The details are in the list of this setup program; the log of the service is %ProgramData%\gucef-lablink\logs\gucefLabLink.log." /SD IDOK
    SetErrorLevel 2
    Abort
  ${EndIf}
  ${If} $R1 != ""
    MessageBox MB_OK|MB_ICONINFORMATION "The service gucefLabLink runs. It finds the cluster by itself and links with the lab secret you gave." /SD IDOK
  ${Else}
    MessageBox MB_OK|MB_ICONINFORMATION "The service gucefLabLink runs. It finds the cluster by itself and waits for approval there: sudo gucefLabLink --approve <name of this PC>.$\r$\n$\r$\nTo link without approval run the setup program with /SECRET=<lab secret>." /SD IDOK
  ${EndIf}
