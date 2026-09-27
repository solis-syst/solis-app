const { app, BrowserWindow, ipcMain } = require("electron");
const path = require("path");

let mainWindow = null;
let browserWindow = null;

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
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: true
    }
  });

  browserWindow.loadURL(url);

  browserWindow.on("closed", () => {
    browserWindow = null;
  });
}

app.whenReady().then(() => {
  ipcMain.handle("browser:open", (_event, url) => {
    const allowed = ["https://www.chess.com", "https://lichess.org"];
    if (!allowed.includes(url)) {
      throw new Error("Unsupported browser destination");
    }

    createBrowserWindow(url);
    return true;
  });

  createMainWindow();

  app.on("activate", () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createMainWindow();
    }
  });
});

app.on("window-all-closed", () => {
  if (process.platform !== "darwin") {
    app.quit();
  }
});
