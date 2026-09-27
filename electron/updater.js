const { app } = require("electron");
const { autoUpdater } = require("electron-updater");

let mainWindow = null;

function sendStatus(status, data = {}) {
  if (!mainWindow || mainWindow.isDestroyed()) return;
  mainWindow.webContents.send("update:status", { status, ...data });
}

function setupUpdater(window) {
  mainWindow = window;
  if (!app.isPackaged) {
    sendStatus("dev");
    return;
  }
  autoUpdater.autoDownload = true;
  autoUpdater.autoInstallOnAppQuit = true;
  autoUpdater.on("checking-for-update", () => sendStatus("checking"));
  autoUpdater.on("update-available", (info) => sendStatus("available", { version: info.version }));
  autoUpdater.on("update-not-available", (info) => sendStatus("up-to-date", { version: info.version }));
  autoUpdater.on("download-progress", (progress) => sendStatus("downloading", { percent: progress.percent, transferred: progress.transferred, total: progress.total, bytesPerSecond: progress.bytesPerSecond }));
  autoUpdater.on("update-downloaded", (info) => sendStatus("downloaded", { version: info.version }));
  autoUpdater.on("error", (error) => sendStatus("error", { message: error.message }));
  autoUpdater.checkForUpdates().catch((error) => sendStatus("error", { message: error.message }));
}

function installUpdate() {
  autoUpdater.quitAndInstall();
}

module.exports = { setupUpdater, installUpdate };
