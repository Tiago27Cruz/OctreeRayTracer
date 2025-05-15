# Simple RayTracing Algorithm using Octrees

Projected developed for EDAA class @ FEUP - M.EIC

## Compile

- Run in root ``cmake -G "Unix Makefiles" -S . -B build``

If it doesn't work add the direct paths to GCC and G++, i.e.
-  ``-DCMAKE_C_COMPILER="C:/msys64/ucrt64/bin/gcc.exe" -DCMAKE_CXX_COMPILER="C:/msys64/ucrt64/bin/g++.exe"``

Then inside ``/build/`` run ```make```

## Architecture

Our code will be divided in two parts, the CPU, which will do the work that happens once and the Shaders, which does things that need to be done many times per second.
This way, we ease the load of work on the GPU, making our app more efficient.

- CPU (normal C/C++ files): Build scene -> Build octree -> Send both to GPU
- GPU (shaders): For each ray -> Traverse octree -> Test only relevant spheres

## Notes 

1- Install something for cpp and make like Mingw
2- gcc-g++, binutils, gdb, libstdc++
3- "terminal.integrated.env.windows": {
        "Path": "C:\\cygwin64\\bin;${env:Path}"
    } in settings.json
4-install Cmake

## References

- https://www.cs.swarthmore.edu/~jcarste1/pdfs/octree.pdf
- http://wscg.zcu.cz/wscg2000/Papers_2000/X31.pdf
- https://www.geeksforgeeks.org/octree-insertion-and-searching/
