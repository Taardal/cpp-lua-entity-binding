# C++ Lua Entity Binding

Proof-of-concept for binding Lua-scripts to entities in a C++ game.

**Problem**

Given a game written in C++ that manages entities in a Entity-Component-System (ECS), how do you attach Lua-scripts to each entity and make them communicate across the C++/Lua boundary?

**Scope**

1. Create and manage entities in C++
2. Call an entity's Lua lifecycle functions from C++ (onCreate, onUpdate, onDelete)
3. Access C++ objects and functions from Lua-scripts (get components, check keyboard press etc.)

# Prerequisites&nbsp;:vertical_traffic_light:

- [Git][git]
- [CMake][cmake]
- C++ compiler
    - Windows: [MSVC][msvc] (Bundled with [Visual Studio][msvs])
    - MacOS: [Clang][clang] (Bundled with [Xcode][xcode])
    - Linux: [GCC][gcc]

# Getting started&nbsp;:runner:

### Getting the code&nbsp;:octocat:

- Clone the repository: `git clone https://github.com/taardal/cpp-lua-entity-binding`

### Running the app&nbsp;:rocket:

**Script**

- Make the script executable: `chmod +x run.sh`
- Run the script: `./run.sh`

**Manual**

- Generate project files: `cmake -S . -B build`
- Build executable from generated files: `cmake --build build`
- Install executable: `cmake --install build` 
- Compile the Lua code: `luac src/*.lua`    
- Run the executable: `./app`

# Resources&nbsp;:books:

- [Embedding Lua in C++][davepoo:lua] (YouTube @ [Dave Poo][davepoo])
- [Embedding Lua in C++][javidx9:lua] (YouTube @ [javidx9][javidx9])
- [Entity Component System][thecherno:ecs] (YouTube @ [TheCherno][thecherno])


[clang]: http://clang.org/
[cmake]: https://cmake.org/
[davepoo]: https://www.youtube.com/@DavePoo
[davepoo:lua]: https://www.youtube.com/watch?v=xrLQ0OXfjaI&list=PLLwK93hM93Z3nhfJyRRWGRXHaXgNX0Itk&ab_channel=DavePoo
[gcc]: https://gcc.gnu.org/
[git]: https://git-scm.com
[javidx9]: https://www.youtube.com/@javidx9
[javidx9:lua]: https://www.youtube.com/watch?app=desktop&v=4l5HdmPoynw&t=0s&ab_channel=javidx9
[msvc]: https://visualstudio.microsoft.com/vs/features/cplusplus/
[msvs]: https://visualstudio.microsoft.com/
[thecherno]: https://www.youtube.com/@TheCherno
[thecherno:ecs]: https://www.youtube.com/watch?v=Z-CILn2w9K0&ab_channel=TheCherno
[xcode]: https://developer.apple.com/xcode/