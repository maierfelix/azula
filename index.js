const fs = require("fs");
const path = require("path");
const pkg = require("./package.json");

let {platform} = process;

let addonPath = path.join(__dirname, `/build/Release/addon-${platform}.node`);

// make sure the bindings exist
if (!fs.existsSync(addonPath)) {
  process.stderr.write(`Failed to load addon from '${addonPath}'\n`);
  process.stderr.write(`Your platform might not be supported\n`);
  throw `Exiting..`;
}

module.exports = require(addonPath);
