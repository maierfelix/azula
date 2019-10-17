import fs from "fs";
import azula from "azula";

let window = new azula.Window({
  width: 480,
  height: 320,
  title: "azula",
  useOffscreenRendering: false
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

window.onbinarymessage(new ArrayBuffer(16), { kind: 420 });

(function updateLoop() {
  if (window.shouldClose()) return;
  window.update();
  setImmediate(updateLoop);
})();
