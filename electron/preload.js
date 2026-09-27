const { contextBridge, ipcRenderer } = require("electron");

contextBridge.exposeInMainWorld("solis", {
  openBrowser: (url) => ipcRenderer.invoke("browser:open", url),
  getConfig: () => ipcRenderer.invoke("config:get"),
  setConfig: (config) => ipcRenderer.invoke("config:set", config),
  clearConfig: () => ipcRenderer.invoke("config:clear")
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
