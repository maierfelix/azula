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

### OSR

*azula* supports running in OSR (*Offscreen rendering*) mode. This means, that instead of creating a window, an invisible texture gets used and rendered into. This texture can then be imported into a 3D engine for example. Another common use case would be, to display the texture in a VR environment.

## License

Azula is MIT licensed, while [Ultralight](https://github.com/ultralight-ux/Ultralight/tree/master/license) comes with the following License:

````
Ultralight is free for non-commercial use, educational use, 
and also free for commercial use by small indie developers making
less than US$100,000 a year. You can find full terms in the SDK. 
Pricing plans for larger commercial projects will be announced later.
````

For further information regaring the licensing of Ultralight, see [this](https://github.com/ultralight-ux/Ultralight) link.
