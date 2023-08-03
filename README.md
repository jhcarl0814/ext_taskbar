# ext_taskbar
Restore Windows 11 taskbar's lost functionalities when switching it to multi-row & never-combining mode!

## 1. Prepare to use multi-row taskbar on Windows 11

### 1.1 Settings >> Personalization >> Themes >> Desktop icon settings

Make sure Control Panel icon is appearing on your desktop.

### 1.2 `to_win10.ps1` (need "run as administrator" to work) (switch to multi-row & never-combining taskbar, but `file explorer` can not be started by double-clicking folder icons on desktop and `start menu` is lost)

```pwsh
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope LocalMachine -Force
# Restore the Classic Taskbar in Windows 11
New-ItemProperty -Path "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Shell\Update\Packages" -Name "UndockingDisabled" -PropertyType DWord -Value "00000001";
Set-ItemProperty -Path "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Shell\Update\Packages" -Name "UndockingDisabled" -Value "00000001";
# Disable Taskbar / Cortana Search Box on Windows 11
New-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Search" -Name "SearchboxTaskbarMode" -PropertyType DWord -Value "00000000";
Set-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Search" -Name "SearchboxTaskbarMode" -Value "00000000";
# Ungroup Taskbar Icons / Enable Text Labels in Windows 11
New-ItemProperty -Path "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\Explorer" -Name "NoTaskGrouping" -PropertyType DWord -Value "00000001";
Set-ItemProperty -Path "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\Explorer" -Name "NoTaskGrouping" -Value "00000001";
#Restart Explorer to see the changes
./taskkill /f /im explorer.exe;
./CMD /Q /C START /REALTIME explorer.exe;
```

### 1.3 `to_win11.ps1` (need "run as administrator" to work) (switch to single row & always-combining taskbar, but `file explorer` can be started by double-clicking folder icons on desktop and `start menu` is back)

```pwsh
#Run this to reverse the changes
Set-ItemProperty -Path "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Shell\Update\Packages" -Name "UndockingDisabled" -Value "00000000"
Set-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Search" -Name "SearchboxTaskbarMode" -Value "00000001";
Set-ItemProperty -Path "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\Explorer" -Name "NoTaskGrouping" -Value "00000000";
#Restart Explorer to see the changes
./taskkill /f /im explorer.exe;
./CMD /Q /C START /REALTIME explorer.exe;
```

## 2. `ext_taskbar`

### 2.1 Use Qt Creator to compile

### 2.2 Installation: copy ext_taskbar.exe to a folder, open `Qt *.*.* Command prompt` shortcut in Qt's start menu, `cd the_folder`, `windeployqt ext_taskbar.exe` to pull `***.dll`s in

### 2.3 Open ext_taskbar.exe, right-click taskbar item of ext_taskbar.exe >> pin to taskbar

## 3. While using multi-row taskbar on Windows 11

### 3.1 Use Control Panel icon on desktop to start `file explorer` (it seems this is the only way to open a `file explorer` window)

### 3.2 Use `ext_taskbar` to bypass that troublesome process if you want to start some program from inside `start menu` or inside folders on `desktop`
