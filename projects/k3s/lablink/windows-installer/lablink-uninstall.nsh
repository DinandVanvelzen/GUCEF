  ; ---- remove the service gucefLabLink with uninstall.ps1 of this package; the lab secret and the identity of the machine stay ----
  SetRegView 64
  DetailPrint "Removing the service gucefLabLink"
  ${DisableX64FSRedirection}
  nsExec::ExecToLog '"$SYSDIR\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "$INSTDIR\scripts\uninstall.ps1"'
  Pop $R0
  ${EnableX64FSRedirection}
  ${If} $R0 != "0"
    MessageBox MB_OK|MB_ICONEXCLAMATION "The service gucefLabLink could not be removed completely (exit code $R0), see the list of this program. Remove it with uninstall.ps1 -Purge from an elevated PowerShell." /SD IDOK
  ${EndIf}
