<p align="center">
  <a href="#">
    <img src="https://i.imgur.com/FBN4t6x.png" height="118">
  </a>
  <br/>
  <br/>
  Lightweight GPU accelerated HTML GUI for Node
  <br/>
  <br/>
  <a href="https://www.npmjs.com/package/azula">
    <img src="https://img.shields.io/npm/v/azula.svg?style=flat-square" alt="NPM Version" />
  </a>
  <a href="//www.npmjs.com/package/azula">
    <img src="https://img.shields.io/npm/dt/azula.svg?style=flat-square" alt="NPM Downloads" />
  </a>
  <br/>
  <br/>
  <img src="https://i.imgur.com/bfwxryC.gif" />
</p>

#

*azula* is a lightweight GPU accelerated HTML GUI for Node. This project uses [Ultralight](https://github.com/ultralight-ux/Ultralight), which is an embedding friendly Fork of [WebKit](https://webkit.org/), with less memory usage and low disk space requirements.

*azula* can optionally run in [OSR](#OSR) mode, which makes it easy to embed *azula* in low-level JavaScript projects.

## Platforms

*azula* comes with pre-built N-API binaries for the following platforms:

|       OS      |     Status    |
| ------------- | ------------- |
| <img src="https://i.imgur.com/FF3Ssp6.png" alt="" height="16px">  Windows       | ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ✔ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌|
| <img src="https://i.imgur.com/bkBCY7V.png" alt="" height="16px">  Linux         | ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ In Progress ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌|
| <img src="https://i.imgur.com/iPt4GHz.png" alt="" height="16px">  MacOS         | ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ In Progress ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌|

## Getting Started

Install azula using:
````
npm install azula
````

You can now import *azula* into your project:
````js
const azula = require("azula");
````

## API

### Window

When creating a new Window, the following parameters are available:

| Name | Type | Description |
| :--- | :--- | :--- |
| width | *Number* | The initial width of the window |
| height | *Number* | The initial height of the window |
| title | *String* | The initial title of the window |
| useOffscreenRendering | *Boolean* | Creates the window in [OSR mode](#OSR) |

````js
let window = new azula.Window({
  width: 480,
  height: 320,
  title: "My App",
  useOffscreenRendering: false
});
````

#### Window.prototype.title

| Type | Description |
| :--- | :--- |
| *String* | A getter/setter allowing to retrieve or update the title of the window |

````js
window.title = "My App";
window.title; // "My App"
````

#### Window.prototype.loadHTML

| name | Type | Description |
| :--- | :--- | :--- |
| html | *String* | String representation of the HTML to load |

````js
window.loadHTML("<button>Hello World!</button>");
````

#### Window.prototype.loadFile

| name | Type | Description |
| :--- | :--- | :--- |
| path | *String* | The path from where the content gets read from |

````js
window.loadFile("./index.html");
````

#### Window.prototype.onresize

| Type | Description |
| :--- | :--- |
| *Function* | The function to call when the window gets resized |

The callback's Event parameter has the following structure:

| name | Type | Description |
| :--- | :--- | :--- |
| width | *Number* | The new width of the window |
| height | *Number* | The new height of the window |

````js
window.onresize = e => {
  console.log(e.width, e.height);
};
````

#### Window.prototype.onconsolemessage

| Type | Description |
| :--- | :--- |
| *Function* | The function to call when a console message got sent |

The underlying JavaScript engine of *azula* is WebKit's [JavaScriptCore](https://developer.apple.com/documentation/javascriptcore) engine. Now this means, that the JavaScript running in the GUI is separated from the JavaScript in Node. When the JavaScript in the GUI makes a call to the console, e.g. `console.log(42);`, we have to route this call over to Node.

The callback's Event parameter has the following structure:

| name | Type | Description |
| :--- | :--- | :--- |
| level | *String* | The level of the console call. For example *"log"*, *"warn"* or *"error"* |
| callee | *Function* | Node's equivalent console function to call |
| message | *String* | The message passed to the console call |
| source | *String* | The file or location where the call was made. Is empty when [loadHTML](#windowprototypeloadhtml) was used |
| location | *Object* | An Object describing the exact code location where the console call was made from |

The location Object comes with the following structure:

| name | Type | Description |
| :--- | :--- | :--- |
| line | *Number* | The code line where the console call originated from |
| column | *Number* | The code column where the console call originated from |

````js
window.onconsolemessage = e => {
  let location = `at ${e.source ? e.source + ":" : ""}${e.location.line}:${e.location.column}`;
  e.callee.apply(console, [e.message, location]);
};
````

#### Window.prototype.oncursorchange

| Type | Description |
| :--- | :--- |
| *Function* | The function to call when the cursor should be changed |

The callback's Event parameter has the following structure:

| name | Type | Description |
| :--- | :--- | :--- |
| name | *String* | A name representing the cursor type to change to |

````js
window.oncursorchange = e => {
  console.log(e.name);
};
````

#### Window.prototype.onbinarymessage

The *binarymessage* system should only be used when sending large data between Node and *azula*. The `buffer` argument is a shared buffer, which means there is no overhead when sending it between Node and *azula* as the data is effectively referenced.

The second argument is an Object (and is optional), which can be used to give some additional information about the `buffer` argument. This Object should be kept small, as it gets serialized behind the scenes, and so comes with some overhead.

| Type | Description |
| :--- | :--- |
| *Function* | The function to call when a binary message was sent from the GUI |

The callback's Event parameter has the following structure:

| name | Type | Description |
| :--- | :--- | :--- |
| buffer | *ArrayBuffer* | The ArrayBuffer sent from the GUI |
| args (*Optional*) | *Object* | An Used-defined Object providing additional information about the sent *buffer* |

````js
window.onbinarymessage = (buffer, args) => {
  console.log(buffer, args);
};
````

#### Window.prototype.dispatchBinaryBuffer

The *binarymessage* system should only be used when sending large data between Node and *azula*. The `buffer` argument is a shared buffer, which means there is no overhead when sending it between Node and *azula* as the data is effectively referenced.

The second argument is an Object (and is optional), which can be used to give some additional information about the `buffer` argument. This Object should be kept small, as it gets serialized behind the scenes, and so comes with some overhead.

| name | Type | Description |
| :--- | :--- | :--- |
| buffer | *ArrayBuffer* | The ArrayBuffer to send to the GUI |
| args (*Optional*) | *Object* | An Used-defined Object providing additional information about the *buffer* |

````js
window.dispatchBinaryBuffer(new ArrayBuffer(16), { kind: "SOME_DATA" });
````

An equivalent method is available in the GUI.

#### Window.prototype.getSharedHandleD3D11

| Type | Description |
| :--- | :--- |
| *BigInt* | A BigInt representing a Windows HANDLE |

On Windows, you can use this method to retrieve a shared [HANDLE](https://docs.microsoft.com/en-us/windows/win32/winprog/windows-data-types) to the underlying D3D11 render texture.

````js
let handle = window.getSharedHandleD3D11();
````

### OSR

*azula* supports running in OSR (*Offscreen rendering*) mode. This means, that instead of creating a window, an invisible texture gets used and rendered into. This texture can then be imported into a 3D engine for example. Another common use case would be, to display the texture in a VR environment.

On **Windows**, you can request a shared HANDLE using the Window's [getSharedHandleD3D11](#windowprototypegetsharedhandled3d11) method.

## License

Azula is MIT licensed, while [Ultralight](https://github.com/ultralight-ux/Ultralight/tree/master/license) comes with the following License:

````
Ultralight is free for non-commercial use, educational use, 
and also free for commercial use by small indie developers making
less than US$100,000 a year. You can find full terms in the SDK. 
Pricing plans for larger commercial projects will be announced later.
````

For further information regaring the licensing of Ultralight, see [this](https://github.com/ultralight-ux/Ultralight) link.
