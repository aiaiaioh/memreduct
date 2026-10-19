# Reset-Windows-Update-Cache.ps1
# The *safe* Windows Update reset: clears the download cache and catalogue so a
# stuck update can start over. It stops/starts services; it never deletes service
# registrations or touches the registry - the thing debloat scripts get wrong.
$services = 'wuauserv', 'bits', 'cryptsvc', 'msiserver'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'

foreach ($s in $services) { Stop-Service $s -Force -ErrorAction SilentlyContinue }

foreach ($dir in "$env:SystemRoot\SoftwareDistribution", "$env:SystemRoot\System32\catroot2") {
    if (Test-Path $dir) { Rename-Item $dir "$dir.$stamp.bak" -ErrorAction SilentlyContinue }
}

foreach ($s in $services) { Start-Service $s -ErrorAction SilentlyContinue }

Write-Host "`nDone. Old caches kept as *.$stamp.bak - delete them once updates work again." -ForegroundColor Green
