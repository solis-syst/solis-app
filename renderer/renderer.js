const message = document.getElementById("message");

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
