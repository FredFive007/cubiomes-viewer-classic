param(
    [Parameter(Mandatory=$true)][string]$QtRoot,
    [Parameter(Mandatory=$true)][string]$MinGwRoot
)
$ErrorActionPreference = 'Stop'
$env:PATH = "$MinGwRoot\bin;$QtRoot\bin;" + $env:PATH
$buildDir = Join-Path $PSScriptRoot 'build-release'
& "$QtRoot\bin\lrelease.exe" "$PSScriptRoot\rc\lang\zh_CN.ts" -qm "$PSScriptRoot\rc\lang\zh_CN.qm"
if ($LASTEXITCODE) { throw 'Translation build failed' }
& cmake -S $PSScriptRoot -B $buildDir -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_C_COMPILER=$MinGwRoot/bin/gcc.exe" "-DCMAKE_CXX_COMPILER=$MinGwRoot/bin/g++.exe" "-DCMAKE_PREFIX_PATH=$QtRoot"
if ($LASTEXITCODE) { throw 'CMake configure failed' }
& cmake --build $buildDir --parallel 6
if ($LASTEXITCODE) { throw 'Build failed' }
& ctest --test-dir $buildDir --output-on-failure
if ($LASTEXITCODE) { throw 'Tests failed' }
foreach ($name in @('Qt5Core.dll','Qt5Gui.dll','Qt5Widgets.dll')) {
    Copy-Item -LiteralPath "$QtRoot\bin\$name" -Destination $buildDir -Force
}
foreach ($name in @('libgcc_s_seh-1.dll','libstdc++-6.dll','libwinpthread-1.dll')) {
    Copy-Item -LiteralPath "$MinGwRoot\bin\$name" -Destination $buildDir -Force
}
foreach ($plugin in @('platforms/qwindows.dll','styles/qwindowsvistastyle.dll',
                     'imageformats/qgif.dll','imageformats/qico.dll','imageformats/qjpeg.dll')) {
    $target = Join-Path $buildDir $plugin
    New-Item -ItemType Directory -Path (Split-Path $target -Parent) -Force | Out-Null
    Copy-Item -LiteralPath "$QtRoot/plugins/$plugin" -Destination $target -Force
}
@('[Paths]','Prefix=.','Plugins=.') | Set-Content -LiteralPath "$buildDir\qt.conf" -Encoding ascii
