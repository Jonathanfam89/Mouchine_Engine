# Moustach've Been The Wind

A single-player 2D speedrunning platformer. You play as Mr. Mouch, a grumpy barber racing to fix haircut disasters across fast, precision-heavy levels by chaining movement abilities together: jumps, glides, wall kicks, ziplines, ground pounds and rolls.

The game runs on a custom C++20 engine written from scratch by team **AvidMoustacheEnjoyers** for DigiPen Institute of Technology Singapore's GAM200 module (CSD2401 / UXG2400 / DAA2402, Fall 2025).

---

## Engine Features

| System | What it does |
| --- | --- |
| **Core / Factory** | Game object and component architecture, message passing between systems, layer management, JSON prefabs and scenes |
| **Graphics** | OpenGL renderer (GLFW + GLEW), sprite sheet animation, particle emitters, FreeType text rendering, screen transitions, vignette and debug shaders, parallax backgrounds |
| **Physics & Collision** | Rigid body physics with forces, collider components, spatial grid broad phase |
| **Scripting** | Embedded Mono runtime. Gameplay code is written in C# (`Assets/Scripts`) and compiled to `GameScripts.dll`, which the engine loads at runtime |
| **Audio** | FMOD Core + Studio, with per-category volume settings |
| **Level Editor** | ImGui docking editor with ImGuizmo transform gizmos, a tilemap editor, an asset browser and a build size analyzer |
| **Replay** | Records runs, plays back a ghost of your best run alongside you, and has a replay viewer with scrubbing |
| **Cutscenes** | MPEG video playback via `pl_mpeg` with synced audio |
| **Input** | Keyboard, mouse and gamepad, with in-game controller connect/disconnect notifications |

---

## Controls

### Gameplay

| Action | Keyboard | Gamepad |
| --- | --- | --- |
| Move | `A` / `D` | Left stick / D-pad |
| Jump | `Space` | `A` |
| Glide (hold, in the air) | `W` or `Space` | `A` |
| Fast drop | `S` (hold) | `X` |
| Run | `J` (hold) | `R3` |
| Wall kick / Zipline | `L` | `LB` |
| Ground pound / Roll | `K` | `B` |
| Pause menu | `Esc` | `Start` |
| Toggle fullscreen | `Alt` + `Enter` | |

### Debug and Editor

| Key | Action |
| --- | --- |
| `F1` | Toggle debug drawing |
| `F2` | Toggle step-by-step mode |
| `F3` | Advance one frame (while in step mode) |
| `F4` | Toggle FPS counter |
| `F6` | Replay your best run as a ghost alongside the player |
| `F10` | Enter replay watch mode |
| `F7` / `F8` / `F9` | Rewind 10s / Pause-resume / Forward 10s (in replay mode) |
| `M` | Pause background music |
| `Ctrl` + `T` | Toggle tilemap visibility |
| `P` | Toggle the ImGui editor (outside play mode) |
| Left click | Interact with ImGui windows |

A full walkthrough of the editor is in [`CSD2401_GAM_200/EditorGuide.pdf`](CSD2401_GAM_200/EditorGuide.pdf).

---

## Building

The project is Windows-only and builds with CMake + Visual Studio 2022.

### Prerequisites

