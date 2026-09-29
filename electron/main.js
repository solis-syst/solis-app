const { app, BrowserWindow, ipcMain, net, protocol, screen } = require("electron");
const { spawn } = require("node:child_process");
const fs = require("node:fs");
const path = require("node:path");
const { pathToFileURL } = require("node:url");
const { setupUpdater, installUpdate } = require("./updater");

protocol.registerSchemesAsPrivileged([
  {
    scheme: "solis",
    privileges: {
      standard: true,
      secure: true,
      supportFetchAPI: true,
      corsEnabled: true
    }
  }
]);

let mainWindow = null;
let browserWindow = null;
let cursorSidecar = null;

function getConfigPath() {
  return path.join(app.getPath("userData"), "config.json");
}

function readStorage() {
  try {
    return JSON.parse(fs.readFileSync(getConfigPath(), "utf8"));
  } catch {
    return {};
  }
}

function writeStorage(storage) {
  fs.mkdirSync(path.dirname(getConfigPath()), { recursive: true });
  fs.writeFileSync(getConfigPath(), JSON.stringify(storage, null, 2), "utf8");
}

function postPageMessage(message) {
  if (!browserWindow || browserWindow.isDestroyed()) return;
  const payload = JSON.stringify(message);
  browserWindow.webContents.executeJavaScript(
    `window.postMessage(${payload}, "*");`,
    true
  ).catch(() => {});
}

function broadcastStorageChange(changes) {
  postPageMessage({
    source: "solis-electron-main",
    message: {
      type: "storage:changed",
      changes,
      areaName: "local"
    }
  });
}

function sendAnalyzerMessage(message) {
  if (!mainWindow || mainWindow.isDestroyed()) return;
  mainWindow.webContents.send("analyzer:message", message);
}

function getCursorSidecarPath() {
  if (app.isPackaged) {
    return path.join(process.resourcesPath, "sidecar", "CursorSidecar.exe");
  }

  return path.join(__dirname, "../sidecar/publish/CursorSidecar.exe");
}

function startCursorSidecar() {
  if (cursorSidecar && !cursorSidecar.killed && cursorSidecar.stdin?.writable) {
    return cursorSidecar;
  }

  const exePath = getCursorSidecarPath();

  if (!fs.existsSync(exePath)) {
    console.error("Cursor sidecar not found:", exePath);
    return null;
  }

  const child = spawn(exePath, [], {
    stdio: ["pipe", "ignore", "pipe"],
    windowsHide: true
  });

  child.stderr.on("data", (data) => {
    console.error("Cursor sidecar error:", data.toString());
  });

  child.on("error", (error) => {
    console.error("Cursor sidecar process error:", error);
    if (cursorSidecar === child) cursorSidecar = null;
  });

  child.on("exit", () => {
    if (cursorSidecar === child) cursorSidecar = null;
  });

  cursorSidecar = child;
  return child;
}

function sendCursorCommand(command) {
  const child = startCursorSidecar();
  if (!child || !child.stdin?.writable) return false;

  try {
    child.stdin.write(JSON.stringify(command) + "\n");
    return true;
  } catch (error) {
    console.error("Cursor sidecar command failed:", error);
    return false;
  }
}

function pagePointToScreenPoint(x, y) {
  const contentBounds = browserWindow.getContentBounds();

  return screen.dipToScreenPoint({
    x: contentBounds.x + x,
    y: contentBounds.y + y
  });
}

function performPageMouseDrag(message) {
  if (!browserWindow || browserWindow.isDestroyed()) return false;

  const fromX = Number(message?.fromX);
  const fromY = Number(message?.fromY);
  const toX = Number(message?.toX);
  const toY = Number(message?.toY);

  if (![fromX, fromY, toX, toY].every(Number.isFinite)) return false;

  browserWindow.focus();

  const start = pagePointToScreenPoint(fromX, fromY);
  const end = pagePointToScreenPoint(toX, toY);
  const middle = {
    x: Math.round((start.x + end.x) / 2),
    y: Math.round((start.y + end.y) / 2)
  };

  if (!sendCursorCommand({
    action: "move",
    x: Math.round(start.x),
    y: Math.round(start.y)
  })) {
    return false;
  }

  sendCursorCommand({ action: "down" });
  sendCursorCommand({
    action: "move",
    x: middle.x,
    y: middle.y
  });
  sendCursorCommand({
    action: "move",
    x: Math.round(end.x),
    y: Math.round(end.y)
  });
  sendCursorCommand({ action: "up" });

  return true;
}

function createMainWindow() {
  mainWindow = new BrowserWindow({
    width: 1200,
    height: 760,
    minWidth: 960,
    minHeight: 620,
    webPreferences: {
      preload: path.join(__dirname, "preload.js"),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: true
    }
  });

  mainWindow.loadFile(path.join(__dirname, "../renderer/index.html"));

  mainWindow.on("closed", () => {
    mainWindow = null;
  });
}

function isSupportedSite(url) {
  try {
    const host = new URL(url).hostname;
    return [
      "chess.com",
      "www.chess.com",
      "lichess.org",
      "www.lichess.org"
    ].some((allowed) => host === allowed || host.endsWith("." + allowed));
  } catch {
    return false;
  }
}

function getPageBridge() {
  return fs.readFileSync(path.join(__dirname, "page-bridge.js"), "utf8");
}

