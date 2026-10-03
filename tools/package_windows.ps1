param(
    [string]$BuildDirectory = ".output/build",
    [string]$RuntimePrefix = "C:\msys64\mingw64",
    [string]$MakeNSIS = "C:\msys64\mingw64\bin\makensis.exe"
)
$ErrorActionPreference = "Stop"
$sourceRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$buildRoot = (Resolve-Path (Join-Path $sourceRoot $BuildDirectory)).Path
$versionMatch = Select-String -LiteralPath (Join-Path $sourceRoot "CMakeLists.txt") -Pattern 'project\(SolarOdyssey VERSION ([0-9.]+)'
$version = $versionMatch.Matches[0].Groups[1].Value
$cache = Get-Content -LiteralPath (Join-Path $buildRoot "CMakeCache.txt")
if ($cache -notcontains 'CMAKE_BUILD_TYPE:STRING=Release') { throw "Packaging requires an optimized Release build" }
$exe = Join-Path $buildRoot "SolarOdyssey.exe"
if ((Get-Item -LiteralPath $exe).VersionInfo.ProductVersion -ne $version) { throw "Executable version differs from CMake project" }
$packageParent = Join-Path $sourceRoot ".output/staging"
$stage = Join-Path $packageParent "SolarOdyssey-$version"
$artifacts = Join-Path $sourceRoot ".output/release"
New-Item -ItemType Directory -Force $packageParent,$artifacts | Out-Null
if (Test-Path -LiteralPath $stage) {
    $resolved = (Resolve-Path -LiteralPath $stage).Path
    if (-not $resolved.StartsWith($packageParent + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw "Unsafe staging destination" }
    Remove-Item -LiteralPath $resolved -Recurse -Force
}
New-Item -ItemType Directory $stage | Out-Null
Copy-Item -LiteralPath $exe -Destination $stage
foreach ($name in 'Textures','shaders','assets','icon.png','icon.ico','LICENSE','THIRD_PARTY_NOTICES.md') {
    Copy-Item -LiteralPath (Join-Path $buildRoot $name) -Destination $stage -Recurse
}
Copy-Item -LiteralPath (Join-Path $sourceRoot 'docs/PLAYER_GUIDE.md') -Destination $stage
# Inspect the actual import graph. Never ship Windows system DLLs.
$queue = [Collections.Generic.Queue[string]]::new()
$queue.Enqueue($exe)
$visited = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
while ($queue.Count) {
    $binary = $queue.Dequeue()
    $imports = & (Join-Path $RuntimePrefix 'bin/objdump.exe') -p $binary
    if ($LASTEXITCODE) { throw "Cannot inspect dependencies of $binary" }
    foreach ($line in $imports) {
        if ($line -match 'DLL Name: (.+)$') {
            $dll = $Matches[1].Trim()
            if (-not $visited.Add($dll)) { continue }
            $dependency = Join-Path $RuntimePrefix "bin/$dll"
            if (Test-Path -LiteralPath $dependency) {
                Copy-Item -LiteralPath $dependency -Destination $stage
                $queue.Enqueue($dependency)
            } elseif (-not (Test-Path -LiteralPath (Join-Path $env:WINDIR "System32/$dll")) -and $dll -notmatch '^(api-ms-|ext-ms-)') {
                throw "Unresolved runtime dependency: $dll"
            }
        }
    }
}
$ownedDlls = Join-Path $packageParent "uninstall-runtime-$version.nsh"
Get-ChildItem -LiteralPath $stage -Filter '*.dll' -File | ForEach-Object {
    'Delete "$INSTDIR\' + $_.Name + '"'
} | Set-Content -LiteralPath $ownedDlls -Encoding ascii
$portable = Join-Path $artifacts "SolarOdyssey-$version-Windows-Portable.zip"
Compress-Archive -LiteralPath $stage -DestinationPath $portable -Force -CompressionLevel Optimal
$setup = Join-Path $artifacts "SolarOdyssey-$version-Windows-Setup.exe"
& $MakeNSIS "/WX" "/DVERSION=$version" "/DSTAGE=$stage" "/DOUTPUT=$setup" "/DOWNED_DLLS=$ownedDlls" (Join-Path $sourceRoot 'tools/windows_installer.nsi')
if ($LASTEXITCODE) { throw "Installer compilation failed" }
Get-FileHash -LiteralPath $portable,$setup -Algorithm SHA256 | ForEach-Object { "$($_.Hash.ToLower())  $([IO.Path]::GetFileName($_.Path))" } | Set-Content -LiteralPath (Join-Path $artifacts 'SHA256SUMS.txt') -Encoding ascii
Get-Item -LiteralPath $portable,$setup | Select-Object Name,Length
