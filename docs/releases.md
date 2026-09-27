# Solis Releases

Solis is packaged as a Windows Electron application with electron-builder.

## Release workflow

The release workflow is:

`.github/workflows/release.yml`

It can run in two ways:

- Push a tag matching `v*.*.*`
- Manually use GitHub Actions → Release Solis → Run workflow

The workflow installs the Node dependencies, builds the Windows NSIS installer with electron-builder, and publishes the release to the `solis-syst/solis-app` GitHub Releases.

## Auto-update

The packaged application uses `electron-updater` and checks the GitHub Releases feed for `solis-syst/solis-app`.

When a newer release is available, the updater downloads it in the background and can install it on application restart.

## Versioning

Update the `version` field in `package.json` for a new application release.

Example:

`0.1.1`

For tag-based releases, create the matching tag:

`v0.1.1`
