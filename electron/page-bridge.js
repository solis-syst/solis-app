(() => {
  const listeners = [];
  const pending = new Map();
  let requestId = 0;

  function request(type, payload = {}) {
    const id = ++requestId;

    return new Promise((resolve) => {
      pending.set(id, resolve);
      window.postMessage(
        {
          source: "solis-page",
          requestId: id,
          type,
          ...payload
        },
        "*"
      );
    });
  }

  window.addEventListener("message", (event) => {
    const data = event.data;

    if (!data || data.source !== "solis-electron") return;

    const resolve = pending.get(data.requestId);
    if (!resolve) return;

    pending.delete(data.requestId);
    resolve(data.value || {});
  });

  const storage = {
    local: {
      get(keys, callback) {
        const list = Array.isArray(keys)
          ? keys
          : typeof keys === "string"
            ? [keys]
            : Object.keys(keys || {});

        request("storage:get", { keys: list }).then((value) => {
          if (typeof callback === "function") callback(value);
        });
      },

      set(items, callback) {
        request("storage:set", { items }).then(() => {
          for (const listener of listeners) {
            listener(value.changes || {}, "local");
          }
          if (typeof callback === "function") callback();
        });
      },

      clear(callback) {
        request("storage:clear").then(() => {
          for (const listener of listeners) {
            listener({});
          }
          if (typeof callback === "function") callback();
        });
      }
    },

    onChanged: {
      addListener(listener) {
        if (typeof listener === "function") listeners.push(listener);
      }
    }
  };

  window.chrome = {
    runtime: {
      id: "solis",
      getURL(path) {
        return "solis://bundle/" + String(path).replace(/^\/+/, "");
      },
      sendMessage(message, callback) {
        request("runtime:message", { message }).then((value) => {
          if (typeof callback === "function") callback(value);
        });
        return Promise.resolve({});
      },
      onMessage: {
        addListener(listener) {
          window.addEventListener("message", (event) => {
            const data = event.data;
            if (
              !data ||
              data.source !== "solis-electron-main" ||
              !data.message
            ) return;
            listener(data.message, {}, () => {});
          });
        }
      }
    },
    storage
  };
})();