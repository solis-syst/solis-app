# Solis Releases

## Repositories

`solis-syst/solis-app` contains the application source and Windows release workflow.
`solis-syst/solis-app-updates` is the public GitHub Releases feed used by the native updater.

## Release workflow

Push a semantic version tag to `solis-syst/solis-app`:

```text
v1.0.1
```

The workflow builds the Windows Release configuration, creates the NSIS installer, calculates its SHA-256 checksum, and publishes both assets to `solis-syst/solis-app-updates`.

## Required repository secret

Add `SOLIS_RELEASE_TOKEN` under `solis-syst/solis-app` repository Actions secrets.

The token must be allowed to write Contents for the `solis-syst/solis-app-updates` repository. A fine-grained personal access token limited to that repository is preferred.

Do not store the token in the source tree, CMake files, or workflow YAML.

## Release assets

Each stable release should contain:

- a `Solis*.exe` Windows NSIS installer
- the matching `Solis*.exe.sha256` checksum

The native updater reads the latest published release from the update repository, downloads the installer and checksum, verifies SHA-256, and launches the installer only after verification succeeds.
