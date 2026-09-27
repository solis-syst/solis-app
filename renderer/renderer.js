const message = document.getElementById("message");
const updateStatus = document.getElementById("update-status");
const updateBadge = document.getElementById("update-badge");
const installUpdateButton = document.getElementById("install-update");
const sessionTitle = document.getElementById("session-title");
const sessionDetail = document.getElementById("session-detail");

async function openSite(url, name) {
  message.textContent = "Opening " + name + "...";

  try {
    await window.solis.openBrowser(url);
    message.textContent = name + " opened.";
    sessionTitle.textContent = name + " opened";
    sessionDetail.textContent = "Waiting for a position from the managed browser.";
  } catch (error) {
    message.textContent = error.message;
    sessionTitle.textContent = "Browser could not open";
    sessionDetail.textContent = error.message;
  }
}

document.getElementById("chesscom").addEventListener("click", () => {
  openSite("https://www.chess.com", "Chess.com");
});

document.getElementById("lichess").addEventListener("click", () => {
  openSite("https://lichess.org", "Lichess");
});

document.getElementById("browser-action").addEventListener("click", () => {
  openSite("https://www.chess.com", "Chess.com");
});

document.getElementById("nav-browser").addEventListener("click", () => {
  document.getElementById("browser-action").focus();
});

document.getElementById("nav-updates").addEventListener("click", () => {
  document.getElementById("updates-card").scrollIntoView({ behavior: "smooth", block: "center" });
});

installUpdateButton.addEventListener("click", async () => {
  installUpdateButton.disabled = true;
  await window.solis.installUpdate();
});

function setUpdateBadge(text) {
  updateBadge.textContent = text;
}

window.solis.onUpdateStatus((data) => {
  if (data.status === "dev") {
    updateStatus.textContent = "Updater disabled in development.";
    setUpdateBadge("Development");
    return;
  }

  if (data.status === "checking") {
    updateStatus.textContent = "Checking for updates...";
    setUpdateBadge("Checking");
    return;
  }

  if (data.status === "available") {
    updateStatus.textContent = "Update " + data.version + " found. Downloading...";
    setUpdateBadge("Downloading");
    return;
  }

  if (data.status === "downloading") {
    updateStatus.textContent = "Downloading update " + Math.round(data.percent || 0) + "%";
    setUpdateBadge(Math.round(data.percent || 0) + "%");
    return;
  }

  if (data.status === "downloaded") {
    updateStatus.textContent = "Update " + data.version + " downloaded.";
    setUpdateBadge("Ready");
    installUpdateButton.hidden = false;
    return;
  }

  if (data.status === "up-to-date") {
    updateStatus.textContent = "Solis is up to date.";
    setUpdateBadge("Up to date");
    return;
  }

  if (data.status === "error") {
    updateStatus.textContent = "Update check failed: " + data.message;
    setUpdateBadge("Error");
  }
});

window.solis.onAnalyzerMessage((data) => {
  if (!data || typeof data !== "object") return;

  if (data.type === "FROM_CONTENT" && data.fen) {
    message.textContent = "Position detected. Analyzer is working...";
    sessionTitle.textContent = "Position detected";
    sessionDetail.textContent = "Local engine analysis is running.";
    return;
  }

  if (data.type === "STREAM") {
    message.textContent = "Engine analysis received.";
    sessionTitle.textContent = "Analysis received";
    sessionDetail.textContent = "Solis received a new engine result.";
  }
});
