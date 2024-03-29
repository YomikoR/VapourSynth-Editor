; https://jrsoftware.org/ishelp/index.php

#define AppName "VapourSynth Editor"
#define ExeName "vsedit"
#define Version "r19-mod-6.5"

[Setup]
AppId={#AppName}
AppName={#AppName}
AppPublisher=YomikoR
AppPublisherURL=https://github.com/YomikoR/VapourSynth-Editor
AppReadmeFile=https://github.com/YomikoR/VapourSynth-Editor/blob/vs-api4/README
AppSupportURL=https://github.com/YomikoR/VapourSynth-Editor/issues
AppUpdatesURL=https://github.com/YomikoR/VapourSynth-Editor/blob/vs-api4/CHANGELOG
AppVerName={#AppName} {#Version}
AppVersion={#Version}
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
ChangesAssociations=yes
Compression=lzma2/max
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
LicenseFile=..\LICENSE
OutputBaseFilename={#AppName}-{#Version}-setup
OutputDir=dist
OutputManifestFile={#AppName}-{#Version}-setup-manifest.txt
PrivilegesRequiredOverridesAllowed=dialog commandline
SetupIconFile=..\resources\{#ExeName}.ico
SolidCompression=yes
VersionInfoVersion=1.0.0
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: ..\build\release-64bit-msvc\vsedit.exe; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\vsedit-job-server.exe; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\vsedit-job-server-watcher.exe; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\vsedit-previewer.exe; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\vsedit.ico; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\vsedit.svg; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\README; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\LICENSE; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\CHANGELOG; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\Qt6Core.dll; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\Qt6Core5Compat.dll; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\Qt6Gui.dll; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\Qt6Multimedia.dll; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\Qt6Network.dll; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\Qt6WebSockets.dll; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\Qt6Widgets.dll; DestDir: {app}; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\imageformats\qwebp.dll; DestDir: {app}\imageformats; Flags: ignoreversion uninsrestartdelete restartreplace
Source: ..\build\release-64bit-msvc\platforms\qwindows.dll; DestDir: {app}\platforms; Flags: ignoreversion uninsrestartdelete restartreplace

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#ExeName}.exe"
Name: "{group}\VapourSynth Jobs Server Watcher"; Filename: "{app}\{#ExeName}-job-server-watcher.exe"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#ExeName}.exe"; Tasks: desktopicon

[Registry]
Root: HKCR; Subkey: ".vpy"; ValueType: string; ValueName: ""; ValueData: "VapourSynthPythonScript"; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKCR; Subkey: "VapourSynthPythonScript"; ValueType: string; ValueName: ""; ValueData: "VapourSynth Python Script"; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKCR; Subkey: "VapourSynthPythonScript\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#ExeName}.exe"" ""%1"""; Flags: uninsdeletevalue uninsdeletekeyifempty

[Run]
Filename: "{app}\{#ExeName}.exe"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
