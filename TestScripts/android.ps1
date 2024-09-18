param(
    $PathToGradlewBat = "$PSScriptRoot/../build/android_gradle/gradlew.bat",
    $PathToAdb = "C:/Program Files (x86)/Android/android-sdk/platform-tools/adb.exe",
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
if (-not (Test-Path $PathToGradlewBat))
{
    & "py $PSScriptRoot/../scripts/generate.py" android
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
function Build()
{
    & $PathToGradlewBat assembleRelease
}

function Deploy()
{
    & $PathToAdb install "./app/build/outputs/apk/release/vulkan_samples-release.apk"
}

function Run()
{
    param(
        [String] $SampleName
    )
    
    & $PathToAdb shell am start -n com.khronos.vulkan_samples/com.khronos.vulkan_samples.SampleLauncherActivity -e sample $SampleName
}

function KillTestApp()
{
    & $PathToAdb shell am force-stop com.khronos.vulkan_samples
}

function IsTestDataFilePresent
{
    $fileList = & $PathToAdb shell "run-as com.khronos.vulkan_samples ls /data/data/com.khronos.vulkan_samples/files/"

    if ($fileList -contains "data.json")
    {
        return $true
    }

    return $false
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
    & $PathToAdb shell "run-as com.khronos.vulkan_samples rm /data/data/com.khronos.vulkan_samples/files/data.json"
}

function SaveTestData()
{
    param(
        [String] $SampleName,
        [String] $Timestamp
    )

    if (-not (Test-Path "$PSScriptRoot/TestResults")) { mkdir "$PSScriptRoot/TestResults" | Out-Null }
    if (-not (Test-Path "$PSScriptRoot/TestResults/$Timestamp")) { mkdir "$PSScriptRoot/TestResults/$Timestamp" | Out-Null }
    if (-not (Test-Path "$PSScriptRoot/TestResults/$Timestamp/Android")) { mkdir "$PSScriptRoot/TestResults/$Timestamp/Android" | Out-Null }

    & $PathToAdb shell "run-as com.khronos.vulkan_samples cat /data/data/com.khronos.vulkan_samples/files/data.json" > "$PSScriptRoot/TestResults/$Timestamp/Android/${SampleName}_data.json"
    RemoveTestDataFile
}
#endregion


#region MainScript
try
{
    Push-Location
    Set-Location (Split-Path $PathToGradlewBat)

    if ($Operation -eq "Build")
    {
        Build
        Deploy
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
        Deploy
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
