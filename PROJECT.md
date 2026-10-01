# MC Client Project

## Locked baseline

- Minecraft Java Edition 26.3
- Fabric Loader 0.19.5
- Fabric Loom 1.18-SNAPSHOT from the current Fabric 26.3 example template
- Java 25
- Fabric API 0.161.0+26.3

## Architecture direction

The project is split conceptually into a launcher layer and a Minecraft client runtime. The launcher will own authentication, Java/runtime management, version manifests, installation state, updates, profiles, and launch arguments. The client runtime will own the in-game UI, HUD, rendering, modules, configuration, notifications, keybinds, and client-specific integrations.

The Minecraft runtime is currently a standalone Fabric project so its game-facing code stays isolated from launcher code.

## First milestone

Establish a clean Fabric 26.3 client runtime that launches, initializes the core runtime, and provides a stable base for the launcher and feature modules.

## Not locked yet

- Public client name
- Launcher UI framework
- Account backend
- Update distribution service
- Cosmetic service
- Optional performance/rendering stack
