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

## 4. `quick_launch.json` example

```json
{
    "buttons_groups": [
        {
            "buttons": [
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/Control Panel.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "create_process": {
                        "command_line": "rundll32.exe shell32.dll,Control_RunDLL firewall.cpl",
                        "environment_block": null,
                        "title": null,
                        "working_directory": null
                    },
                    "display_text": "firewall",
                    "icons": null
                },
                {
                    "collapsed": false,
                    "create_process": {
                        "command_line": "rundll32.exe shell32.dll,Control_RunDLL appwiz.cpl,,3",
                        "environment_block": null,
                        "title": null,
                        "working_directory": null
                    },
                    "display_text": "default programs",
                    "icons": null
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/Resource Monitor.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/Task Manager.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/Run.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/Shows Desktop.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "create_process": {
                        "command_line": "rundll32.exe devmgr.dll DeviceManager_Execute",
                        "environment_block": null,
                        "title": null,
                        "working_directory": null
                    },
                    "display_text": "device manager",
                    "icons": null
                },
                {
                    "collapsed": false,
                    "create_process": {
                        "command_line": "rundll32.exe Shell32.dll,Control_RunDLL Sysdm.cpl,,3",
                        "environment_block": null,
                        "title": null,
                        "working_directory": null
                    },
                    "display_text": "system properties",
                    "icons": null
                },
                {
                    "collapsed": false,
                    "create_process": {
                        "command_line": "rundll32.exe shell32.dll,Control_RunDLL ncpa.cpl",
                        "environment_block": null,
                        "title": null,
                        "working_directory": null
                    },
                    "display_text": "network connections",
                    "icons": null
                },
                {
                    "collapsed": false,
                    "create_process": {
                        "command_line": "rundll32.exe shell32.dll,Control_RunDLL desk.cpl",
                        "environment_block": null,
                        "title": null,
                        "working_directory": null
                    },
                    "display_text": "display settings",
                    "icons": null
                },
                {
                    "collapsed": false,
                    "create_process": {
                        "command_line": "rundll32.exe shell32.dll,Options_RunDLL 1",
                        "environment_block": null,
                        "title": null,
                        "working_directory": null
                    },
                    "display_text": "taskbar settings",
                    "icons": null
                },
                {
                    "collapsed": true,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/Window Switcher.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                }
            ],
            "name": "system"
        },
        {
            "buttons": [
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/File Explorer.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/WinSCP.exe - 快捷方式.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "create_process": {
                        "command_line": "rundll32.exe Shell32.dll,SHHelpShortcuts_RunDLL FontsFolder",
                        "environment_block": null,
                        "title": null,
                        "working_directory": null
                    },
                    "display_text": "fonts folder",
                    "icons": null
                },
                {
                    "collapsed": true,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/Microsoft Edge.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": true,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/Google Chrome.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/Han (jhcarl0814) - Chrome.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                }
            ],
            "name": "browser"
        },
        {
            "buttons": [
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/Windows PowerShell.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/Command Prompt.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/ImplicitAppShortcuts/58dc9c9c4a87b968/Git Bash.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                }
            ],
            "name": "command line"
        },
        {
            "buttons": [
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/Visual Studio 2022.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWNORMAL",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/Visual Studio Code.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/Qt Creator 8.0.1 (Community).lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/Notepad++.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "create_process": {
                        "command_line": "notepad",
                        "environment_block": null,
                        "title": null,
                        "working_directory": null
                    },
                    "display_text": null,
                    "icons": null
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/KDiff3.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "create_process": {
                        "command_line": "mspaint",
                        "environment_block": null,
                        "title": null,
                        "working_directory": null
                    },
                    "display_text": null,
                    "icons": null
                }
            ],
            "name": "editor"
        },
        {
            "buttons": [
                {
                    "collapsed": true,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/desktop.ini",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": true,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": true,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/ImplicitAppShortcuts",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": true,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/ImplicitAppShortcuts/58dc9c9c4a87b968",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": true,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/ImplicitAppShortcuts/69639df789022856",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": true,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": true,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/desktop.ini",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/Laragon.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/Slack.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/Discord.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/Icons Extractor.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/QiPress by Aalap Shah.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/ScreenToGif.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/百度网盘.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Local/Temp",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWDEFAULT",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/ext_taskbar.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWNORMAL",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "C:/Users/jhcar/AppData/Roaming/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/适用于 Linux 的 Windows 子系统.lnk",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWNORMAL",
                        "working_directory": null
                    }
                },
                {
                    "collapsed": false,
                    "display_text": null,
                    "icons": null,
                    "shell_execute": {
                        "file_path": "G:/",
                        "parameters": null,
                        "verb": null,
                        "window_show_state": "SW_SHOWNORMAL",
                        "working_directory": null
                    }
                }
            ],
            "name": "default"
        }
    ],
    "directories": [
        "C:\\Users\\jhcar\\AppData\\Roaming\\Microsoft\\Internet Explorer\\Quick Launch"
    ]
}
```
