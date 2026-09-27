# Solis Releases

Solis is packaged as a Windows Electron application with electron-builder.

## Release workflow

The release workflow is:

`.github/workflows/release.yml`

It can run in two ways:

- Push a tag matching `v*.*.*`
- Manually use GitHub Actions → Release Solis → Run workflow

The workflow installs the Node dependencies, builds the Windows NSIS installer with electron-builder, and publishes the release to the `solis-syst/solis-app-updates` GitHub Releases.

## Auto-update

The packaged application uses `electron-updater` and checks the GitHub Releases feed for `solis-syst/solis-app-updates`.

When a newer release is available, the updater downloads it in the background and can install it on application restart.

## Versioning

Update the `version` field in `package.json` for a new application release.

Example:

`0.1.1`

For tag-based releases, create the matching tag:

`v0.1.1`


## Update repository

The release/update repository is `solis-syst/solis-app-updates`. It is separate from the source repository `solis-syst/solis-app`.

Expected Windows release assets:
- `latest.yml`
- Windows NSIS installer
- Windows installer `.blockmap`

## Publishing token

GitHub Actions uses the repository secret `SOLIS_RELEASE_TOKEN` and exposes it to electron-builder as `GITHUB_RELEASE_TOKEN`. Never place the token in the packaged application.

## Differential updates

NSIS differential updates are enabled with `differentialPackage: true`. If differential data cannot be used, electron-updater can fall back to the full installer.

Solis is currently unsigned, so Windows SmartScreen warnings are expected. Update code-signature verification remains disabled until Authenticode signing is available.
