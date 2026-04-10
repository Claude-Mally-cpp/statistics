param(
    [string]$Preset = "linux-clang-coverage"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$reportPath = Join-Path "out/coverage/$Preset/html" "index.html"
if (-not (Test-Path $reportPath)) {
    throw "Coverage report not found: $reportPath"
}

Start-Process $reportPath
