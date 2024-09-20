$venvPath = "$PSScriptRoot/.plotVenv"
if (-not (Test-Path $venvPath))
{
    Write-Host "Virtual Environment not found. Creating it..." -ForegroundColor Cyan
    py.exe -m venv $venvPath
    Write-Host "Virtual Environment created!" -ForegroundColor Green
}
else
{
    Write-Host "Virtual Environment already existed!" -ForegroundColor Green
}

Write-Host "Activating Virtual Environment..." -ForegroundColor Cyan
& $venvPath/scripts/activate.ps1
Write-Host "Virtual Environment activated!" -ForegroundColor Green

Write-Host "Installing dependencies" -ForegroundColor Cyan
python.exe -m pip install -r $PSScriptRoot/requirements.txt
Write-Host "Dependencies installed successfully!" -ForegroundColor Green