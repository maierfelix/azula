import fs from "fs";
import azula from "azula";

let width = 384;
let height = 384;

let window = new azula.Window({
  width: 512,
  height: 512,
  title: "binary"
});

window.loadHTML(fs.readFileSync("./index.html", "utf8"));

// generate a simple color fractal
let viewU8 = new Uint8Array(width * height * 4);
for (let ii = 0; ii < width * height; ++ii) {
  let x = (ii % width) | 0;
  let y = (ii / width) | 0;
  let v = ((ii * x) / (width * 3.0)) | 0;
  viewU8[ii * 4 + 0] = v * 1.05;
  viewU8[ii * 4 + 1] = v * 1.075;
  viewU8[ii * 4 + 2] = v * 1.10;
  viewU8[ii * 4 + 3] = 255;
};

// send the pixels to the GUI
setTimeout(() => {
  window.dispatchBinaryBuffer(viewU8.buffer, { width, height });
}, 1e2);

(function updateLoop() {
  if (window.shouldClose()) return;
  window.update();
  setImmediate(updateLoop);
})();
