
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

function Write-GeneratedCode {
    # extract engine APIs
    $engineHeaderFiles = @(
        'src\Engine\engine_assets.h',
        'src\Engine\engine_renderer.h'
    )
    $apiDefineMacro = [regex]::new('^#define (?<macro>([A-Z]|\d|_)+)\(name\)')
    $apis = New-Object System.Collections.ArrayList
    foreach ($headerFile in $engineHeaderFiles) {
        $lines = Get-Content $headerFile
        for ($i = 0; $i -lt $lines.Count; $i++) {
            $line = $lines[$i]
            $macroMatch = $apiDefineMacro.Match($line)
            if ($macroMatch.Success) {
                $macroName = $macroMatch.Groups['macro'].Value
                $typeNameBuilder = [System.Text.StringBuilder]::new()
                $uppercase = $true
                for ($j = 0; $j -lt $macroName.Length; $j++) {
                    $char = $macroName[$j]
                    if ($char -eq '_') {
                        $uppercase = $true
                    }
                    elseif ($uppercase) {
                        $typeNameBuilder.Append($char) | Out-Null
                        $uppercase = $false
                    }
                    else {
                        $typeNameBuilder.Append([char]::ToLower($char)) | Out-Null
                    }
                }
                $typeName = $typeNameBuilder.ToString()
                $functionName = [char]::ToLower($typeName[0]) + $typeName.Substring(1)
                $apis.Add([PSCustomObject]@{
                    MacroName = $macroName;
                    TypeName = $typeName;
                    FunctionName = $functionName;
                }) | Out-Null
            }
        }
    }

    # engine_generated.h
    $generatedSrcBuilder = [System.Text.StringBuilder]::new()
    foreach ($api in $apis) {
        $generatedSrcBuilder.AppendLine("typedef $($api.MacroName)($($api.TypeName));") | Out-Null
    }
    $generatedSrcBuilder.AppendLine("`nstruct EngineApi`n{") | Out-Null
    foreach ($api in $apis) {
        $generatedSrcBuilder.AppendLine("    $($api.TypeName) *$($api.FunctionName);") | Out-Null
    }
    $generatedSrcBuilder.AppendLine("};") | Out-Null
    [System.IO.File]::WriteAllText('src\Engine\engine_generated.h', $generatedSrcBuilder.ToString())

    # engine_generated.cpp
    $generatedSrcBuilder.Clear() | Out-Null
    $generatedSrcBuilder.AppendLine("void bindApi(EngineApi *api)`n{") | Out-Null
    foreach ($api in $apis) {
        $generatedSrcBuilder.AppendLine("    api->$($api.FunctionName) = $($api.FunctionName);") | Out-Null
    }
    $generatedSrcBuilder.AppendLine("};") | Out-Null
    [System.IO.File]::WriteAllText('src\Engine\engine_generated.cpp', $generatedSrcBuilder.ToString())

    # editor_generated.cpp
    $generatedSrcBuilder.Clear() | Out-Null
    foreach ($api in $apis) {
        $generatedSrcBuilder.AppendLine("#define $($api.FunctionName) Engine->$($api.FunctionName)") | Out-Null
    }
    [System.IO.File]::WriteAllText('src\EditorCore\editor_generated.cpp', $generatedSrcBuilder.ToString());
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

    $procArgs = $CommonCompilerFlags + $compilerFlags + '/link' + $CommonLinkerFlags + $linkerFlags
    $proc = Start-Process cl -ArgumentList $procArgs -NoNewWindow -PassThru
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

    $procArgs = @(
        'build',
        $SolutionFile,
        '--noLogo',
        '--verbosity:quiet',
        '--no-incremental',
        '-p:CopyRetryCount=0'
    )
    $proc = Start-Process dotnet -ArgumentList $procArgs -NoNewWindow -PassThru
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

if (!(Test-Path 'deps\nuget\glm*')) {
    & nuget restore 'packages.config'
}

$LockFilePath = "$OutputPath\build.lock"
[System.IO.File]::WriteAllText($LockFilePath, 'build');

Write-GeneratedCode

$builds = @()
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