# Solis Minecraft Client

## Locked baseline

- Minecraft Java Edition 26.3
- Fabric Loader 0.19.5
- Fabric Loom 1.17
- Java 25
- Fabric API 0.161.0+26.3
- Mojang mappings, not Yarn

## Architecture

The repository root is the Minecraft client project.

- com.solis.SolisClient is the Fabric client entrypoint.
- com.solis.SolisMod owns client initialization and the module registry.
- module/ contains the module base, registry, categories, and feature packages.
- gui/ contains Click GUI, HUD editor, and HUD widgets.
- event/ contains the event bus and listeners.
- mixin/ contains client mixins.
- setting/ contains reusable setting types.
- util/ contains client utility classes.
- src/main/resources/ contains Fabric metadata and mixin configuration.

Feature classes are initially structural stubs. Their gameplay behavior will be implemented incrementally without changing the project architecture.

## Mapping decision

Minecraft 26.3 uses the official Mojang mapping workflow. Yarn is not used for this target.

## First milestone

Keep the 26.3 Fabric client buildable and initialize the module architecture cleanly. Feature implementation comes after the foundation is verified.
