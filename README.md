# Simple RayTracing Algorithm using Octrees

Projected developed for EDAA class @ FEUP - M.EIC

## Compile

- Run in root ``cmake -G "Unix Makefiles" -S . -B build``

If it doesn't work add
-  ``-DCMAKE_C_COMPILER="C:/msys64/ucrt64/bin/gcc.exe" -DCMAKE_CXX_COMPILER="C:/msys64/ucrt64/bin/g++.exe"``

Then inside ``/build/`` run
```make```

## Notes 

1- Install something for cpp and make like Mingw
2- gcc-g++, binutils, gdb, libstdc++
3- "terminal.integrated.env.windows": {
        "Path": "C:\\cygwin64\\bin;${env:Path}"
    } in settings.json
4-install Cmake
