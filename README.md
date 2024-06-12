<p align="center">
  <img width="258" height="258" src="https://raw.githubusercontent.com/heikkiseppanen/crunch/master/crunch.png">
</p>
<h1 align="center">
    Crunch
</h1>
Hobbyist rendering-/game-engine project built with C/C++20 for learning purposes. The project is in initial stages and current main goal is prototyping a framework for Vulkan components and pipelines.

Dependencies included.

# Building
The project is maintained and has confirmed functionality only on Linux(x86-64) with GTX 1080 at the moment. The project does include static Windows libraries but is untested at this moment.

## Linux (Make)
```
cmake -S ./ -B ./Build
make -C ./Build
cp -r ./Assets ./Build
```