async function injectSolis() {
  if (!browserWindow || browserWindow.isDestroyed()) return;

  const url = browserWindow.webContents.getURL();
  if (!isSupportedSite(url)) return;

  const files = [
    "a.js",
    "alert.js",
    "engines/chess_min.js",
    "engines/maia3/maia3-tokenizer.js",
    "utils/const.js",
    "utils/config.js",
    "utils/engines.js",
    "utils/func.js",
    "utils/draw.js",
    "content.js"
  ];

  try {
    const code = [
      getPageBridge(),
      ...files.map((file) =>
        fs.readFileSync(path.join(app.getAppPath(), file), "utf8")
      )
    ].join("\n;\n");

    await browserWindow.webContents.executeJavaScript(code, true);
  } catch (error) {
    console.error("Solis injection failed:", error);
  }
}

function createBrowserWindow(url) {
  if (browserWindow && !browserWindow.isDestroyed()) {
    browserWindow.focus();
    browserWindow.loadURL(url);
    return;
  }

  browserWindow = new BrowserWindow({
    width: 1400,
    height: 900,
    minWidth: 1000,
    minHeight: 700,
    webPreferences: {
      preload: path.join(__dirname, "preload.js"),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: true
    }
  });

  browserWindow.webContents.on("did-finish-load", injectSolis);

  browserWindow.on("closed", () => {
    browserWindow = null;
  });

  browserWindow.loadURL(url);
}

async function handlePageMessage(sender, message) {
  if (!message || message.source !== "solis-page") return {};

  if (message.type === "storage:get") {
    const storage = readStorage();
    return {
      requestId: message.requestId,
      type: "storage:response",
      value: message.keys.reduce((result, key) => {
        result[key] = storage[key];
        return result;
      }, {})
    };
  }

  if (message.type === "storage:set") {
    const storage = readStorage();
    const changes = {};

    for (const [key, newValue] of Object.entries(message.items || {})) {
      changes[key] = {
        oldValue: storage[key],
        newValue
      };
      storage[key] = newValue;
    }

    writeStorage(storage);
    broadcastStorageChange(changes);

    return {
      requestId: message.requestId,
      type: "storage:response",
      value: { changes }
    };
  }

  if (message.type === "storage:clear") {
    const storage = readStorage();
    const changes = Object.fromEntries(
      Object.entries(storage).map(([key, oldValue]) => ({
        [key]: { oldValue, newValue: undefined }
      })).map((entry) => Object.entries(entry)[0])
    );

    writeStorage({});
    broadcastStorageChange(changes);

    return {
      requestId: message.requestId,
      type: "storage:response",
      value: { changes }
    };
  }

  if (message.type === "runtime:message") {
    await handleRuntimeMessage(sender, message.message);
    return {};
  }

  return {};
}

async function handleRuntimeMessage(sender, message) {
  if (!message || typeof message !== "object") return;

  sendAnalyzerMessage(message);

  if (message.type === "DRAG_MOVE") {
    performPageMouseDrag(message);
    return;
  }

  if (message.type === "FETCH_AUDIO") {
    return;
  }

  if (message.type === "ATTACH_DEBUGGER") {
    return;
  }

  if (message.type === "FROM_CONTENT" || message.type === "BOARD_INFO") {
    return;
  }

  if (
    message.type === "STREAM" ||
    message.type === "FEN_UPDATE" ||
    message.type === "ACC" ||
    message.type === "HINT" ||
    message.type === "PV" ||
    message.type === "SVG"
  ) {
    return;
  }
}

app.whenReady().then(() => {
  protocol.handle("solis", (request) => {
    const { host, pathname } = new URL(request.url);

    if (host !== "bundle") {
      return new Response("Not found", { status: 404 });
    }

    const relativePath = decodeURIComponent(pathname).replace(/^\/+/, "");
    const root = path.resolve(app.getAppPath());
    const filePath = path.resolve(root, relativePath);
    const relative = path.relative(root, filePath);

    if (!relative || relative.startsWith("..") || path.isAbsolute(relative)) {
      return new Response("Forbidden", { status: 403 });
    }

    return net.fetch(pathToFileURL(filePath).toString());
  });

  ipcMain.handle("browser:open", (_event, url) => {
    const allowed = ["https://www.chess.com", "https://lichess.org"];
    if (!allowed.includes(url)) {
      throw new Error("Unsupported browser destination");
    }

    createBrowserWindow(url);
    return true;
  });

  ipcMain.handle("browser:message", (_event, message) => {
    if (!message || typeof message !== "object") {
      throw new Error("Invalid browser message");
    }

    postPageMessage({
      source: "solis-electron-main",
      message
    });

    return true;
  });

  ipcMain.handle("config:get", () => readStorage().chessConfig || {});

  ipcMain.handle("config:set", (_event, config) => {
    const storage = readStorage();
    const oldValue = storage.chessConfig;
    const newValue = config || {};

    storage.chessConfig = newValue;
    writeStorage(storage);

    broadcastStorageChange({
      chessConfig: {
        oldValue,
        newValue
      }
    });

    return true;
  });

  ipcMain.handle("config:clear", () => {
    const storage = readStorage();
    const oldValue = storage.chessConfig;

    delete storage.chessConfig;
    writeStorage(storage);

    broadcastStorageChange({
      chessConfig: {
        oldValue,
        newValue: undefined
      }
    });

    return true;
  });

  ipcMain.handle("browser:page-message", (event, message) =>
    handlePageMessage(event.sender, message)
  );

  ipcMain.handle("update:install", () => {
    installUpdate();
    return true;
  });

  createMainWindow();
  setupUpdater(mainWindow);

  app.on("activate", () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createMainWindow();
      setupUpdater(mainWindow);
    }
  });
});

app.on("will-quit", () => {
  if (cursorSidecar && !cursorSidecar.killed) {
    cursorSidecar.kill();
    cursorSidecar = null;
  }
});

app.on("window-all-closed", () => {
  if (process.platform !== "darwin") {
    app.quit();
  }
});