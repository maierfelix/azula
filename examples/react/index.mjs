import fs from "fs";
import azula from "azula";

// create a window
let window = new azula.Window({
  width: 640,
  height: 640,
  title: "azula"
});

// forwards GUI console calls to node CLI
window.onconsolemessage = e => {
  let message = e.message;
  let loc = `at ${e.source ? e.source + ":" : ""}${e.location.line}:${e.location.column}`;
  e.callee.apply(console, [e.message, loc]);
};

// load the basic HTML file
window.loadHTML(fs.readFileSync("./index.html", "utf8"));

// execute the update loop
// this loop runs as long as our window is active
(function updateLoop() {
  // abort, if the window was closed
  if (window.shouldClose()) return;
  // poll events (make the window interactive)
  window.update();
  // re-run this loop as soon as possible
  setImmediate(updateLoop);
})();
