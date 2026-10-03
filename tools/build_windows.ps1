param(
    [ValidateSet('Release','Debug','test','clean')]
    [string]$Mode = 'Release',
    [ValidateRange(1,64)]
    [int]$Jobs = 6
)
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$output = Join-Path $root '.output'
$build = Join-Path $output 'build'
try {
    if ($Mode -eq 'clean') {
        if (Test-Path -LiteralPath $build) {
            $resolved = (Resolve-Path -LiteralPath $build).Path
            if ($resolved -ne $build -or -not $resolved.StartsWith($output + '\', [StringComparison]::OrdinalIgnoreCase)) {
                throw 'Build cleanup target is outside the expected output directory'
            }
            Remove-Item -LiteralPath $resolved -Recurse -Force
        }
        Write-Output 'Cleaned .output/build. Source and release artifacts are unchanged.'
        exit 0
    }
    $prefix = $env:SOLAR_MINGW_PREFIX
    if (-not $prefix -and (Test-Path -LiteralPath (Join-Path $build 'CMakeCache.txt'))) {
        $compiler = Select-String -LiteralPath (Join-Path $build 'CMakeCache.txt') -Pattern '^CMAKE_CXX_COMPILER:FILEPATH=(.+)$'
        if ($compiler) { $prefix = Split-Path (Split-Path $compiler.Matches[0].Groups[1].Value -Parent) -Parent }
    }
    if (-not $prefix -and (Get-Command g++.exe -ErrorAction SilentlyContinue)) {
        $prefix = Split-Path (Split-Path (Get-Command g++.exe).Source -Parent) -Parent
    }
    if (-not $prefix) {
        foreach ($candidate in @((Join-Path $env:SystemDrive 'msys64/mingw64'), (Join-Path $env:LOCALAPPDATA 'msys64/mingw64'))) {
            if (Test-Path -LiteralPath (Join-Path $candidate 'bin/g++.exe')) { $prefix = $candidate; break }
        }
    }
    if (-not $prefix -or -not (Test-Path -LiteralPath (Join-Path $prefix 'bin/g++.exe'))) {
        throw 'MinGW GCC not found. Use an MSYS2 MINGW64 toolchain on PATH or set SOLAR_MINGW_PREFIX.'
    }
    $env:PATH = (Join-Path $prefix 'bin') + ';' + $env:PATH
    foreach ($command in 'cmake.exe','ninja.exe','g++.exe','windres.exe') {
        if (-not (Get-Command $command -ErrorAction SilentlyContinue)) { throw "Required tool missing: $command" }
    }
    $configuration = if ($Mode -eq 'test') { 'Release' } else { $Mode }
    Write-Output "Solar Odyssey: configuring $configuration with MinGW/Ninja in .output/build"
    & cmake.exe -S $root -B $build -G Ninja "-DCMAKE_BUILD_TYPE=$configuration" '-DHEADLESS_TESTS=OFF'
    if ($LASTEXITCODE) { exit $LASTEXITCODE }
    & cmake.exe --build $build --parallel $Jobs
    if ($LASTEXITCODE) { exit $LASTEXITCODE }
    if ($Mode -eq 'test') {
        & ctest.exe --test-dir $build --output-on-failure
        if ($LASTEXITCODE) { exit $LASTEXITCODE }
    }
    Write-Output "Ready: $build\SolarOdyssey.exe"
    exit 0
} catch {
    Write-Error $_ -ErrorAction Continue
    exit 1
}
