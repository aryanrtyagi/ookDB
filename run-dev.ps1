
$root = $PSScriptRoot
$backend = Join-Path $root "backend"
$frontend = Join-Path $root "frontend"

Write-Host "Starting AryDB development environment..." -ForegroundColor Cyan

# Start Django backend
Start-Process powershell -ArgumentList @(
    "-NoExit",
    "-Command",
    "Set-Location '$backend'; & '.\.venv\Scripts\Activate.ps1'; python manage.py runserver"
)

# Start React frontend
Start-Process powershell -ArgumentList @(
    "-NoExit",
    "-Command",
    "Set-Location '$frontend'; npm run dev"
)

Write-Host "Django and React startup commands launched." -ForegroundColor Green
