const { contextBridge, ipcRenderer } = require("electron");

contextBridge.exposeInMainWorld("solis", {
  openBrowser: (url) => ipcRenderer.invoke("browser:open", url)
});
