param(
    $PathToCmake = "cmake",
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
    [String] $Timestamp = (Get-Date -Format "yyyy-MM-dd_hh-mm-ss"),
    [Parameter(Mandatory=$true)][ValidateSet("All", "Android", "Windows")] $Platforms
)

if ($Platforms -eq "All" -or $Platforms -eq "Android")
{
    & $PSScriptRoot\android.ps1 -Operation $Operation -SampleToRun $SampleToRun -Timestamp $Timestamp -PathToGradlewBat $PathToGradlewBat -PathToAdb $PathToAdb
}

if ($Platforms -eq "All" -or $Platforms -eq "Windows")
{
    & $PSScriptRoot\windows.ps1 -Operation $Operation -SampleToRun $SampleToRun -Timestamp $Timestamp -PathToCmake $PathToCmake
}