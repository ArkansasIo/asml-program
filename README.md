# Lithography Control Studio — C++23 Foundation

Windows-first desktop engineering and simulation project.

## Included in this repository
- C++23 CMake foundation (CLI and desktop-shell entry points)
- Virtual machine state model and engineering mathematics starter
- Configuration files and MySQL schema
- CSS command-center layout and dark industrial theme
- UML diagrams and hardware/software schematic sources
- ChipLab architecture and toolchain design documents
- Plugin and update/safety design notes

## Build on Windows
Install Visual Studio 2022 with the C++ desktop workload, CMake 3.25+, and Ninja.

```powershell
cmake --preset windows-msvc-release
cmake --build build/release
ctest --test-dir build/release --output-on-failure
```

## Scope and limitations
This is a starter foundation, not a completed commercial application. The desktop target is currently a console shell; the native GUI, live MySQL connector, production plugin loader, advanced simulator physics, graphing, and ChipLab toolchain require further implementation. The project has not been verified in a Windows build environment.

Simulation is the default. Physical equipment adapters must use authorized, documented vendor interfaces and preserve equipment-native safety interlocks.

The downloadable ZIP package was created in the originating ChatGPT conversation; the current GitHub contents API only supports UTF-8 text files, so this README does not pretend the binary ZIP was uploaded.
