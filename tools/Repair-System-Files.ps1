# Repair-System-Files.ps1
# From windows-cleaner's Scan-Protected-System-Files.ps1, with DISM first so SFC
# has a healthy component store to repair from. Order matters.
# Expect 10-30 minutes. Reboot afterwards if either reports repairs.
Dism.exe /Online /Cleanup-Image /RestoreHealth
sfc.exe /scannow
