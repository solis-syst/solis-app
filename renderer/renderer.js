const message = document.getElementById("message");
const updateStatus = document.getElementById("update-status");
const updateBadge = document.getElementById("update-badge");
const installUpdateButton = document.getElementById("install-update");
const sessionTitle = document.getElementById("session-title");
const sessionDetail = document.getElementById("session-detail");
const settingsStatus = document.getElementById("settings-status");

const engineInput = document.getElementById("engine");
const eloInput = document.getElementById("elo");
const depthInput = document.getElementById("depth");
const linesInput = document.getElementById("lines");
const eloValue = document.getElementById("elo-value");
const depthValue = document.getElementById("depth-value");
const linesValue = document.getElementById("lines-value");
const showEvalInput = document.getElementById("show-eval");
const accuracyInput = document.getElementById("accuracy");
const coachInput = document.getElementById("coach");
const autoStartInput = document.getElementById("auto-start");

const DEFAULTS = {
  engine: "komodo",
  elo: 3500,
  depth: 10,
  lines: 5,
  coach: 999,
  showEval: false,
  accuracy: false,
  autoStart: false
};

let savedConfig = { ...DEFAULTS };

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

document.getElementById("nav-dashboard").addEventListener("click", () => {
  window.scrollTo({ top: 0, behavior: "smooth" });
});

function renderControlValues() {
  eloValue.textContent = eloInput.value;
  depthValue.textContent = depthInput.value;
  linesValue.textContent = linesInput.value;
}

function renderControls(config) {
  const next = { ...DEFAULTS, ...(config || {}) };

  engineInput.value = next.engine;
  eloInput.value = Number(next.elo);
  depthInput.value = Number(next.depth);
  linesInput.value = Number(next.lines);
  showEvalInput.checked = Boolean(next.showEval);
  accuracyInput.checked = Boolean(next.accuracy);
  coachInput.checked = Number(next.coach) < 998;
  autoStartInput.checked = Boolean(next.autoStart);

  renderControlValues();
}

function collectControls() {
  return {
    engine: engineInput.value,
    elo: Number(eloInput.value),
    depth: Number(depthInput.value),
    lines: Number(linesInput.value),
    coach: coachInput.checked ? 987 : 999,
    showEval: showEvalInput.checked,
    accuracy: accuracyInput.checked,
    autoStart: autoStartInput.checked
  };
}

async function loadSettings() {
  try {
    const config = await window.solis.getConfig();
    savedConfig = { ...DEFAULTS, ...(config || {}) };
    renderControls(savedConfig);
    settingsStatus.textContent = "Settings loaded.";
  } catch (error) {
    renderControls(DEFAULTS);
    settingsStatus.textContent = "Could not load settings: " + error.message;
  }
}

eloInput.addEventListener("input", renderControlValues);
depthInput.addEventListener("input", renderControlValues);
linesInput.addEventListener("input", renderControlValues);

document.getElementById("save-settings").addEventListener("click", async () => {
  const nextConfig = { ...savedConfig, ...collectControls() };

  try {
    await window.solis.setConfig(nextConfig);
    savedConfig = nextConfig;
    settingsStatus.textContent = "Settings saved. Reopen the chess browser to apply engine changes.";
  } catch (error) {
    settingsStatus.textContent = "Could not save settings: " + error.message;
  }
});

document.getElementById("reset-settings").addEventListener("click", async () => {
  const nextConfig = { ...savedConfig, ...DEFAULTS };

  try {
    await window.solis.setConfig(nextConfig);
    savedConfig = nextConfig;
    renderControls(nextConfig);
    settingsStatus.textContent = "Controls reset and saved. Reopen the chess browser to apply.";
  } catch (error) {
    settingsStatus.textContent = "Could not reset settings: " + error.message;
  }
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

loadSettings();
