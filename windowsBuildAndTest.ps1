param(
    [switch]$Verbose
)

function RunStep {
    param(
        [string]$Label,
        [scriptblock]$Command
    )
    Write-Host "=== $Label ==="
    if ($Verbose) {
        & $Command
        if ($LASTEXITCODE -eq 0) {
            Write-Host "${Label}: SUCCESS" -ForegroundColor Green
        } else {
            Write-Host "${Label}: FAILURE" -ForegroundColor Red
        }
    } else {
        $output = & $Command 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Host "${Label}: SUCCESS" -ForegroundColor Green
        } else {
            Write-Host "${Label}: FAILURE" -ForegroundColor Red
            Write-Host $output
        }
    }
}

RunStep "Release Configure" { cmake -S . -B build -DCMAKE_BUILD_TYPE=Release }
RunStep "Release Build" { cmake --build build --config Release }
RunStep "Release Test" { .\build\Release\statistics_test.exe }

RunStep "Debug Configure" { cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug }
RunStep "Debug Build" { cmake --build build --config Debug }
RunStep "Debug Test" { .\build\Debug\statistics_test.exe }
