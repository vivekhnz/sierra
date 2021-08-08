
$ProgressPreference = 'SilentlyContinue'
$sw = [System.Diagnostics.Stopwatch]::StartNew()

$Configuration = 'Debug'
$Platform = 'x64'
$BaseIntermediateOutputPath = "obj\$Configuration\$Platform"
$OutputPath = "bin\$Configuration\$Platform"
$Random = [System.Random]::new();

$CommonCompilerFlags = @(
    '/nologo',
    '/std:c++17',
    '/EHa',
    '/Z7',
    '/FC'
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

function Invoke-Msvc {
    param
    (
        [Parameter(Mandatory = $true)] [string] $SourceFile,
        [Parameter(Mandatory = $true)] [string] $OutputName,
        [Parameter(Mandatory = $true)] [string] $IntermediateOutputDirName,
        [Parameter(Mandatory = $false)] [string[]] $IncludePaths,
        [Parameter(Mandatory = $false)] [string[]] $ImportLibs,
        [Switch] $BuildDll,
        [Switch] $NoImportLib,
        [Switch] $RandomizePdbFilename
    )

    $intermediateOutputPath = "$BaseIntermediateOutputPath\$IntermediateOutputDirName"
    if (!(Test-Path $intermediateOutputPath)) {
        New-Item -Path $intermediateOutputPath -ItemType Directory | Out-Null
    }
    
    $compilerFlags = @(
        $SourceFile,
        "/Fo$intermediateOutputPath/"
    )
    $compilerFlags += ($IncludePaths | ForEach-Object { "/I $_" })

    $linkerFlags = @();
    if ($RandomizePdbFilename.IsPresent) {
        Remove-Item "$OutputPath\$OutputName.*.pdb" -ErrorAction SilentlyContinue
        $linkerFlags += "/PDB:$OutputPath\$OutputName.$($Random.Next()).pdb"
    }
    else {
        $linkerFlags += "/PDB:$OutputPath\$OutputName.pdb"
    }
    if ($ImportLibs) {
        $linkerFlags += $ImportLibs
    }
    if ($BuildDll.IsPresent) {
        $linkerFlags += "/out:$OutputPath\$OutputName.dll"
        $linkerFlags += '/DLL'
    }
    else {
        $linkerFlags += "/out:$OutputPath\$OutputName.exe"
    }
    if ($NoImportLib.IsPresent) {
        $linkerFlags += '/NOIMPLIB'
    }

    $args = $CommonCompilerFlags + $compilerFlags + '/link' + $CommonLinkerFlags + $linkerFlags
    $proc = Start-Process cl -ArgumentList $args -NoNewWindow -PassThru
    $proc.Handle | Out-Null

    return @{
        OutputName = $OutputName;
        Process    = $proc;
    }
}

function Invoke-Dotnet {
    param
    (
        [Parameter(Mandatory = $true)] [string] $SolutionFile,
        [Parameter(Mandatory = $true)] [string] $OutputName
    )

    $args = @(
        'build',
        $SolutionFile,
        '--noLogo',
        '--verbosity:quiet',
        '--no-incremental',
        '-p:CopyRetryCount=0'
    )
    $proc = Start-Process dotnet -ArgumentList $args -NoNewWindow -PassThru
    $proc.Handle | Out-Null
    
    return @{
        OutputName = $OutputName;
        Process = $proc;
    }
}

Initialize-Environment -Platform $platform

if (!(Test-Path $OutputPath)) {
    New-Item -Path $OutputPath -ItemType Directory | Out-Null
}

if (!(Test-Path 'deps\nuget\glfw*')) {
    & nuget restore 'packages.config'
}

$LockFilePath = "$OutputPath\build.lock"
[System.IO.File]::WriteAllText($LockFilePath, 'build');

$builds = @()
$builds += Invoke-Msvc `
    -SourceFile 'src\Engine\engine.cpp' -OutputName 'terrain_engine' `
    -IntermediateOutputDirName 'Engine' `
    -BuildDll -RandomizePdbFilename -NoImportLib `
    -IncludePaths @(
        'deps',
        'deps\nuget\glm.0.9.9.700\build\native\include'
    )
$builds += Invoke-Msvc `
    -SourceFile 'src\Game\game.cpp' -OutputName 'terrain_game' `
    -IntermediateOutputDirName 'Game' `
    -BuildDll -RandomizePdbFilename -NoImportLib `
    -IncludePaths @(
        'deps',
        'deps\nuget\glm.0.9.9.700\build\native\include'
)
# don't try build the game executable if it is currently running
if (!(Get-Process "win32_terrain" -ErrorAction SilentlyContinue)) {
    $builds += Invoke-Msvc `
        -SourceFile 'src\Game\win32_game.cpp' -OutputName 'win32_terrain' `
        -IntermediateOutputDirName 'Game' `
        -NoImportLib `
        -IncludePaths @(
            'deps',
            'deps\nuget\glm.0.9.9.700\build\native\include',
            'deps\nuget\glfw.3.3.2\build\native\include'
        ) `
        -ImportLibs @(
            "deps\nuget\glfw.3.3.2\build\native\lib\dynamic\v142\$platform\glfw3dll.lib"
        )
}

$builds += Invoke-Msvc `
    -SourceFile 'src\EditorCore\editor.cpp' -OutputName 'terrain_editor' `
    -IntermediateOutputDirName 'EditorCore' `
    -BuildDll -RandomizePdbFilename -NoImportLib `
    -IncludePaths @(
        'deps',
        'deps\nuget\glm.0.9.9.700\build\native\include'
    )

# don't try build the editor executable if it is currently running
if (!(Get-Process "Terrain.Editor" -ErrorAction SilentlyContinue)) {
    $builds += Invoke-Dotnet -SolutionFile 'Terrain.sln' -OutputName 'Editor'
}

$glfwDllSrcPath = "deps\nuget\glfw.3.3.2\build\native\bin\dynamic\v142\$platform\glfw3.dll"
$glfwDllDstPath = "$OutputPath\glfw3.dll"
if (!(Test-Path $glfwDllDstPath)) {
    Copy-Item -Path $glfwDllSrcPath -Destination $glfwDllDstPath
}

$builds.Process | Wait-Process

# create a junction of the repo into 'C:\temp' so hardcoded executable paths in solution
# files can point there
$MirrorPath = 'C:\temp\terrain_mirror'
if (!(Test-Path $MirrorPath)) {
    New-Item -ItemType Junction -Path $MirrorPath -Value '.' | Out-Null
}

Remove-Item $LockFilePath -Force

$sw.Stop();
if ($builds.Process.ExitCode | Where-Object { $_ -ne 0 }) {
    Write-Host "Build failed" -ForegroundColor Red
    $tick = [char]::ConvertFromUtf32([convert]::ToInt32('2705', 16))
    $cross = [char]::ConvertFromUtf32([convert]::ToInt32('274C', 16))
    $builds | ForEach-Object {
        if ($_.Process.ExitCode -eq 0) {
            Write-Host "$tick $($_.OutputName)" -ForegroundColor Green
        }
        else {
            Write-Host "$cross $($_.OutputName)" -ForegroundColor Red
        }
    }
    exit 1
}
else {
    Write-Host "Build succeeded ($($sw.ElapsedMilliseconds)ms)" -ForegroundColor Green
    exit 0
}