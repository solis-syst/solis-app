const message = document.getElementById("message");
const updateStatus = document.getElementById("update-status");
const installUpdateButton = document.getElementById("install-update");

async function openSite(url, name) {
  message.textContent = "Opening " + name + "...";

  try {
    await window.solis.openBrowser(url);
    message.textContent = name + " opened.";
  } catch (error) {
    message.textContent = error.message;
  }
}

document.getElementById("chesscom").addEventListener("click", () => {
  openSite("https://www.chess.com", "Chess.com");
});

document.getElementById("lichess").addEventListener("click", () => {
  openSite("https://lichess.org", "Lichess");
});

installUpdateButton.addEventListener("click", async () => {
  installUpdateButton.disabled = true;
  await window.solis.installUpdate();
});

window.solis.onUpdateStatus((data) => {
  if (data.status === "dev") {
    updateStatus.textContent = "Updater disabled in development.";
    return;
  }

  if (data.status === "checking") {
    updateStatus.textContent = "Checking for updates...";
    return;
  }

  if (data.status === "available") {
    updateStatus.textContent = "Update " + data.version + " found. Downloading...";
    return;
  }

  if (data.status === "downloading") {
    updateStatus.textContent = "Downloading update " + Math.round(data.percent || 0) + "%";
    return;
  }

  if (data.status === "downloaded") {
    updateStatus.textContent = "Update " + data.version + " downloaded.";
    installUpdateButton.hidden = false;
    return;
  }

  if (data.status === "up-to-date") {
    updateStatus.textContent = "Solis is up to date.";
    return;
  }

  if (data.status === "error") {
    updateStatus.textContent = "Update check failed: " + data.message;
  }
});

window.solis.onAnalyzerMessage((data) => {
  if (!data || typeof data !== "object") return;

  if (data.type === "FROM_CONTENT" && data.fen) {
    message.textContent = "Analyzing position...";
    return;
  }

  if (data.type === "STREAM") {
    message.textContent = "Engine analysis received.";
  }
});