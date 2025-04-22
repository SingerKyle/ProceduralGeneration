# Grammar's Edge
## A Mirror's Edge Based Grammar Generator

![Unreal Engine 5.4.4](https://img.shields.io/badge/Engine-Unreal%205.4.4-5c7ec2?logo=unrealengine&logoColor=white)
![Language: C++](https://img.shields.io/badge/Language-C%2B%2B-00599C?logo=c%2B%2B&logoColor=white)
![Project Type: Procedural Generation](https://img.shields.io/badge/Project%20Type-Procedural%20Generation-blueviolet)
![Status: WIP](https://img.shields.io/badge/Status-Work%20in%20Progress-yellow)


---

## 📖 Overview

This project is a **Procedural Level Generation Tool** created in **Unreal Engine 5.4.4**.  
It uses **grammar-based rules** to dynamically generate chains of platforms with traversal mechanics like:

- Small and long jumps
- Climbing (mantling) high/low points
- Wall running
- Vaulting

Each level should feel *handcrafted* but is generated **entirely procedurally**, based on a set of structured rules.

---

## 🎯 Project Aims

- Investigate different PCG techniques and identify the most effective for parkour-based gameplay.
- Provide designers a flexible system to control traversal paths, platform size, spacing, and movement types.
- Explore how different traversal obstacles can be organically incorporated into generation.
- Support **both in-editor generation** and **runtime generation**.

---

## 🎮 Controls

| Action  | Input |
|:-------:|:-----:|
| Jump | Spacebar |
| Vault / Mantle | Automatic when close to an obstacle |
| Wallrun | Automatic on approach to a wallrun wall |
| Move | WASD |
| Sprint | Shift |

---

## ⚙️ Key Components and Variables

### 🏗️ Grammar System
- **GrammarRules** map categories (jump, climb, wallrun) into expansion rules.
- Recursive expansion generates **platform chains**.

### 📐 Important Structs

- **FPlatformCalculations**  
  Calculates dimensions and positioning of each platform.

- **FPlatformEdges**  
  Stores the 3D coordinates (corners, center) of a platform for connecting and obstacle spawning.

- **FGrammarRules**  
  Designer-tweakable settings that define how the generation behaves:
  - **NumPlatforms**: How many platforms to generate.
  - **PlatformScale**: Min size of a generated platform.
  - **GridUnit**: How much snapping happens (default 50 units).
  - **Obstacle settings**: Meshes and distances for vaults, walls, mantling.

- **FDecorateLevelRules**  
  - **NumLayers**: How many vertical layers of "city blocks."
  - **LayerSpacing**: How far apart vertical layers are.

### 🔢 Placement Categories

Platforms can be placed:
- **Beside (Horizontal):** Forward, Left, Right, Back
- **Small Jump:** Forward, Left, Right, Back
- **Long Jump:** Forward, Left, Right, Back
- **Above or Below:** Forward, Left, Right, Back
- **Very High/Very Low Points:** Dropdown points / mantle climbs

Each type changes how the player needs to move: jump, climb, or wallrun.

---

## 🛠️ Level Generation Process

1. **Start Platform:**  
   Spawns an initial platform with a special material.

2. **Expansion:**  
   A chain of platforms is recursively spawned by picking grammar rules.

3. **Obstacle Spawning:**  
   Mantle walls, wallrun walls, and vault points are spawned between or onto platforms based on traversal type.

4. **Finish Platform:**  
   Marks the end of the generated path. Spawns decorative pieces and buildings in the backdrop

---

## 🔧 Editor Settings (Exposed to Blueprint)

| Setting | Purpose |
|:-------:|:-------:|
| NumPlatforms | Total platforms spawned |
| PlatformScale | Size range of each platform |
| GridUnit | Snap precision |
| SpawnHeight | Initial spawn height for platforms |
| SmallJumpMinimum/Maximum | Distance limits for small jumps |
| LongJumpMinimum/Maximum | Distance limits for long jumps |
| Above/Below Gap/Height Min/Max | Height/distance rules for climbing up/down |

---

## 🧠 Future Improvements

- Add "dead end" generation (optional toggle).
- Smoother wallrun transitions.
- More dynamic heights for flat level spaces
- Special traversal points like ziplines, multiple floors
- Better visual indicators for platform types (e.g., color coding jumps vs vaults).

---
