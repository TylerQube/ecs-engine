# Building a Game from “Scratch”

A C++ OpenGL renderer and ECS engine built to explore the underlying systems that get abstracted away by most modern tools.

Essentially a code playground for me to experiment with fun tech in an engine context.

I worked my way from opening a window and drawing a triangle on the screen (with a lot of help from learnopengl.com), to implementing a data-driven ECS system and procedural map generator for a demo game.

Each solution raised more questions, leading me down even more rabbit holes. 

- The limitations of OOP inheritance led me to a data-driven post via this [awesome blog](https://austinmorlan.com/posts/entity_component_system/) by Austin Morlan.
- My love for procedurally-generated maps (and the dread of building a level editor) led me to use Binary Space Partitioning to quickly spin up levels for a first-person game.

## What I’ve had fun implementing!

- 3D renderer and shader system
- Entity-Component-System architecture (love this for quickly adding new behavior)
- Skeletal animation (supports FBX thanks to ASSIMP)
- First-person player controller
- Sphere-triangle collision
- Procedural level generator

## What I’ll try next?
- UI & Audio
- Better visual debugging at runtime
- Multiplayer! (Exploring UDP in action, authoritative server, client prediction, cloud-compatibility?)