- Windows 10/11 (x64)
- [Visual Studio 2022](https://visualstudio.microsoft.com/) with these workloads:
  - **Desktop development with C++**
  - **.NET desktop development** (the C# scripts target .NET Framework 4.8)
- [CMake](https://cmake.org/download/) 3.15 or newer, on your `PATH`
- [Git](https://git-scm.com/install/), which CMake uses to fetch dependencies

FMOD and Mono are already bundled in `CSD2401_GAM_200/Extern`, so you don't need to install them separately.

### Quick build

```bat
git clone <this-repo-url>
cd csd2401f25_avidmoustacheenjoyers\CSD2401_GAM_200
run.bat
```

`run.bat` checks for CMake and the MSVC tools, wipes any previous `Build/` folder, generates a Visual Studio 2022 x64 solution and builds the Debug configuration.

### Manual build

```bat
cd CSD2401_GAM_200
cmake -S . -B Build -G "Visual Studio 18 2026" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build Build --config Debug --parallel
```

The first configure takes a while because CMake downloads GLFW, GLEW, GLM, ImGui, ImGuizmo, FreeType, nlohmann/json and stb.

### Running

1. Open `CSD2401_GAM_200/Build/GAM_200_AME.sln` in Visual Studio.
2. `Game` is already set as the startup project. Press **F5**.

Launch the game from Visual Studio rather than double-clicking `Game.exe` in `Build/bin/Debug`. Asset paths are resolved relative to the `Build/` working directory, so running the exe from another folder won't find `Assets/`.

Release builds run without a console window. Debug builds keep the console open for logs.

### Editing C# scripts

Gameplay scripts live in `CSD2401_GAM_200/Assets/Scripts`. The `GameScript.csproj` project is included in the solution, so editing a `.cs` file and rebuilding is enough: the post-build step copies the new `GameScripts.dll` next to the executable.

### Cleaning

Run `clean.bat` to delete the `Build/` folder.

> **Note:** `engine_installer.bat` is for setting up a fresh project from the engine. It overwrites `Game/main.cpp` and `Game/main.h` with the blank templates in `Logs/default/`, so don't run it on this repo unless that's what you want.

---

## Project Structure

```
CSD2401_GAM_200/
├── Assets/            Game content
│   ├── Audio/         Sound effects, music, audio settings
│   ├── Prefabs/       JSON prefab definitions
│   ├── Replay/        Saved replay data per level
│   ├── Scenes/        Level and menu scenes (JSON)
│   ├── Scripts/       C# gameplay scripts (player movement, pickups, ziplines, parallax...)
│   ├── Sprites/       UI screens, icons, props, sprite sheets
│   └── Tiles/         Tilesets and .tilemap files
├── Engine/            Engine source (static library)
│   ├── Audio/  Camera/  Collision/  Core/  Factory/  Graphics/
│   ├── Input/  LevelEditor/  Logic/  Physics/  Replay/
│   └── ResourceManager/  Scripting/  Serialization/  Transform/  UI/
├── Game/              Game executable: entry point, game states, menus, timers, win/lose logic
├── Scripts/           C# project that compiles Assets/Scripts into GameScripts.dll
├── Shaders/           GLSL vertex and fragment shaders
├── Extern/            Bundled third-party SDKs (FMOD, Mono)
├── cmake/             Dependency import module (FetchContent)
├── Logs/              Runtime config, editor layout, crash logs, project templates
└── CMakeLists.txt
```

---

## Tech Stack

| Library | Version | Used for |
| --- | --- | --- |
| [GLFW](https://github.com/glfw/glfw) | 3.4 | Windowing and input |
| [GLEW](https://github.com/Perlmint/glew-cmake) | 2.2.0 | OpenGL loading |
| [GLM](https://github.com/g-truc/glm) | 1.0.1 | Math |
| [Dear ImGui](https://github.com/ocornut/imgui) | 1.92.4 (docking) | Editor UI |
| [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) | master | Transform gizmos |
| [FreeType](https://github.com/freetype/freetype) | 2.13.2 | Font rendering |
| [nlohmann/json](https://github.com/nlohmann/json) | 3.11.3 | Serialization |
| [stb_image](https://github.com/nothings/stb) | master | Image loading |
| [pl_mpeg](https://github.com/phoboslab/pl_mpeg) | bundled | Cutscene video |
| [FMOD](https://www.fmod.com/) | bundled | Audio |
| [Mono](https://www.mono-project.com/) | bundled | C# scripting runtime |

---

## Team AvidMoustacheEnjoyers

| Name | Role |
| --- | --- |
| Jonathan Fam Qian Wei | Technical Lead, Programmer, Engine Champion |
| Guan Shao Jun | Programmer, Graphics Champion |
| Lee Hwee Min | Programmer, Input Champion |
| Khalil Belabadia | Programmer, Designer, Physics Champion |
| Leck Xin Yee | Programmer, Designer, Level Design Champion |
| Muhammad Ashfer Danishkhan Bin Afzalkhan | Programmer, Design Lead, Systems Designer Champion |
| Joann Bisseling | Product Manager, Audio Lead, Product Champion |
| Li Runtian | Art Lead, Art Champion |

---

## License

Copyright © 2025 DigiPen Institute of Technology. All rights reserved.

This project was made as part of DigiPen coursework. Reproduction or disclosure of its contents without the prior written consent of DigiPen Institute of Technology is prohibited.
