$paths = @(
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime.TestForDriver\TestForDriver.Main.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.28000.0\ucrt\internal\kernel_initializers.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.28000.0\ucrt\internal\charconv.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.28000.0\ucrt\internal\winapi_thunks.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.28000.0\ucrt\locale\nlsdata.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.28000.0\ucrt\convert\_ctype.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.26100.0\ucrt\internal\kernel_initializers.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.26100.0\ucrt\internal\charconv.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.26100.0\ucrt\internal\winapi_thunks.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.26100.0\ucrt\locale\nlsdata.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.26100.0\ucrt\convert\_ctype.cpp'
)

foreach ($path in $paths) {
    if (Test-Path $path) {
        $content = [System.IO.File]::ReadAllText($path)
        $utf8BOM = New-Object System.Text.UTF8Encoding($true)
        [System.IO.File]::WriteAllText($path, $content, $utf8BOM)
        Write-Host "BOM: $(Split-Path $path -Leaf)"
    }
}
