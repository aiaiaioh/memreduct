# Flush-Network-Caches.ps1
# DNS / ARP / NetBIOS cache flush. Harmless, instant, no reboot.
ipconfig.exe /flushdns
ipconfig.exe /registerdns
arp.exe -d *
nbtstat.exe -R
Write-Host "`nNetwork caches flushed." -ForegroundColor Green
