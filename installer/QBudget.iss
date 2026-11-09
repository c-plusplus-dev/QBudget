
#define AppName "QBudget"
#define AppVersion "0.1.0"

[Setup]
AppId=QBudget
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=QBudget

DefaultDirName={localappdata}\Programs\QBudget
DefaultGroupName=QBudget

PrivilegesRequired=lowest

; QBudget is currently built for Windows x64.
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

OutputDir=..\dist
OutputBaseFilename=QBudget-Setup

UninstallDisplayName=QBudget
UninstallDisplayIcon={app}\QBudget.exe

Compression=lzma2
SolidCompression=yes
WizardStyle=modern

DisableProgramGroupPage=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; \
    Description: "Create a &desktop shortcut"; \
    GroupDescription: "Additional shortcuts:"; \
    Flags: unchecked

[Files]
Source: "..\dist\QBudget\*"; \
    DestDir: "{app}"; \
    Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\QBudget"; \
    Filename: "{app}\QBudget.exe"

Name: "{autodesktop}\QBudget"; \
    Filename: "{app}\QBudget.exe"; \
    Tasks: desktopicon
