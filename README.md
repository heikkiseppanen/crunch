# Crunch
A rendering-/game-engine built with C/C++17 for educational purposes. The project is in initial stages and current main goal is prototyping a framework for Vulkan components and pipelines.

# Dependencies
- CMake

## Included Libraries
- GLM
- Vulkan Memory Allocator
- GLFW

# Building
The project is maintained and has confirmed functionality only on Linux(x86-64) with GTX 1080 at the moment.

## Linux (Make)
```
cmake -S ./ -B ./Build
make -C ./Build
cp -r ./Assets ./Build
```
