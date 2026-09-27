const { app, BrowserWindow, ipcMain, net, protocol } = require("electron");
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

app.on("window-all-closed", () => {
  if (process.platform !== "darwin") {
    app.quit();
  }
});