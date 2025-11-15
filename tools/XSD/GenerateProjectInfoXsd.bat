@echo off

SET BATCHSTARTDIR=%~dp0
SET REPO_ROOT=%BATCHSTARTDIR%\..\..\
SET PROJECTINFO_XML=%REPO_ROOT%\Project.xml
SET PROJECTINFO_XSD=%BATCHSTARTDIR%\Project.xsd

powershell.exe -NoProfile -NoLogo -NonInteractive -ExecutionPolicy Bypass -File "%BATCHSTARTDIR%\Generate-XSD.ps1" -XmlPath "%PROJECTINFO_XML%" -OutputDir "%PROJECTINFO_XSD%"
