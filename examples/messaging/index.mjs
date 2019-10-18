import fs from "fs";
import azula from "azula";

let window = new azula.Window({
  width: 512,
  height: 512,
  title: "messaging"
});

window.loadHTML(fs.readFileSync("./index.html", "utf8"));

// forwards GUI console calls to node CLI
window.onconsolemessage = e => {
  e.callee.apply(console, [e.message]);
};

// listen for incoming messages from the GUI
// when we receive a "PONG" message from the GUI,
// then we answer back with a "PING" after a short delay
window.onobjectmessage = obj => {
  console.log("<Node>: Got Message: " + obj.message);
// send a message back to the GUI
  setTimeout(() => {
    // only answer if the message we received was a ping-pong message
    if (obj.message === "PONG") window.dispatchObject({ message: "PING" });
  }, 750);
};

// send an initial fake message to start the ping-pong communication
setTimeout(() => {
  window.onobjectmessage({ message: "PONG" });
}, 1e2);

(function updateLoop() {
  if (window.shouldClose()) return;
  window.update();
  setImmediate(updateLoop);
})();
