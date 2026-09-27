# Solis Project Specification


Date: 2026-09-27

This file is the source of truth for Solis architecture, repository structure, runtime boundaries, engine inventory, and locked decisions. New changes must be checked against this document before implementation.

## 1. Project

Solis is a Windows desktop chess analyzer built fully with Electron.

Primary goals:
- Run chess analysis locally on the user's PC.
- Open Chess.com and Lichess inside a managed Electron browser window.
- Inject the existing analyzer into supported chess pages.
- Keep engines and required model/assets inside the application.
- Provide analyzer results and settings through the Solis desktop UI.
- Support automatic updates through electron-updater and electron-builder.

Current package versions:
- Electron: 44.4.4
- electron-builder: 26.16.1
- electron-updater: 6.8.9

Target platform:
- Windows only
- Current configured target: NSIS

## 2. Locked Architecture Decisions

These decisions are part of the current Solis direction.

- Electron is the application runtime.
- Do not reintroduce Go as the Solis architecture.
- Do not reintroduce the previous C++/Qt architecture.
- Do not add a localhost backend for analyzer communication.
- Use Electron IPC and the existing page bridge for app/page communication.
- Use the `solis://` custom protocol for packaged local asset loading.
- Keep the analyzer and engine assets local.
- Do not require users to manually download engines.
- Preserve the existing `engines/` assets unless a file is explicitly confirmed obsolete or is an obvious placeholder.
- Changes should be minimal and scoped to the requested component.
- Do not perform unrelated refactors while implementing a feature.
- Before changing a shared interface, inspect its current callers and consumers.
- When a new architecture decision is made, update this file.

## 3. Current Repository Structure

```text
solis-app/
├── .github/
│   └── workflows/
│       └── release.yml
├── electron/
│   ├── main.js
│   ├── page-bridge.js
│   ├── preload.js
│   └── updater.js
├── renderer/
│   ├── index.html
│   ├── renderer.js
│   └── styles.css
├── engines/
│   ├── chess_min.js
│   ├── engine.js
│   ├── komodo.js
│   ├── komodo.wasm
│   ├── lozza.js
│   ├── maia3/
│   │   ├── all_moves.json
│   │   ├── all_moves_reversed.json
│   │   ├── maia3-5m.onnx
│   │   ├── maia3-engine.js
│   │   ├── maia3-tokenizer.js
│   │   └── maia3-worker.js
│   ├── ort/
│   │   ├── ort-wasm-simd-threaded.wasm
│   │   ├── ort-wasm-simd.wasm
│   │   ├── ort-wasm-threaded.js
│   │   ├── ort-wasm-threaded.wasm
│   │   ├── ort-wasm-threaded.worker.js
│   │   ├── ort-wasm.wasm
│   │   ├── ort.min.js
│   │   └── ort.wasm.min.js
│   ├── stockfish11.js
│   ├── stockfish6.js
│   ├── torch.js
│   ├── torch.wasm
│   └── wukong.js
├── utils/
│   ├── config.js
│   ├── const.js
│   ├── draw.js
│   ├── engines.js
│   ├── func.js
│   └── no
├── book/
│   ├── book.bin
│   └── no
├── a.js
├── alert.js
├── content.js
├── package.json
├── README.md
└── docs/
    └── releases.md
```

Notes:
- `engines/engine.js` is present but currently contains only the text `this is not needed, delete this`. It is not part of the active engine inventory and should be treated as a confirmed placeholder.
- `utils/no` and `book/no` also exist and contain placeholder-style content. They are outside the `engines/` directory and have not been removed in this spec update.
- `utils/const.js` is a large generated/static data file and must not be rewritten casually.

## 4. Electron Runtime Architecture

### Main process

File:
`electron/main.js`

Responsibilities:
- Create the main Solis window.
- Create the managed browser window.
- Restrict browser destinations to supported chess sites.
- Inject the Solis bridge and analyzer files into supported pages.
- Serve packaged/local analyzer assets through the `solis://bundle/` protocol.
- Persist configuration in Electron `userData/config.json`.
- Handle IPC for browser control, configuration, page messages, and update installation.
- Forward analyzer messages to the Solis renderer.

Security settings currently used for Electron windows:
- `contextIsolation: true`
- `nodeIntegration: false`
- `sandbox: true`

### Renderer

Files:
- `renderer/index.html`
- `renderer/renderer.js`
- `renderer/styles.css`

