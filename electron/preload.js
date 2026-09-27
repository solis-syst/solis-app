const { contextBridge, ipcRenderer } = require("electron");

contextBridge.exposeInMainWorld("solis", {
  openBrowser: (url) => ipcRenderer.invoke("browser:open", url),
  getConfig: () => ipcRenderer.invoke("config:get"),
  setConfig: (config) => ipcRenderer.invoke("config:set", config),
  clearConfig: () => ipcRenderer.invoke("config:clear"),
  installUpdate: () => ipcRenderer.invoke("update:install"),
  onUpdateStatus: (callback) => {
    const listener = (_event, status) => callback(status);
    ipcRenderer.on("update:status", listener);
    return () => ipcRenderer.removeListener("update:status", listener);
  }
});

window.addEventListener("message", async (event) => {
  const data = event.data;

  if (!data || data.source !== "solis-page") return;

  const response = await ipcRenderer.invoke("browser:page-message", data);

  if (response && response.type === "storage:response") {
    window.postMessage(
      {
        source: "solis-electron",
        requestId: response.requestId,
        type: response.type,
        value: response.value
      },
      "*"
    );
  }
});
