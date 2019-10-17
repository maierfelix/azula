import fs from "fs";
import addon from "./index.js";

let frame = new addon.GUIFrame({
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

frame.loadHTML(`
  <style>
    body {
      background: rgba(255,255,255,1.0);
      background: linear-gradient(-45deg, #ee7752, #e73c7e, #23a6d5, #23d5ab);
      background-size: 400% 400%;
      animation: gradientBG 15s ease infinite;
      font-family: Roboto,-apple-system,BlinkMacSystemFont,"Helvetica Neue","Segoe UI","Oxygen","Ubuntu","Cantarell","Open Sans",sans-serif;
    }
    .btn {
      position:absolute;
      right: 0.5rem;
      text-decoration: none;
      color: rgba(222,222,222,0.8);
      background-color: rgba(0, 0, 0, 0.475);
      border-radius: 2em;
      padding: 0.575em;
      padding-left: 0.875em;
      padding-right: 0.875em;
      text-transform: capitalize;
      z-index: 9999;
      transition: background 0.5s;
    }
    .btn:hover {
      background-color: rgba(0, 0, 0, 0.575);
    }
    .btn:active {
      color: rgba(222,222,222,1.0);
    }
    @keyframes gradientBG {
      0% {
        background-position: 0% 50%;
      }
      50% {
        background-position: 100% 50%;
      }
      100% {
        background-position: 0% 50%;
      }
    }
  </style>
  <a id="test" class="btn" href="#">Hell o World</a>
  <script>
  window.onbinarymessage = (buf, args) => {
    setTimeout(() => {
      new Uint8Array(buf)[0] = (Math.random() * 255) | 0;
      args.kind = 42;
      console.log(buf);
      window.dispatchBinaryBuffer(buf, args);
    }, 1e3);
  };
  setTimeout(() => {
    window.onbinarymessage(buf, { kind: 666 });
  }, 100);
  </script>
`);
frame.onbinarymessage(new ArrayBuffer(16), { kind: 420 });

(function updateLoop() {
  if (frame.shouldClose()) return;
  frame.update();
  setImmediate(updateLoop);
})();
