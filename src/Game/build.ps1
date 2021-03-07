$Configuration = 'Debug'
$Platform = 'x64'
$IntermediateOutputPath = "..\..\obj\$Configuration\$Platform\Game"
$OutputPath = "..\..\bin\$Configuration\$Platform"

$CommonCompilerFlags = @(
    '/nologo',
    '/std:c++17',
    '/EHa',
    '/Z7'
)
if ($Configuration -eq 'Debug') {
    $CommonCompilerFlags += '/MTd'
    $CommonCompilerFlags += '/D _DEBUG'
}
else {
    $CommonCompilerFlags += '/MT'
}

$CommonLinkerFlags = @(
    '/INCREMENTAL:NO'
)

function Initialize-Environment([string] $Platform) {
    $envVarCacheFile = "C:\temp\terrain_build.$Platform.env"
    $envVars = ''

    $useCacheFile = $false
    if (Test-Path $envVarCacheFile) {
        $useCacheFile = $true
        $cacheAge = ([DateTime]::Now - (Get-Item $envVarCacheFile).LastWriteTime)
        if ($cacheAge.TotalHours -gt 6) {
            Remove-Item -Path $envVarCacheFile
            $useCacheFile = $false
        }
    }
    if ($useCacheFile) {
        $envVars = Get-Content -Path $envVarCacheFile
    }
    else {
        $vcvarsall = 'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat'
        $envVars = cmd /c "`"$vcvarsall`" $Platform > nul 2>&1 && set";
        $envVars | Out-File $envVarCacheFile
    }
    
    foreach ($var in $envVars) {
        $equalsPos = $var.IndexOf('=')
        $varName = $var.Substring(0, $equalsPos)
        $varValue = $var.Substring($equalsPos + 1, $var.Length - $equalsPos - 1)
        [System.Environment]::SetEnvironmentVariable($varName, $varValue)
    }
}

function Invoke-Compiler {
    param
    (
        [Parameter(Mandatory = $true)] [string] $SourceFile,
        [Parameter(Mandatory = $true)] [string] $OutputName,
        [Parameter(Mandatory = $true)] [string[]] $IncludePaths,
        [Parameter(Mandatory = $true)] [string[]] $ImportLibs,
        [Switch] $BuildDll,
        [Switch] $NoImportLib,
        [Switch] $RandomizePdbFilename
    )

    $compilerFlags = @(
        $SourceFile,
        "/Fo$IntermediateOutputPath/"
    )
    $compilerFlags += ($IncludePaths | ForEach-Object { "/I $_" })

    $linkerFlags = @();
    if ($RandomizePdbFilename.IsPresent) {
        Remove-Item "$OutputPath\$OutputName.*.pdb" -ErrorAction SilentlyContinue
        $linkerFlags += "/PDB:$OutputPath\$OutputName.$(Get-Random).pdb"
    }
    else {
        $linkerFlags += "/PDB:$OutputPath\$OutputName.pdb"
    }
    $linkerFlags += $ImportLibs
    if ($BuildDll.IsPresent) {
        $linkerFlags +=  "/out:$OutputPath\$OutputName.dll"
        $linkerFlags += '/DLL'
    }
    else {
        $linkerFlags +=  "/out:$OutputPath\$OutputName.exe"
    }
    if ($NoImportLib.IsPresent) {
        $linkerFlags += '/NOIMPLIB'
    }

    $args = $CommonCompilerFlags + $compilerFlags + '/link' + $CommonLinkerFlags + $linkerFlags
    $proc = Start-Process cl -ArgumentList $args -NoNewWindow -PassThru
    return $proc
}

$LockFilePath = "$OutputPath\build.lock"
"build" | Out-File $LockFilePath

Initialize-Environment -Platform $platform

if (!(Test-Path $IntermediateOutputPath)) {
    New-Item -Path $IntermediateOutputPath -ItemType Directory | Out-Null
}
if (!(Test-Path $OutputPath)) {
    New-Item -Path $OutputPath -ItemType Directory | Out-Null
}

$procs = @()
$procs += Invoke-Compiler -SourceFile 'game.cpp' -OutputName 'terrain_game' `
    -BuildDll -RandomizePdbFilename -NoImportLib `
    -IncludePaths @(
        '..\..\deps',
        '..\..\deps\nuget\glm.0.9.9.700\build\native\include'
    ) `
    -ImportLibs @(
        "$OutputPath\Terrain.Engine.lib"
    )
$procs += Invoke-Compiler -SourceFile 'win32_game.cpp' -OutputName 'win32_terrain' -NoImportLib `
    -IncludePaths @(
        '..\..\deps',
        '..\..\deps\nuget\glm.0.9.9.700\build\native\include',
        '..\..\deps\nuget\glfw.3.3.2\build\native\include'
    ) `
    -ImportLibs @(
        "$OutputPath\Terrain.Engine.lib"
    )
$procs | Wait-Process

# create a junction of the repo into 'C:\temp' so hardcoded executable paths in solution
# files can point there
$MirrorPath = 'C:\temp\terrain_mirror'
if (!(Test-Path $MirrorPath)) {
    New-Item -ItemType Junction -Path $MirrorPath -Value '..\..\' | Out-Null
}

Remove-Item $LockFilePath -Force

Write-Host "Done."