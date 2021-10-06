; https://jrsoftware.org/ishelp/index.php

#define AppName "VapourSynth Editor"
#define ExeName "vsedit"
#define Version "r19-mod-3"

[Setup]
AppId={#AppName}
AppName={#AppName}
AppPublisher=YomikoR
AppPublisherURL=https://github.com/YomikoR/VapourSynth-Editor
AppReadmeFile=https://github.com/YomikoR/VapourSynth-Editor/blob/master/README
AppSupportURL=https://github.com/YomikoR/VapourSynth-Editor/issues
AppUpdatesURL=https://github.com/YomikoR/VapourSynth-Editor/blob/master/CHANGELOG
AppVerName={#AppName} {#Version}
AppVersion={#Version}
Compression=lzma2/max
DefaultDirName={autopf}\{#AppName}
LicenseFile=LICENSE
; Support disabled for <= Windows 7/Server 2008 R2 SP1. No documented reason, assumed.
MinVersion=6.2
OutputBaseFilename={#AppName}-{#Version}-setup
OutputDir=dist
OutputManifestFile={#AppName}-{#Version}-setup-manifest.txt
PrivilegesRequiredOverridesAllowed=dialog commandline
SetupIconFile={#ExeName}.ico
SolidCompression=yes
VersionInfoVersion=1.0.0
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: *; DestDir: {app}; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#ExeName}.exe"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#ExeName}.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\{#ExeName}.exe"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