Responsibilities:
- Solis desktop UI.
- Open Chess.com or Lichess through the exposed preload API.
- Display analyzer/update status.
- Trigger update installation through the preload API.

The current renderer is still a basic test/dashboard UI, not the final Solis interface.

### Preload

File:
`electron/preload.js`

Responsibilities:
- Expose the minimum Solis API to the renderer.
- Bridge browser opening/messages.
- Bridge configuration get/set/clear.
- Bridge update installation.
- Subscribe to analyzer and updater events.

### Page bridge

File:
`electron/page-bridge.js`

Responsibilities:
- Recreate the subset of Chrome extension APIs used by the legacy analyzer.
- Provide:
  - `chrome.storage.local.get/set/clear`
  - `chrome.storage.onChanged`
  - `chrome.runtime.id`
  - `chrome.runtime.getURL`
  - `chrome.runtime.sendMessage`
  - `chrome.runtime.onMessage`
- Translate page messages into Electron IPC requests.
- Receive Electron/main-process messages and expose them to injected analyzer code.

## 5. Managed Browser Flow

Current intended runtime flow:

```text
Solis UI
  ↓
preload
  ↓
main process
  ↓
Managed BrowserWindow
  ↓
Chess.com / Lichess
  ↓
page-bridge.js
  ↓
a.js + analyzer utilities + content.js
  ↓
Local engine/model workers
  ↓
Analyzer results
  ↓
page bridge
  ↓
Electron IPC
  ↓
Solis UI
```

Supported browser destinations currently accepted by `browser:open`:
- `https://www.chess.com`
- `https://lichess.org`

Page support checking also recognizes supported subdomains.

## 6. Local Asset Loading

Solis registers a privileged custom protocol:

`solis://bundle/<path>`

The main process:
- Resolves requested assets relative to `app.getAppPath()`.
- Rejects traversal outside the application root.
- Loads local files through Electron's network layer.

This is the current replacement for extension URLs such as `chrome-extension://...`.

The analyzer currently uses `chrome.runtime.getURL(...)`, which the page bridge maps to `solis://bundle/...`.

## 7. Analyzer Architecture

Primary analyzer file:
`content.js`

Supporting files:
- `a.js`
- `alert.js`
- `utils/const.js`
- `utils/config.js`
- `utils/engines.js`
- `utils/func.js`
- `utils/draw.js`
- `engines/chess_min.js`
- engine-specific assets under `engines/`

Current analyzer responsibilities include:
- Board/state detection.
- FEN generation/update handling.
- Move handling.
- Engine analysis.
- Evaluation/PV handling.
- SVG/arrows and overlay behavior.
- Hints.
- Accuracy/move classification behavior.
- Coach functionality.
- Chess.com/Lichess-specific behavior.

Important current implementation detail:
- Engine selection/instantiation is currently performed in `content.js`.
- Engine wrappers and worker logic are primarily in `utils/engines.js`.
- This is not yet a dedicated standalone Electron engine-manager service.

## 8. Engine Inventory

Current active engine/model families referenced by the analyzer:

### Komodo
Files:
- `engines/komodo.js`
- `engines/komodo.wasm`

Current usage:
- Main selectable engine.
- Config supports Elo, depth, MultiPV/lines, threads, hash, and personality/style.

### Stockfish 6
File:
- `engines/stockfish6.js`

Current usage:
- Selectable engine.
- Includes Stockfish-specific evaluation option configuration.

### Stockfish 11
File:
- `engines/stockfish11.js`

Current usage:
- Selectable engine.
- Uses UCI options such as MultiPV and Ponder.

### Lozza
File:
- `engines/lozza.js`

Current usage:
- Selectable engine.
- Runs through a Web Worker.

### Wukong
File:
- `engines/wukong.js`

Current usage:
- Selectable engine.
- Runs through a Web Worker.

### Maia 3
Directory:
- `engines/maia3/`

Required assets include:
- ONNX model
- move dictionaries
- tokenizer
- engine worker
- supporting engine code

Current usage:
- Selectable model/engine path.
- Uses ONNX Runtime assets from `engines/ort/`.

### Torch / Coach
Files:
- `engines/torch.js`
- `engines/torch.wasm`

Current usage:
- Used for coach-related functionality.
- Treat as a required runtime component unless code inspection proves otherwise.

### ONNX Runtime assets
Directory:
- `engines/ort/`

