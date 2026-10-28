param(
    [Parameter(Mandatory=$true)][string]$Stage,
    [Parameter(Mandatory=$true)][string]$Output,
    [string]$Mingw = 'C:\msys64\mingw64'
)
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$framework = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319'
function Build-MidnightSetup([string]$Destination, [string]$Payload) {
    $compilerArgs = @('/nologo','/target:winexe','/platform:x64','/optimize+',
        "/out:$Destination", "/win32icon:$repo\src\qt\res\icons\machinecoin.ico",
        "/resource:$repo\COPYING,Machinecoin.License", '/reference:System.dll',
        '/reference:System.Core.dll', '/reference:System.Drawing.dll',
        "/reference:$framework\WPF\PresentationFramework.dll",
        "/reference:$framework\WPF\PresentationCore.dll",
        "/reference:$framework\WPF\WindowsBase.dll",
        "/reference:$framework\System.Xaml.dll")
    if ($Payload) { $compilerArgs += "/resource:$Payload,Machinecoin.Payload" }
    $compilerArgs += (Join-Path $PSScriptRoot 'MidnightSetup.cs')
    & "$framework\csc.exe" @compilerArgs
    if ($LASTEXITCODE) { throw 'Midnight setup interface compilation failed.' }
}
$Stage = [IO.Path]::GetFullPath($Stage)
if (Test-Path -LiteralPath $Stage) { throw 'Use a new staging directory to avoid including stale files.' }
New-Item -ItemType Directory -Path $Stage | Out-Null
foreach ($binary in @('src\qt\machinecoin-qt.exe','src\machinecoin-cli.exe','src\machinecoind.exe')) {
    Copy-Item -LiteralPath (Join-Path $repo $binary) -Destination $Stage
}
Copy-Item -LiteralPath (Join-Path $repo 'COPYING') -Destination $Stage
Copy-Item -LiteralPath (Join-Path $repo 'doc\INSTALL-Windows-preview.md') -Destination (Join-Path $Stage 'README.txt')
Build-MidnightSetup (Join-Path $Stage 'MachinecoinMaintenance.exe') ''
foreach ($plugin in @('platforms\qwindows.dll','imageformats\qico.dll','imageformats\qjpeg.dll','imageformats\qsvg.dll')) {
    $destination = Join-Path $Stage $plugin
    New-Item -ItemType Directory -Force -Path (Split-Path $destination) | Out-Null
    Copy-Item -LiteralPath (Join-Path "$Mingw\share\qt5\plugins" $plugin) -Destination $destination
}
Set-Content -LiteralPath (Join-Path $Stage 'qt.conf') -Encoding utf8 -Value "[Paths]`nPlugins=."
$queue = [Collections.Generic.Queue[string]]::new()
Get-ChildItem -LiteralPath $Stage -Recurse -File | Where-Object { $_.Extension -in '.exe','.dll' } | ForEach-Object { $queue.Enqueue($_.FullName) }
$seen = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
while ($queue.Count) {
    $file = $queue.Dequeue()
    $imports = & "$Mingw\bin\objdump.exe" -p $file
    if ($LASTEXITCODE) { throw "Cannot inspect $file" }
    foreach ($line in $imports) {
        if ($line -notmatch 'DLL Name:\s+(\S+)') { continue }
        $dll = $Matches[1]
        if ($dll -match '^(msys-|cygwin)') { throw "Non-native runtime dependency: $dll" }
        if (-not $seen.Add($dll)) { continue }
        $dependency = Join-Path "$Mingw\bin" $dll
        if (Test-Path -LiteralPath $dependency) {
            $dest = Join-Path $Stage $dll
            Copy-Item -LiteralPath $dependency -Destination $dest
            $queue.Enqueue($dest)
        } elseif ($dll -notmatch '^(api-ms-|ext-ms-)' -and -not (Test-Path -LiteralPath (Join-Path "$env:WINDIR\System32" $dll))) {
            throw "Unresolved runtime dependency: $dll"
        }
    }
}
# Preserve dependency license texts alongside the redistributable libraries.
Copy-Item -LiteralPath "$Mingw\share\licenses" -Destination (Join-Path $Stage 'licenses') -Recurse
$deps = Join-Path (Split-Path $repo) 'deps'
foreach ($item in @('bls-signatures-20181101\LICENSE','db-4.8.30.NC\LICENSE','protobuf-21.12\LICENSE','boost_1_74_0\LICENSE_1_0.txt')) {
    $licenseName = ($item -replace '\\','-')
    Copy-Item -LiteralPath (Join-Path $deps $item) -Destination (Join-Path "$Stage\licenses" $licenseName)
}
foreach ($item in @('COPYING','COPYING.LESSER','LICENSE','LICENSE.Apache-2.0','LICENSE.LGPL-2.1')) {
    Copy-Item -LiteralPath (Join-Path "$deps\bls-signatures-20181101\contrib\relic" $item) -Destination (Join-Path "$Stage\licenses" "relic-$item")
}
$uninstall = @('; Exact generated package manifest.')
Get-ChildItem -LiteralPath $Stage -Recurse -File | ForEach-Object {
    $relative = $_.FullName.Substring($Stage.Length + 1)
    $uninstall += 'Delete "$INSTDIR\' + $relative + '"'
}
$uninstall += 'Delete "$INSTDIR\uninstall-files.nsh"'
Get-ChildItem -LiteralPath $Stage -Recurse -Directory | Sort-Object { $_.FullName.Length } -Descending | ForEach-Object {
    $uninstall += 'RMDir "$INSTDIR\' + $_.FullName.Substring($Stage.Length + 1) + '"'
}
Set-Content -LiteralPath (Join-Path $Stage 'uninstall-files.nsh') -Value $uninstall -Encoding utf8
New-Item -ItemType Directory -Force -Path (Split-Path ([IO.Path]::GetFullPath($Output))) | Out-Null
Push-Location $PSScriptRoot
$payload = Join-Path ([IO.Path]::GetTempPath()) ('Machinecoin-payload-' + [guid]::NewGuid().ToString('N') + '.exe')
try {
    & "$Mingw\bin\makensis.exe" "/INPUTCHARSET" "UTF8" "/DSTAGE=$Stage" "/DOUTPUT=$payload" midnight-installer.nsi
    if ($LASTEXITCODE) { throw 'Installer compilation failed.' }
    Build-MidnightSetup $Output $payload
} finally {
    Pop-Location
    if (Test-Path -LiteralPath $payload) { Remove-Item -LiteralPath $payload }
}
Get-FileHash -Algorithm SHA256 -LiteralPath $Output
