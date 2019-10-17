import fs from "fs";
import addon from "./index.js";

let frame = new addon.GUIFrame({
  width: 480,
  height: 320,
  useOffscreenRendering: false
});

// forwards GUI console calls to node CLI
frame.onconsolemessage = e => {
  let message = e.message;
  let loc = `at ${e.source ? e.source + ":" : ""}${e.location.line}:${e.location.column}`;
  e.callee.apply(console, [e.message, loc]);
};

frame.onbinarymessage = (buffer, args) => {
  setTimeout(() => {
    args.kind = 666;
    console.log(buffer);
    frame.dispatchBinaryBuffer(buffer, args);
  }, 1e3);
};

console.log(frame.title);
frame.title = "azula";
console.log(frame.title);

frame.onresize = e => {
  console.log(e.width, e.height);
};

frame.loadHTML(fs.readFileSync("./index.html", "utf8"));
frame.onbinarymessage(new ArrayBuffer(16), { kind: 420 });

(function updateLoop() {
  if (frame.shouldClose()) return;
  frame.update();
  setImmediate(updateLoop);
})();
