param(
    $PathToCmake = "cmake",
    [Parameter(Mandatory=$true)][ValidateSet("Build", "Run", "BuildAndRun", "BuildAndTest", "TestWithoutBuild", "SaveTestData", "Kill")] $Operation,
    [Parameter(Mandatory=$true)][ArgumentCompleter({
        param(
            [string] $CommandName,
            [string] $ParameterName,
            [string] $WordToComplete,
            [System.Management.Automation.Language.CommandAst] $CommandAst,
            [System.Collections.IDictionary] $FakeBoundParameters
        )
        $possibleValues = @("All")
        $possibleValues += (Get-ChildItem "$PSScriptRoot/../samples/hackathon" -Directory | Where-Object { $_.Name -ne "hack_base" }).Name
        $possibleValues | Where-Object { $_ -like "$WordToComplete*" }
    })] $SampleToRun,
    [String] $Timestamp = (Get-Date -Format "yyyy-MM-dd_hh-mm-ss")
)

# Check if build is configured
if (-not (Test-Path "$PSScriptRoot/../build/windows/vulkan_samples.sln"))
{
    Push-Location
    Set-Location "$PSScriptRoot/.."
    & $PathToCmake -G "Visual Studio 17 2022" -A x64 -S . -Bbuild/windows
    Pop-Location
}

#region Variables that are static between each test
$samplesToRun = if ($SampleToRun -eq "All")
{
    (Get-ChildItem "$PSScriptRoot/../samples/hackathon" -Directory | Where-Object { $_.Name -ne "hack_base" }).Name
}
else
{
    @($SampleToRun)
}
#endregion

#region Functions
function Build
{
    Push-Location
    Set-Location "$PSScriptRoot/.."
    & $PathToCmake --build build/windows --config relwithdebinfo --target vulkan_samples
    Pop-Location
}

function Run
{
    param(
        [String] $SampleName
    )
    
    Start-Process -FilePath "$PSScriptRoot/../build/windows/app/bin/relwithdebinfo/AMD64/vulkan_samples.exe" -ArgumentList @("sample", $SampleName) -WorkingDirectory "$PSScriptRoot/.."
}

function KillTestApp
{
    Get-Process "vulkan_samples" | Stop-Process
}

function IsTestDataFilePresent
{
    return (Test-Path "$PSScriptRoot/../data.json")
}

function WaitForTestFinish
{
    while (-not (IsTestDataFilePresent))
    {
        Start-Sleep -Milliseconds 10
    }
}

function EnsureOldTestDataIsDeleted
{
    if (IsTestDataFilePresent)
    {
        RemoveTestDataFile
    }
}

function RemoveTestDataFile
{
    Remove-Item "$PSScriptRoot/../data.json"
}

function SaveTestData
{
    param(
        [String] $SampleName,
        [String] $Timestamp
    )

    if (-not (Test-Path "$PSScriptRoot/TestResults")) { mkdir "$PSScriptRoot/TestResults" | Out-Null }
    if (-not (Test-Path "$PSScriptRoot/TestResults/$Timestamp")) { mkdir "$PSScriptRoot/TestResults/$Timestamp" | Out-Null }
    if (-not (Test-Path "$PSScriptRoot/TestResults/$Timestamp/Windows")) { mkdir "$PSScriptRoot/TestResults/$Timestamp/Windows" | Out-Null }

    Move-Item "$PSScriptRoot/../data.json" "$PSScriptRoot/TestResults/$Timestamp/Windows/${SampleName}_data.json"
}
#endregion


#region MainScript
try
{
    Push-Location
    Set-Location $PSScriptRoot
    
    if ($Operation -eq "Build")
    {
        Build
    }

    if ($Operation -eq "Run")
    {
        foreach ($sample in $samplesToRun)
        {
            Run -SampleName $sample
        }
    }

    if ($Operation -eq "BuildAndRun")
    {
        Build
        foreach ($sample in $samplesToRun)
        {
            Run -SampleName $sample
        }
    }

    if ($Operation -eq "BuildAndTest")
    {
        Build
        foreach ($sample in $samplesToRun)
        {
            EnsureOldTestDataIsDeleted
            Run -SampleName $sample
            WaitForTestFinish
            KillTestApp
            SaveTestData -SampleName $sample -Timestamp $Timestamp
        }
    }

    if ($Operation -eq "TestWithoutBuild")
    {
        foreach ($sample in $samplesToRun)
        {
            EnsureOldTestDataIsDeleted
            Run -SampleName $sample
            WaitForTestFinish
            KillTestApp
            SaveTestData -SampleName $sample -Timestamp $Timestamp
        }
    }

    if ($Operation -eq "SaveTestData")
    {
        foreach ($sample in $samplesToRun)
        {
            if ($sample -eq "All")
            {
                Write-Warning "Specifying -Operation SaveTestData and -SampleToRun All is incompatible, as only one test data can exist without re-running the tests. Aborting."
                exit 1
            }

            SaveTestData -SampleName $sample -Timestamp $Timestamp
        }
    }

    if ($Operation -eq "Kill")
    {
        KillTestApp
    }
}
finally
{
    Pop-Location
}
#endregion