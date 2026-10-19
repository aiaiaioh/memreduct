# Clean-Temp-Files.ps1
# Adapted from markulie/windows-cleaner (MIT). Changes from the original:
#   - does NOT wipe C:\Windows\Prefetch (that slows boot/app launch until Windows rebuilds it)
#   - Recycle Bin is only emptied if you pass -IncludeRecycleBin
#   - reports how much was actually freed
param([switch]$IncludeRecycleBin)

$targets = @(
    "$env:LOCALAPPDATA\Temp",
    "$env:SystemRoot\Temp",
    "$env:LOCALAPPDATA\Microsoft\Windows\INetCache",
    "$env:LOCALAPPDATA\Microsoft\Windows\Explorer\thumbcache_*.db"
)

$before = (Get-PSDrive C).Free
foreach ($t in $targets) {
    if (Test-Path $t) {
        Write-Host "Cleaning $t"
        Get-ChildItem -Path $t -Force -ErrorAction SilentlyContinue |
            Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
    }
}
if ($IncludeRecycleBin) { Clear-RecycleBin -Force -ErrorAction SilentlyContinue }

$freed = ((Get-PSDrive C).Free - $before) / 1MB
Write-Host ("`nFreed {0:N0} MB (files in use were skipped, which is normal)." -f [math]::Max($freed, 0)) -ForegroundColor Green
