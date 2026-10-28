# servo-cal-keys.ps1 - arrow-key front end for the servo calibration console (servo-calibration.ino)
#
# The console on the board does the real work and keeps its safety rules (no startup motion, test window,
# max 10 us per pulse command). This script only sends console commands: an arrow key = one or more
# "pulse" commands in 10 us steps, Enter = "record" / "reference". It saves the results as a table.
#
# Wiring (Uno / Nano): servo signal -> D9, red -> 5V (SG92R / SG90 only; MG996R needs an external supply),
# brown -> GND. One servo at a time.
#
# Run (Windows PowerShell):
#   powershell -ExecutionPolicy Bypass -File servo-cal-keys.ps1 -Port COM7
#
# Results (saved after every servo, so you can quit and come back):
#   results\servo-cal-results.csv   one line per servo (reloaded on the next run)
#   results\servo-cal-table.txt     the table
#   results\console-<name>.csv      the console's own "table" output for that servo
param(
  [string]$Port = "COM7",
  [int]$Slot = 4,               # console unit slot to use (1..6); only the label printed by the console
  [string]$OutDir = (Join-Path $PSScriptRoot "results")
)
$ErrorActionPreference = "Stop"

$US = [string][char]0x00B5 + "s"; $DEG = [string][char]0x00B0; $ARROW = [string][char]0x2192; $DASH = [string][char]0x2013
$MIN_US = 500; $MAX_US = 2500; $STEP_MAX = 10
New-Item -ItemType Directory -Force $OutDir | Out-Null
$csvPath = Join-Path $OutDir "servo-cal-results.csv"
$txtPath = Join-Path $OutDir "servo-cal-table.txt"
$results = New-Object System.Collections.Generic.List[object]
if (Test-Path $csvPath) { foreach ($r in (Import-Csv $csvPath)) { $results.Add($r) } }

# ------------------------------------------------------------------ table
function Write-Table {
  $headers = [string[]]@("Servo", "Reference pulse", "Measured angle-pulse pairs", "Safe pulse range",
                         "Installed joint-zero pulse")
  $rows = New-Object System.Collections.Generic.List[object]
  $rows.Add($headers)
  foreach ($r in $results) {
    $rows.Add([string[]]@($r.Servo, "$($r.Us90) $US at measured 90$DEG",
      "0$DEG $ARROW $($r.Us0) $US; 90$DEG $ARROW $($r.Us90) $US; 180$DEG $ARROW $($r.Us180) $US",
      "$($r.SafeMin)$DASH$($r.SafeMax) $US", "Measure after installation"))
  }
  $widths = New-Object int[] $headers.Length
  foreach ($row in $rows) { for ($i = 0; $i -lt $row.Length; $i++) { $widths[$i] = [Math]::Max($widths[$i], $row[$i].Length) } }
  $sep = "+"; foreach ($w in $widths) { $sep += ("-" * ($w + 2)) + "+" }
  $lines = New-Object System.Collections.Generic.List[string]
  $lines.Add("Servo calibration - " + (Get-Date -Format "yyyy-MM-dd HH:mm")); $lines.Add(""); $lines.Add($sep)
  $first = $true
  foreach ($row in $rows) {
    $line = "|"; for ($i = 0; $i -lt $row.Length; $i++) { $line += " " + $row[$i].PadRight($widths[$i]) + " |" }
    $lines.Add($line)
    if ($first) { $lines.Add($sep.Replace("-", "=")); $first = $false }
  }
  $lines.Add($sep)
  $lines | Out-File -FilePath $txtPath -Encoding utf8
  $results | Export-Csv -Path $csvPath -NoTypeInformation -Encoding UTF8
  return $lines
}

# ------------------------------------------------------------------ console link
$script:ser = $null
$script:cur = 1500

function Read-Reply([int]$ms = 400) {
  # read until the console has been quiet for a moment (every command answers with at least one line)
  $text = ""; $deadline = (Get-Date).AddMilliseconds($ms); $quietSince = $null
  while ((Get-Date) -lt $deadline) {
    $chunk = $script:ser.ReadExisting()
    if ($chunk) { $text += $chunk; $quietSince = Get-Date }
    elseif ($quietSince -and ((Get-Date) - $quietSince).TotalMilliseconds -gt 40) { break }
    Start-Sleep -Milliseconds 5
  }
  return $text
}

function Send([string]$cmd, [int]$ms = 400) {
  $script:ser.Write("$cmd`n")
  $reply = Read-Reply $ms
  if ($reply -match "Servo calibration console") {
    Write-Host ""
    Write-Host "  !! The board reset (power dip?). Its RAM records are gone; this script re-arms the servo." -ForegroundColor Red
  }
  if ($reply -match "ERR") { Write-Host ""; Write-Host ("  console: " + ($reply.Trim() -split "`n")[-1]) -ForegroundColor Yellow }
  return $reply
}

function Setup-Console {
  [void](Send "select $Slot")
  [void](Send "window $MIN_US $MAX_US")
}

# move to a pulse in console-legal steps (<= 10 us each), slowly enough to watch
function Move-To([int]$us) {
  $us = [Math]::Max($MIN_US, [Math]::Min($MAX_US, $us))
  while ($script:cur -ne $us) {
    $step = [Math]::Max(-$STEP_MAX, [Math]::Min($STEP_MAX, $us - $script:cur))
    $r = Send "pulse $($script:cur + $step)" 200
    if ($r -match "ERR") { return $false }
    if ($r -match "Servo calibration console") { Setup-Console; [void](Send "arm $($script:cur)") ; continue }
    $script:cur += $step
  }
  return $true
}

