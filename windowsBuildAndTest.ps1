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

function Find-TestExecutable {
    param(
        [string]$BuildDir,
        [string]$Config
    )

    $candidates = @(
        (Join-Path $BuildDir "statistics_test.exe"),
        (Join-Path $BuildDir $Config | Join-Path -ChildPath "statistics_test.exe")
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    throw "statistics_test.exe not found under '$BuildDir' for config '$Config'"
}

$releaseBuildDir = "build-release"
$debugBuildDir   = "build-debug"

RunStep "Release Configure" { cmake -S . -B $releaseBuildDir -DCMAKE_BUILD_TYPE=Release }
RunStep "Release Build" { cmake --build $releaseBuildDir --config Release }
RunStep "Release Test" {
    $testExe = Find-TestExecutable -BuildDir $releaseBuildDir -Config "Release"
    & $testExe
}

RunStep "Debug Configure" { cmake -S . -B $debugBuildDir -DCMAKE_BUILD_TYPE=Debug }
RunStep "Debug Build" { cmake --build $debugBuildDir --config Debug }
RunStep "Debug Test" {
    $testExe = Find-TestExecutable -BuildDir $debugBuildDir -Config "Debug"
    & $testExe
}