These are supporting runtime assets for Maia 3 and must remain available to the packaged application.

## 9. Engine Worker Rule

Engine/model execution should remain off the renderer's main UI thread whenever the existing implementation already uses a Worker.

Do not convert an engine from Worker execution to renderer/main-thread execution without a specific reason and explicit scope.

When adding or replacing an engine adapter:
1. Inspect the existing wrapper interface.
2. Match the existing promise/worker lifecycle.
3. Preserve current result shapes.
4. Verify startup, analysis, stop, and restart behavior.
5. Test packaging of all dependent JS/WASM/ONNX assets.

## 10. IPC Contract

Current main-process IPC handlers include:

- `browser:open`
- `browser:message`
- `config:get`
- `config:set`
- `config:clear`
- `browser:page-message`
- `update:install`

Current renderer event channels include:
- `analyzer:message`
- `update:status`

The page bridge uses `window.postMessage` for communication between injected page code and Electron preload/main.

Do not add direct Node/Electron access to the renderer. New renderer capabilities should be exposed through preload.

Future IPC changes must specify:
- sender
- receiver
- channel name
- payload shape
- response/error behavior
- validation requirements

## 11. Storage

Configuration is persisted as:

`<Electron userData>/config.json`

Current bridge behavior mirrors the subset of Chrome extension storage needed by the legacy analyzer.

Current analyzer config contains engine and UI settings such as:
- engine
- Elo
- depth
- lines/MultiPV
- thread/hash values
- display options
- coach settings
- classification/accuracy settings
- Stockfish 6-specific evaluation parameters

Do not silently rename existing config keys because the legacy analyzer reads them directly.

## 12. Updater Architecture

Current updater file:
`electron/updater.js`

Current package:
- `electron-updater` 6.8.9

Current behavior:
- Updater is initialized from the Electron main process.
- Checks for updates on app startup when packaged.
- Sends status events to the renderer.
- Automatically downloads an available update.
- Current code still has `autoInstallOnAppQuit = true`.
- Current code exposes an IPC path for installation.

Current update repository configuration is still:
`solis-syst/solis-app`

This is a current-state fact, not the final decision.

### Planned update repository

Target update repository:
`solis-syst/solis-app-updates`

Target release assets:
- `latest.yml`
- Windows NSIS installer
- Windows installer `.blockmap`

Target packaging decision:
- NSIS differential package enabled with `differentialPackage: true`.

Target token mapping:
- GitHub Actions secret/environment name: `SOLIS_RELEASE_TOKEN`
- electron-builder compatible publishing variable: `GITHUB_RELEASE_TOKEN`

Do not put the release token into the shipped application.

## 13. Release Workflow

Current workflow:
`.github/workflows/release.yml`

Current trigger modes:
- Push of a tag matching `v*.*.*`
- Manual workflow dispatch with a required `tag` input

Current workflow behavior:
1. Checkout the workflow ref.
2. Validate the release tag.
3. For manual runs, create and push the requested tag when it does not already exist.
4. Set the package version in the build workspace.
5. Install dependencies.
6. Run the electron-builder release script.

Current publishing environment:
`GH_TOKEN: \${{ secrets.GITHUB_TOKEN }}`

This is not yet the planned custom-token mapping.

Important consistency note:
- The current workflow creates the manual release tag before running `npm version` in the workspace.
- Therefore, the tag itself currently points at the checked-out source commit, while the build workspace receives the release version afterward.
- Do not silently redesign this release/version flow. Treat it as a separate release-workflow improvement.

## 14. Build Configuration

Current scripts:
- `npm start` → `electron .`
- `npm run dist` → Windows NSIS build with publishing disabled
- `npm run release` → Windows NSIS build with publishing enabled

Current electron-builder target:
- Windows
- NSIS

Planned update configuration:
- `win.verifyUpdateCodeSignature: true`
- NSIS `differentialPackage: true`
- GitHub publisher points to `solis-syst/solis-app-updates`
- Published release type is a normal published release, not a draft

Before packaging changes, verify that every required JS/WASM/ONNX/model asset is included in the final application.

## 15. Security Rules

Current Electron security baseline:
- `contextIsolation: true`
- `nodeIntegration: false`
- `sandbox: true`
- Renderer access to Electron APIs only through preload.
- Browser navigation is restricted to the supported chess sites for the Solis browser opener.
- Local asset protocol enforces path containment.

