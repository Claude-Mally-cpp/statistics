param(
    [string]$Preset = "linux-clang-coverage"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
    throw "docker is required to run coverage-report.ps1"
}

try {
    & docker image inspect cpp-ci-clang:latest | Out-Null
} catch {
    & docker build -f Dockerfile.clang -t cpp-ci-clang .
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to build cpp-ci-clang image"
    }
}

$workspace = (Resolve-Path ".").Path
& docker run --rm `
    -v "${workspace}:/project" `
    -w /project `
    cpp-ci-clang `
    bash ./coverage-report.sh $Preset

if ($LASTEXITCODE -ne 0) {
    throw "coverage-report.sh failed"
}
