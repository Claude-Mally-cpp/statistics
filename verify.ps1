param(
    [ValidateSet("quick", "full")]
    [string]$Mode = "quick",
    [switch]$Verbose
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Invoke-Step {
    param(
        [string]$Label,
        [scriptblock]$Command
    )

    Write-Host "==> $Label"
    if ($Verbose) {
        & $Command
        return
    }

    $output = & $Command 2>&1
    if ($LASTEXITCODE -ne 0) {
        if ($output) {
            Write-Host $output
        }
        throw "$Label failed"
    }

    if ($output) {
        Write-Host $output
    }
}

function Test-Command {
    param([string]$Name)

    return $null -ne (Get-Command $Name -ErrorAction SilentlyContinue)
}

function Invoke-FormatCheck {
    $clangFormat = if ($env:CLANG_FORMAT) { $env:CLANG_FORMAT } else { "clang-format" }
    $files = git ls-files '*.cpp' '*.hpp' '*.h'
    if (-not $files) {
        Write-Host "No source files found."
        return
    }

    & $clangFormat --version
    if ($LASTEXITCODE -ne 0) {
        throw "clang-format version check failed"
    }

    & $clangFormat --dry-run --Werror $files
    if ($LASTEXITCODE -ne 0) {
        throw "clang-format check failed"
    }

    Write-Host "All files are properly formatted"
}

function Invoke-DoxygenVerify {
    if (-not (Test-Command "doxygen")) {
        Write-Host "Skipping Doxygen verification: doxygen not found."
        return
    }

    & doxygen Doxyfile
    if ($LASTEXITCODE -ne 0) {
        throw "Doxygen verification failed"
    }
}

function Invoke-CppcheckLocal {
    if (-not (Test-Command "cppcheck")) {
        Write-Host "Skipping cppcheck: command not found."
        return
    }

    $buildDir = $null
    if (Test-Path "out/build/msvc-x64-debug/_deps") {
        $buildDir = "out/build/msvc-x64-debug"
    } elseif (Test-Path "out/build/linux-gcc-debug/_deps") {
        $buildDir = "out/build/linux-gcc-debug"
    } elseif (Test-Path "out/build/linux-clang-debug/_deps") {
        $buildDir = "out/build/linux-clang-debug"
    }

    if (-not $buildDir) {
        Write-Host "Skipping cppcheck: no configured build tree with fetched deps found."
        Write-Host "Run a configure step first so fmt/googletest headers exist under out/build/*/_deps."
        return
    }

    & cppcheck include test `
        -I include `
        -I "$buildDir/_deps/fmt-src/include" `
        -I "$buildDir/_deps/googletest-src/googletest/include" `
        --language=c++ `
        --std=c++23 `
        --enable=warning,style,performance,portability,information,missingInclude `
        --inline-suppr `
        --suppress=missingIncludeSystem `
        -i "$buildDir/_deps" `
        -q
    if ($LASTEXITCODE -ne 0) {
        throw "cppcheck failed"
    }
}

function Invoke-Quick {
    Invoke-Step "format check" { Invoke-FormatCheck }
}

function Invoke-Full {
    Invoke-Quick
    Invoke-Step "doxygen verify" { Invoke-DoxygenVerify }
    Invoke-Step "windows build and test" { & pwsh -File .\windowsBuildAndTest.ps1 @($Verbose ? "-Verbose" : $null) }
    Invoke-Step "cppcheck" { Invoke-CppcheckLocal }

    if (-not (Test-Command "docker")) {
        Write-Host "Skipping Docker checks: docker not found."
        return
    }

    if (-not (Test-Command "bash")) {
        Write-Host "Skipping Docker checks: bash not found."
        return
    }

    Invoke-Step "rebuild docker images" { & bash ./rebuildDockerImages.sh }
    Invoke-Step "docker linux build and test" { & bash ./dockerLinuxBuildAndTest.sh }
}

switch ($Mode) {
    "quick" { Invoke-Quick }
    "full" { Invoke-Full }
}
