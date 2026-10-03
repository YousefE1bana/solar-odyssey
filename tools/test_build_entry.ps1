$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$entry = Join-Path $root 'build.bat'
if (-not (Test-Path -LiteralPath $entry)) { throw 'Supported root build.bat is missing' }
& $entry invalid-build-mode
if ($LASTEXITCODE -eq 0) { throw 'Invalid mode must fail with a nonzero exit code' }
& $entry Release
if ($LASTEXITCODE) { throw 'Release entry failed' }
if (-not (Test-Path -LiteralPath (Join-Path $root '.output/build/SolarOdyssey.exe'))) { throw 'Release binary missing' }
& $entry test
if ($LASTEXITCODE) { throw 'Test entry failed' }
Write-Output 'Windows developer build entry: PASS'
