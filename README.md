<p align="center">
  <a href="#">
    <img src="https://i.imgur.com/FBN4t6x.png" height="118">
  </a>
  <br/>
  <br/>
  <i>azula</i>, a lightweight GPU accelerated HTML GUI for native JavaScript applications
  <br/>
  <br/>
  <a href="https://www.npmjs.com/package/azula">
    <img src="https://img.shields.io/npm/v/azula.svg?style=flat-square" alt="NPM Version" />
  </a>
  <br/>
  <br/>
  <img src="https://i.imgur.com/bfwxryC.gif" />
</p>

## ‌‌

*azula* is a lightweight alternative to Electron. It is based on [Ultralight](https://github.com/ultralight-ux/Ultralight), which is an embedding friendly Fork of [WebKit](https://webkit.org/), with less memory usage and low disk space requirements.

*azula* can optionally run in [OSR](#osr) mode, which makes it easy to embed *azula* in existing Projects like Game/VR Engines.

## Characteristics

|  | Azula | Electron |
| :--- | :--- | :--- |
| CPU | 1.2% | 4.2% |
| RAM | 37Mb | 64Mb |
| DISK | 31Mb | 118Mb |

## Platforms

*azula* comes with pre-built N-API binaries for the following platforms:

|       OS      |     Status    |
| ------------- | ------------- |
| <img src="https://i.imgur.com/FF3Ssp6.png" alt="" height="16px">  Windows       | ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ✔ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌|
| <img src="https://i.imgur.com/bkBCY7V.png" alt="" height="16px">  Linux         | ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ In Progress ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌|
| <img src="https://i.imgur.com/iPt4GHz.png" alt="" height="16px">  MacOS         | ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ In Progress ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌|

## Getting Started

Install *azula* using:
````
npm install azula
````

You can now import *azula* into your project:
````js
const azula = require("azula");
````

Or with *ESM*:
````js
import azula from "azula";
````

## API

  * [Window](#window)
    - [General](#general)
      - [title](#windowprototypetitle)
      - [width](#windowprototypewidth)
      - [height](#windowprototypeheight)
      - [update](#windowprototypeupdate)
      - [flush](#windowprototypeflush)
      - [shouldClose](#windowprototypeshouldclose)
    - [Loading](#loading)
      - [loadHTML](#windowprototypeloadhtml)
      - [loadFile](#windowprototypeloadfile)
    - [Events](#events)
      - [onresize](#windowprototypeonresize)
      - [oncursorchange](#windowprototypeoncursorchange)
      - [onconsolemessage](#windowprototypeonconsolemessage)
    - [Event Dispatching](#event-dispatching)
      - [dispatchMouseEvent](#windowprototypedispatchmouseevent)
      - [dispatchKeyEvent](#windowprototypedispatchkeyevent)
      - [dispatchScrollEvent](#windowprototypedispatchscrollevent)
    - [Object Messaging](#object-messaging)
      - [dispatchObject](#windowprototypedispatchobject)
      - [onobjectmessage](#windowprototypeonobjectmessage)
    - [Binary Messaging](#binary-messaging)
      - [dispatchBinaryBuffer](#windowprototypedispatchbinarybuffer)
      - [onbinarymessage](#windowprototypeonbinarymessage)
    - [OSR Mode](#osr-mode)
      - [getSharedHandleD3D11](#windowprototypegetsharedhandled3d11)

## Window

When creating a new Window, the following parameters are available:

| Name | Type | Description |
| :--- | :--- | :--- |
| width (*Optional*) | *Number* | The initial width of the window |
| height (*Optional*) | *Number* | The initial height of the window |
| title (*Optional*) | *String* | The initial title of the window |
| useOffscreenRendering (*Optional*) | *Boolean* | When *true*, creates the window in [OSR mode](#osr) |

````js
let window = new azula.Window({
  width: 480,
  height: 320,
  title: "My App",
  useOffscreenRendering: false
});
````

## General

### Window.prototype.title

| Type | Description |
| :--- | :--- |
| *String* | A getter/setter allowing to retrieve or update the title of the window |

````js
window.title = "My App";
window.title; // "My App"
````

### Window.prototype.width

| Type | Description |
| :--- | :--- |
| *Number* | A getter/setter allowing to retrieve or update the width of the window |

````js
window.width = 640;
window.width; // 640
````

### Window.prototype.height

| Type | Description |
| :--- | :--- |
| *Number* | A getter/setter allowing to retrieve or update the height of the window |

````js
window.height = 480;
window.height; // 480
````

### Window.prototype.update

This method should be called to poll window events (making the window interactive). In *non-OSR* mode, this method also does the painting of the window.

````js
window.update();
````

### Window.prototype.flush

This method should only be used in [OSR](#osr) mode. Calling this method executes all remaining render operations and flushes the underlying context.

````js
window.flush();
````

### Window.prototype.shouldClose

| Type | Description |
| :--- | :--- |
| *Boolean* | A boolean, indicating if the window should be closed |

````js
window.shouldClose(); // true/false
````

## Loading

### Window.prototype.loadHTML

| Name | Type | Description |
| :--- | :--- | :--- |
| html | *String* | String representation of the HTML to load |

````js
window.loadHTML("<button>Hello World!</button>");
````

### Window.prototype.loadFile

| Name | Type | Description |
| :--- | :--- | :--- |
| path | *String* | The path from where the content gets read from |

````js
window.loadFile("./index.html");
````

## Events

### Window.prototype.onresize

| Type | Description |
| :--- | :--- |
| *Function* | The function to call when the window gets resized |

The callback's Event parameter has the following structure:

| Name | Type | Description |
| :--- | :--- | :--- |
| width | *Number* | The new width of the window |
| height | *Number* | The new height of the window |

````js
window.onresize = e => {
  console.log(e.width, e.height);
};
````

### Window.prototype.oncursorchange

| Type | Description |
| :--- | :--- |
| *Function* | The function to call when the cursor should be changed |

The callback's Event parameter has the following structure:

| Name | Type | Description |
| :--- | :--- | :--- |
| name | *String* | A name representing the cursor type to change to |

````js
window.oncursorchange = e => {
  console.log(e.name);
};
````

### Window.prototype.onconsolemessage

| Type | Description |
| :--- | :--- |
| *Function* | The function to call when a console message got sent |

The underlying JavaScript engine of *azula* is WebKit's [JavaScriptCore](https://developer.apple.com/documentation/javascriptcore) engine. Now this means, that the JavaScript running in the GUI is separated from the JavaScript in Node. When the JavaScript in the GUI makes a call to the console, e.g. `console.log(42);`, we have to route this call over to Node.

The callback's Event parameter has the following structure:

| Name | Type | Description |
| :--- | :--- | :--- |
| level | *String* | The level of the console call. For example *"log"*, *"warn"* or *"error"* |
| callee | *Function* | Node's equivalent console function to call |
| message | *String* | The message passed to the console call |
| source | *String* | The file or location where the call was made. Is empty when [loadHTML](#windowprototypeloadhtml) was used |
| location | *Object* | An Object describing the exact code location where the console call was made from |

The location Object comes with the following structure:

| Name | Type | Description |
| :--- | :--- | :--- |
| line | *Number* | The code line where the console call originated from |
| column | *Number* | The code column where the console call originated from |

````js
window.onconsolemessage = e => {
  let location = `at ${e.source ? e.source + ":" : ""}${e.location.line}:${e.location.column}`;
  e.callee.apply(console, [e.message, location]);
};
````

## Event Dispatching

The Event Dispatching System should only be used in [OSR](#osr) mode. Event Dispatching allows to manually send events to the GUI, such as mouse gestures or key events.

### Window.prototype.dispatchMouseEvent

| Name | Type | Description |
| :--- | :--- | :--- |
| type | *String* | The type of event |
| x | *Number* | The horizontal position of the mouse |
| y | *Number* | The vertical position of the mouse |
| button | *Number* | The currently pressed mouse button |

The following event types are available:

| Name | Type |
| :--- | :--- |
| onmousedown | Simulating a mouse press action |
| onmouseup | Simulating a mouse leave action |
| onmousemove | Simulating a mouse move action |

````js
window.dispatchMouseEvent("onmousedown", 16, 32, 1); // press the left mouse button at 16:32
window.dispatchMouseEvent("onmouseup", 16, 32, 1); // leave the left mouse button at 16:32
window.dispatchMouseEvent("onmousemove", 16, 32, 0); // move the mouse to 16:32 without pressing a mouse button
````

### Window.prototype.dispatchKeyEvent

Key Codes are mapped towards [GLFW's Key Codes](https://www.glfw.org/docs/latest/group__keys.html).

| Name | Type | Description |
| :--- | :--- | :--- |
| type | *String* | The type of event |
| keyCode | *Number* | A key code representing which key to press |

The following event types are available:

| Name | Type |
| :--- | :--- |
| onkeydown | Simulating a key press action |
| onkeyup | Simulating a key leave action |

````js
window.dispatchKeyEvent("onkeydown", x); // press a key
window.dispatchKeyEvent("onkeyup", x); // leave a key
````

### Window.prototype.dispatchScrollEvent

| Name | Type | Description |
| :--- | :--- | :--- |
| type | *String* | The type of event |
| deltaX | *Number* | The horizontal amount to scroll |
| deltaY | *Number* | The vertical amount to scroll |

The following event types are available:

| Name | Type |
| :--- | :--- |
| onmousewheel | Simulating a mouse wheel action |

````js
window.dispatchScrollEvent("onmousewheel", 0, 1); // scroll upwards, vertically by 1
window.dispatchScrollEvent("onmousewheel", -1, 0); // scroll downwards, horizontally by -1
````

## Object Messaging

The underlying JavaScript engine of *azula* is WebKit's [JavaScriptCore](https://developer.apple.com/documentation/javascriptcore) engine. The JavaScript engine of Node is different to the one used in *azula*, so we cannot directly exchange data. The Object Messaging System allows to send Object between both engines.

Note that to be sent Objects should kept small, as behind the scenes, they get serialized.

### Window.prototype.dispatchObject

An equivalent method is available in the GUI. See [this](https://github.com/maierfelix/azula/tree/master/examples/messaging) example as a reference.

| Name | Type | Description |
| :--- | :--- | :--- |
| object | *Object* | The Object to send to the GUI |

````js
window.dispatchObject({ message: "PING" });
````

### Window.prototype.onobjectmessage

An equivalent method is available in the GUI. See [this](https://github.com/maierfelix/azula/tree/master/examples/messaging) example as a reference.

| Type | Description |
| :--- | :--- |
| *Function* | The function to call when an object message was sent from the GUI |

The callback's Event parameter has the following structure:

| Name | Type | Description |
| :--- | :--- | :--- |
| object | *Object* | The Object sent from the GUI |

````js
window.onobjectmessage = object => {
  console.log(object);
};
````

## Binary Messaging

The underlying JavaScript engine of *azula* is WebKit's [JavaScriptCore](https://developer.apple.com/documentation/javascriptcore) engine. The JavaScript engine of Node is different to the one used in *azula*, so we cannot directly exchange data. The Binary Messaging System allows to efficiently pass [ArrayBuffers](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/ArrayBuffer) between both engines. Even though the engines are different, ArrayBuffers can be exchanged without any copying, meaning they don't have any overhead.

### Window.prototype.dispatchBinaryBuffer

An equivalent method is available in the GUI. See [this](https://github.com/maierfelix/azula/tree/master/examples/binary) example as a reference.

The *binarymessage* system should only be used when sending large data between Node and *azula*. The `buffer` argument is a referenced buffer, which means there is no overhead when sending it between Node and *azula* as the data is effectively referenced.

The second argument is an Object (and is optional), which can be used to give some additional information about the `buffer` argument. This Object should be kept small, as it gets serialized behind the scenes, and so comes with some overhead.

| Name | Type | Description |
| :--- | :--- | :--- |
| buffer | *ArrayBuffer* | The ArrayBuffer to send to the GUI |
| args (*Optional*) | *Object* | An Used-defined Object providing additional information about the *buffer* |

````js
window.dispatchBinaryBuffer(new ArrayBuffer(16), { kind: "SOME_DATA" });
````

### Window.prototype.onbinarymessage

An equivalent method is available in the GUI. See [this](https://github.com/maierfelix/azula/tree/master/examples/binary) example as a reference.

The *binarymessage* system should only be used when sending large data between Node and *azula*. The `buffer` argument is a referenced buffer, which means there is no overhead when sending it between Node and *azula* as the data is effectively referenced.

The second argument is an Object (and is optional), which can be used to give some additional information about the `buffer` argument. This Object should be kept small, as it gets serialized behind the scenes, and so comes with some overhead.

| Type | Description |
| :--- | :--- |
| *Function* | The function to call when a binary message was sent from the GUI |

The callback's Event parameter has the following structure:

| Name | Type | Description |
| :--- | :--- | :--- |
| buffer | *ArrayBuffer* | The ArrayBuffer sent from the GUI |
| args (*Optional*) | *Object* | An Used-defined Object providing additional information about the sent *buffer* |

````js
window.onbinarymessage = (buffer, args) => {
  console.log(buffer, args);
};
````

## OSR Mode

### Window.prototype.getSharedHandleD3D11

| Type | Description |
| :--- | :--- |
| *BigInt* | A BigInt representing a Windows HANDLE |

On Windows, you can use this method to retrieve a shared [HANDLE](https://docs.microsoft.com/en-us/windows/win32/winprog/windows-data-types) to the underlying D3D11 render texture.

````js
let handle = window.getSharedHandleD3D11();
````

## OSR

See [this](https://github.com/maierfelix/azula/tree/master/examples/OSR) example as a reference.

*azula* supports running in OSR (*Offscreen rendering*) mode. This means, that instead of creating a window, an invisible texture gets used and rendered into. This texture can then be imported into a 3D engine for example. Another common use case would be, to display the texture in a VR environment.

On **Windows**, you can request a shared HANDLE using the Window's [getSharedHandleD3D11](#windowprototypegetsharedhandled3d11) method.

## License

Azula is MIT licensed, while [Ultralight](https://github.com/ultralight-ux/Ultralight) comes with the following License:

````
Ultralight is free for non-commercial use, educational use, 
and also free for commercial use by small indie developers making
less than US$100,000 a year. You can find full terms in the SDK. 
Pricing plans for larger commercial projects will be announced later.
````

For further information regaring the licensing of Ultralight, see [this](https://github.com/ultralight-ux/Ultralight/tree/master/license) link.

## ‌‌
*No, you miscalculated! You should have feared me more! - [Azula](https://en.wikipedia.org/wiki/Azula)*
