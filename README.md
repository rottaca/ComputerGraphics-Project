# [CG1 Framework](https://bitbucket.org/diemysh/cgi_framework)

## Summary
CG1 Framework is a framework for OpenGL used in the Computer Graphics 1 lecture at Ulm University. It is loosely based on [Glitter](https://github.com/Polytonic/Glitter). It compiles and statically links every required library.

## Getting Started
This framework needs [cmake](http://www.cmake.org/download/) to create platform-specific makefiles or project files. To generate a project file or makefile for your platform you need the IDE installed that you want to use. Don't forget to set the Start-Up Project in Visual Studio or the Target in Xcode.

```bash
# UNIX Makefile
cmake ..

# Mac OSX
cmake -G "Xcode" ..

# Microsoft Windows
cmake -G "Visual Studio 14" ..
cmake -G "Visual Studio 14 Win64" ..
...
```

## Dependencies
CG1 Framework uses the following dependencies (already included in the source package as git submodules).

Functionality             | Library
------------------------- | ------------------------------------------
OpenGL Function Loader    | [glad](https://github.com/Dav1dde/glad)
Windowing and Input       | [glfw](https://github.com/glfw/glfw)
OpenGL Mathematics        | [glm](https://github.com/g-truc/glm)
Texture Loading           | [stb](https://github.com/nothings/stb)
Model Loader              | [assimp](https://github.com/assimp/assimp)
GUI                       | [imgui](https://github.com/ocornut/imgui)

To pull all submodules from remote do

```git submodule update --init
```

or use the `--recursive` when cloning the repository.
