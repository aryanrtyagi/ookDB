
$ErrorActionPreference = "Stop"

$root = $PSScriptRoot
$build = Join-Path $root "build"

$vcvars = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

$pythonInclude = python -c "import sysconfig; print(sysconfig.get_path('include'))"
$pybindInclude = python -c "import pybind11; print(pybind11.get_include())"
$pythonLib = python -c "import sys; print(sys.base_prefix + r'\libs')"

$output = Join-Path $build "arydb.cp315-win_amd64.pyd"
$bindings = Join-Path $root "bindings\python\bindings.cpp"

if (!(Test-Path $vcvars)) {
    throw "MSVC vcvars64.bat not found."
}

if (!(Test-Path $bindings)) {
    throw "Python bindings source not found."
}

# Read sources, excluding the CLI REPL
$sources = Get-Content (Join-Path $root "cpp_files.txt") |
    ForEach-Object { $_.Trim().Trim('"') } |
    Where-Object { $_ -and !( $_ -match '[\\/]cli[\\/]repl[\\/]repl\.cpp$' ) }

$sources += $bindings

foreach ($source in $sources) {
    if (!(Test-Path $source)) {
        throw "Source file not found: $source"
    }
}

New-Item -ItemType Directory -Force -Path $build | Out-Null

Write-Host "Building AryDB Python extension..." -ForegroundColor Cyan
Write-Host "C++ source files: $($sources.Count)"

$arguments = @(
    "/nologo",
    "/std:c++17",
    "/EHsc",
    "/O2",
    "/MD",
    "/LD",
    "/I`"$root`"",
    "/I`"$root\src`"",
    "/I`"$pythonInclude`"",
    "/I`"$pybindInclude`""
)

$arguments += $sources | ForEach-Object { "`"$_`"" }
$arguments += "/link"
$arguments += "/LIBPATH:`"$pythonLib`""
$arguments += "python315.lib"
$arguments += "/OUT:`"$output`""

$clCommand = "cl.exe " + ($arguments -join " ")
$command = "call `"$vcvars`" && $clCommand"

Push-Location $root

try {
    & cmd.exe /c $command

    if ($LASTEXITCODE -ne 0) {
        throw "C++ compilation failed with exit code $LASTEXITCODE"
    }

    if (!(Test-Path $output)) {
        throw "Build completed but the .pyd output was not found."
    }

    Write-Host "AryDB extension built successfully!" -ForegroundColor Green
    Write-Host $output
}
finally {
    Pop-Location
}
