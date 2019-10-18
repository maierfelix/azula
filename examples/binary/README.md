# Binary

This example demonstrates how to use the Binary Message System of *azula*.

 - A simple fractal gets generated in Node and written into an ArrayBuffer
 - This ArrayBuffer gets sent to the GUI using [dispatchBinaryBuffer](https://github.com/maierfelix/azula#windowprototypedispatchbinarybuffer)
 - In the GUI, we listen for this ArrayBuffer using the [onbinarymessage](https://github.com/maierfelix/azula#windowprototypeonbinarymessage) listener
 - After the GUI received the ArrayBuffer, it loops through it's data and generates a pixel grid of `div` elements, where each `div` corresponds to the relative ArrayBuffer pixel

### Preview:

<img src="https://i.imgur.com/7ZqhjEa.png"/>

Since there is no Canvas API yet, each pixel creates a new `div` element, which is unperformant.