# ------------------------------------------------------------------ keys
function Read-Point([string]$title, [string]$hint) {
  Write-Host ""; Write-Host "  $title" -ForegroundColor Cyan; Write-Host "  $hint"
  while ($true) {
    Write-Host -NoNewline ("`r  pulse: {0,4} us   " -f $script:cur)
    $k = [Console]::ReadKey($true)
    $delta = 0
    switch ($k.Key) {
      "RightArrow" { $delta = 10 }; "LeftArrow" { $delta = -10 }
      "UpArrow" { $delta = 50 }; "DownArrow" { $delta = -50 }
      "Enter" { Write-Host ""; return $script:cur }
    }
    switch -CaseSensitive ($k.KeyChar) {
      "+" { $delta = 1 }; "-" { $delta = -1 }
      "s" { Write-Host ""; return "skip" }; "q" { Write-Host ""; return "quit" }
    }
    if ($delta -ne 0) { [void](Move-To ($script:cur + $delta)) }
  }
}

# ------------------------------------------------------------------ main
Write-Host "Servo calibration (console on $Port, slot $Slot). One servo at a time on D9." -ForegroundColor Green
Write-Host "Keys: Left/Right -/+10 us   Down/Up -/+50 us   -/+ -/+1 us   Enter = record   s = skip   q = quit"
Write-Host "Saving to $txtPath"
$script:ser = New-Object System.IO.Ports.SerialPort $Port, 115200, None, 8, One
$script:ser.WriteTimeout = 1000
$script:ser.Open()
$script:ser.DtrEnable = $true; Start-Sleep -Milliseconds 200; $script:ser.DtrEnable = $false  # clean reset
Start-Sleep -Milliseconds 2500
$banner = $script:ser.ReadExisting()
if ($banner -notmatch "Servo calibration console") {
  Write-Host "No calibration console on $Port. Upload code/servo-calibration first." -ForegroundColor Red
  $script:ser.Close(); exit 1
}
try {
  while ($true) {
    Write-Host ""
    $name = Read-Host "Name of the servo, e.g. SG92R-4 (Enter = quit)"
    if ([string]::IsNullOrWhiteSpace($name) -or $name -eq "q") { break }
    Setup-Console
    Write-Host "  Arming at 1500 us (the servo jumps to the middle)."
    [void](Send "arm 1500"); $script:cur = 1500

    $us90 = Read-Point "Step 1/5 - 90 deg" "Seat the horn pointing LEFT, fine-tune with Left/Right, then Enter."
    if ($us90 -is [string]) { if ($us90 -eq "quit") { break } else { [void](Send "off"); continue } }
    [void](Send "record 90"); [void](Send "reference")

    [void](Move-To ($us90 - 700))
    $us0 = Read-Point "Step 2/5 - 0 deg" "Down/Up to get close, Left/Right until EXACTLY up, then Enter."
    if ($us0 -is [string]) { if ($us0 -eq "quit") { break } else { [void](Send "off"); continue } }
    [void](Send "record 0")

    [void](Move-To ($us90 + 700))
    $us180 = Read-Point "Step 3/5 - 180 deg" "Up/Down to get close, Left/Right until EXACTLY down, then Enter."
    if ($us180 -is [string]) { if ($us180 -eq "quit") { break } else { [void](Send "off"); continue } }
    [void](Send "record 180")

    [void](Move-To ([Math]::Min($us0, $us180)))
    $lo = Read-Point "Step 4/5 - safe LOW limit" "Tap Left slowly; when it stops turning or buzzes, back off until quiet, then Enter."
    if ($lo -is [string]) { if ($lo -eq "quit") { break } else { [void](Send "off"); continue } }
    [void](Move-To ([Math]::Max($us0, $us180)))
    $hi = Read-Point "Step 5/5 - safe HIGH limit" "Tap Right slowly; when it stops turning or buzzes, back off until quiet, then Enter."
    if ($hi -is [string]) { if ($hi -eq "quit") { break } else { [void](Send "off"); continue } }

    [void](Move-To $us90)
    [void](Send "off")
    [void](Send "limits $([Math]::Min($lo, $us90)) $([Math]::Max($hi, $us90))")
    $tbl = Send "table" 800
    $tbl | Out-File -FilePath (Join-Path $OutDir ("console-" + ($name -replace '[^A-Za-z0-9_-]', '_') + ".csv")) -Encoding utf8

    $results.Add([pscustomobject]@{ Servo = $name; Us0 = $us0; Us90 = $us90; Us180 = $us180; SafeMin = $lo; SafeMax = $hi })
    $null = Write-Table
    Write-Host ("  check: 0->90 = {0} us, 90->180 = {1} us, total = {2} us" -f ($us90 - $us0), ($us180 - $us90), ($us180 - $us0))
    if ($us0 -le $MIN_US -or $us180 -ge $MAX_US) { Write-Host "  !! 0 or 180 deg is on the 500/2500 us limit: may be clipped" -ForegroundColor Yellow }
    Write-Host "Saved $name. Disconnect it, connect the next servo, or press Enter to finish." -ForegroundColor Green
  }
}
finally {
  if ($script:ser -and $script:ser.IsOpen) { try { $script:ser.Write("off`n") } catch {}; $script:ser.Close() }
}
if ($results.Count -gt 0) { Write-Host ""; Write-Table | ForEach-Object { Write-Host $_ } }
Write-Host "Table: $txtPath"
