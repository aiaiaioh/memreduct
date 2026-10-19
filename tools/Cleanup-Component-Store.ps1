# Cleanup-Component-Store.ps1
# This is what windows-cleaner's "Fix-Windows-Updates.bat" actually does.
# Removes superseded component versions from WinSxS. Safe, repeatable, can take 5-15 min.
# (Does NOT use /ResetBase - that would stop you uninstalling existing updates.)
Dism.exe /Online /Cleanup-Image /AnalyzeComponentStore
Dism.exe /Online /Cleanup-Image /StartComponentCleanup