Security review items for future cross-cutting changes:
- Validate page-message origins and payload schemas.
- Avoid broad `postMessage("*")` patterns where a narrower origin/channel boundary can be used safely.
- Validate all renderer-to-main IPC inputs.
- Do not expose filesystem, process, shell, or arbitrary navigation capabilities directly to the renderer.

## 16. No-Localhost Decision

The current analyzer does not require a localhost API layer in the Solis repository.

Repository/runtime inspection found the analyzer using:
- local engine wrappers
- Web Workers
- the Chrome-API compatibility bridge
- Electron IPC

No internal Solis implementation currently depends on an external `127.0.0.1:5000` analyzer server.

Therefore:
- Do not restore the old localhost API architecture unless new repository evidence shows a required service that cannot reasonably be implemented through Electron/IPC.
- Use Electron IPC/page bridge for Solis-native communication.

## 17. Browser/Analyzer Injection Order

The current main process injects these files in this order:

1. `a.js`
2. `alert.js`
3. `engines/chess_min.js`
4. `engines/maia3/maia3-tokenizer.js`
5. `utils/const.js`
6. `utils/config.js`
7. `utils/engines.js`
8. `utils/func.js`
9. `utils/draw.js`
10. `content.js`

The page bridge is prepended before these analyzer files.

Do not change injection order without checking initialization dependencies.

## 18. Known Current Limitations

- Renderer UI is still a basic dashboard and is not the final Solis UI.
- There is no dedicated engine-manager abstraction yet; engine selection lives in the analyzer.
- Auto-update configuration still targets the main source repository.
- The workflow still uses the default GitHub token environment variable.
- Differential NSIS configuration has not yet been applied to the current `package.json`.
- Updater currently enables `autoInstallOnAppQuit`; the planned UX is to prompt before installation.
- `engines/engine.js` is a confirmed placeholder and should be removed in a scoped cleanup change.
- `utils/no` and `book/no` are still present.
- Current browser/page message handling should receive a dedicated security review before exposing more capabilities.

## 19. Change Rules

For every implementation request:

1. Inspect the relevant existing files first.
2. State assumptions that are not directly confirmed by the repository.
3. Identify the smallest change that satisfies the request.
4. Preserve existing interfaces unless the request requires a change.
5. Do not refactor unrelated code.
6. Do not add dependencies without a specific need.
7. Verify the changed behavior with the narrowest appropriate test/build.
8. Update this file when architecture, interfaces, engine inventory, packaging, or release decisions change.
9. Report exactly what changed and what was intentionally left untouched.

For engine work:
- One adapter/engine at a time when possible.
- Match the current engine wrapper contract.
- Preserve Worker behavior.
- Verify asset loading and packaging.

For Electron cross-cutting work:
- Check main/renderer/preload/page boundaries together.
- Treat IPC, custom-protocol asset loading, Worker execution, WASM/ONNX packaging, and updater behavior as high-risk areas.

## 20. Verification Checklist

Before considering a major Solis change complete:

### Application
- [ ] `npm install` succeeds
- [ ] `npm start` launches Solis
- [ ] Main renderer loads
- [ ] Chess.com opens in managed browser
- [ ] Lichess opens in managed browser

### Bridge
- [ ] `solis://bundle/` asset loading works
- [ ] Analyzer injection runs
- [ ] Chrome compatibility APIs used by the analyzer work
- [ ] Page messages reach Electron
- [ ] Analyzer messages reach the renderer

### Engines
- [ ] Selected engine initializes
- [ ] Worker/model assets load
- [ ] FEN analysis returns
- [ ] PV/move results return
- [ ] Stop/restart behavior works
- [ ] Required WASM/ONNX assets are packaged

### Packaging
- [ ] Windows NSIS installer builds
- [ ] Packaged app starts outside the development environment
- [ ] No missing asset/runtime DLL errors
- [ ] Required engine/model assets exist in the packaged application

### Updates
- [ ] `latest.yml` is published
- [ ] Windows installer is published
- [ ] `.blockmap` is published
- [ ] Packaged Solis checks for a newer version
- [ ] Download progress is reported
- [ ] User confirmation occurs before installation
- [ ] Differential download works when a usable previous blockmap is available
- [ ] Full-download fallback works when differential data cannot be used

## 21. Future Decision Log

Add entries here when an architecture or release decision becomes stable.

Format:

```text
YYYY-MM-DD
Decision:
Reason:
Affected files:
Status:
```
