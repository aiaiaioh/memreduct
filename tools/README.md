# tools/

Anything you drop in here (`.ps1`, `.bat`, `.cmd`, `.reg`) shows up in Mem Reduct
under **Tools** (menu bar) and in the tray right-click menu. The folder is re-scanned
every time the menu opens, so no restart needed.

* `.ps1` -> `powershell.exe -NoProfile -ExecutionPolicy Bypass`, window stays open until you press a key
* `.bat`/`.cmd` -> `cmd.exe /c call ... & pause`
* `.reg` -> `regedit.exe` **without** `/s`, so you still get the "are you sure" prompt
* Scripts inherit Mem Reduct's elevation. If Mem Reduct isn't elevated you get a UAC prompt.
* A confirmation dialog appears before every run; tick "don't ask again" to skip it
  (stored as `IsShowToolsConfirmation` in memreduct.ini, like the other confirmations).

## What's here and what was deliberately left out of windows-cleaner

Kept (reworked - see each file's header):

| Script | Source in windows-cleaner | Why it's suitable for a tray menu |
|---|---|---|
| Clean-Temp-Files.ps1 | Remove-Temp-Files.ps1 | Repeatable, reversible-ish. Prefetch wipe removed. |
| Cleanup-Component-Store.ps1 | Fix-Windows-Updates.bat | Repeatable DISM cleanup, safe. |
| Repair-System-Files.ps1 | Scan-Protected-System-Files.ps1 | Repeatable, read-mostly. |
| Reset-Windows-Update-Cache.ps1 | (none - added) | The correct fix for stuck updates. |
| Flush-Network-Caches.ps1 | (none - added) | Harmless, instant. |

Left out, on purpose:

* **Check-File-System.ps1** - runs `chkdsk c: /f /r /x`. `/x` force-dismounts C:, which can't
  happen while Windows is running, so it schedules a full surface scan at next boot. On a 4 TB
  QVO that's several hours. Not a thing you want one accidental click away.
* **Lower-RAM-Usage.reg** - despite the name it's a grab-bag: disables Fast Startup/hibernation,
  turns off Power Throttling (hurts laptop battery), rewrites MMCSS scheduling, and sets
  `DisableAntiSpyware=1` on Defender. Apply individual keys deliberately, not as a bundle.
* **Disable-Services*.ps1, Disable-Scheduled-Tasks.ps1, Remove-Default-Apps.ps1, Remove-Edge.bat,
  Disable-*-Telemetry.ps1, Fix-Privacy-Settings.ps1** - one-shot debloat/privacy changes. Run them once
  from a terminal where you can read the output and know what changed; a tray menu is the wrong UI
  for operations you shouldn't repeat and may want to undo.
* The `.lnk` files and `Startup-*.bat` - just shutdown/restart shortcuts.
