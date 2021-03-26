
cd out\build\x64-Debug\tests
OpenCPPCoverage --source G:\Repos\grad_aff\ grad_aff_tests_paa.exe --excluded_sources *tests.cpp  --stop_on_assert --excluded_line_regex ".*@NOCOVERAGE.*" --excluded_line_regex "\s*\}.*" --excluded_line_regex "\s*else.*"
cd ..\..\..\..\
pause