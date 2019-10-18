import fs from "fs";
import azula from "azula";

let window = new azula.Window({
  width: 512,
  height: 512,
  title: "azula"
});

// forwards GUI console calls to node CLI
window.onconsolemessage = e => {
  let message = e.message;
  let loc = `at ${e.source ? e.source + ":" : ""}${e.location.line}:${e.location.column}`;
  e.callee.apply(console, [e.message, loc]);
};

window.onbinarymessage = (buffer, args) => {
  setTimeout(() => {
    args.kind = 666;
    console.log(buffer);
    window.dispatchBinaryBuffer(buffer, args);
  }, 1e3);
};

window.onresize = e => {
  console.log(e.width, e.height);
};

window.loadHTML(fs.readFileSync("./index.html", "utf8"));

(function updateLoop() {
  if (window.shouldClose()) return;
  window.update();
  setImmediate(updateLoop);
})();
