$paths = @(
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime.TestForDriver\TestForDriver.Main.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.28000.0\ucrt\internal\kernel_initializers.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.28000.0\ucrt\internal\charconv.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.28000.0\ucrt\internal\winapi_thunks.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.28000.0\ucrt\locale\nlsdata.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.28000.0\ucrt\convert\_ctype.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.28000.0\ucrt\stdio\output.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.26100.0\ucrt\internal\kernel_initializers.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.26100.0\ucrt\internal\charconv.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.26100.0\ucrt\internal\winapi_thunks.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.26100.0\ucrt\locale\nlsdata.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.26100.0\ucrt\convert\_ctype.cpp',
    'D:\DevHub\Code\Musa.Runtime\Musa.Runtime\UCRT\10.0.26100.0\ucrt\stdio\output.cpp'
)

foreach ($p in $paths) {
    if (Test-Path $p) {
        $c = [System.IO.File]::ReadAllText($p, [System.Text.Encoding]::UTF8)
        $b = New-Object System.Text.UTF8Encoding($true)
        [System.IO.File]::WriteAllText($p, $c, $b)
        Write-Host "BOM: $(Split-Path $p -Leaf)"
    }
}
