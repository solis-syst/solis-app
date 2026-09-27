# Solis Releases

## Repositories

`solis-syst/solis-app` contains the application source and Windows release workflow.
`solis-syst/solis-app-updates` is the public GitHub Releases feed used by the native updater.

## Release workflow

Push a semantic version tag to `solis-syst/solis-app`:

`v1.0.1`

The workflow builds the Windows Release configuration, creates the NSIS installer, publishes the application executable, calculates SHA-256 checksums, and creates a binary delta from the previous stable application executable when the delta is smaller than the new executable.

The workflow uses the Windows MSDelta API through the Solis updater helper to create the binary delta. The installed Solis application uses the same helper to apply it.

## Required repository secret

Add `SOLIS_RELEASE_TOKEN` under `solis-syst/solis-app` repository Actions secrets.

The token must be allowed to read and write Contents for the `solis-syst/solis-app-updates` repository. A fine-grained personal access token limited to that repository is preferred.

Do not store the token in the source tree, CMake files, or workflow YAML.

## Release assets

Each stable release contains:

- a `Solis*.exe` Windows NSIS installer
- its matching `.sha256` checksum
- a `SolisApp-<version>.exe` application executable
- its matching `.sha256` checksum

When a smaller differential update can be created from the previous stable release, it also contains:

- a `SolisDelta-<from>-to-<to>.delta` binary delta
- its matching `.sha256` checksum

The application updater first looks for a delta whose source version matches the installed version. It downloads only the delta and the small checksum files, reconstructs the new executable locally, verifies the reconstructed executable against the published application checksum, and then replaces the running executable through the installed `solis-updater.exe` helper.

If a matching delta is unavailable, larger than the full application binary, or cannot be applied, the updater falls back to the full NSIS installer.

Users who skip versions currently fall back to the full installer because each release creates a delta only from the immediately previous stable release.
