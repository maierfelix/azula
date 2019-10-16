const fs = require("fs");
const ncp = require("ncp");
const { spawn } = require("child_process");

const pkg = require("./package.json");

const platform = process.platform;
const v8Version = process.versions.v8;
const nodeVersion = process.versions.node;
const architecture = process.arch;

ncp.limit = 16;

const msvsVersion = process.env.npm_config_msvsversion || "";

process.stdout.write(`
Compiling...
Platform: ${platform} | ${architecture}
Node: ${nodeVersion}
V8: ${v8Version}
`);

// build
// build/release
let buildDir = `./build/`;
let buildReleaseDir = buildDir + "Debug/";
if (!fs.existsSync(buildDir)) fs.mkdirSync(buildDir);
if (!fs.existsSync(buildReleaseDir)) fs.mkdirSync(buildReleaseDir);

function copyFiles() {
  process.stdout.write(`\nCopying files..\n`);
  return new Promise(resolve => {
    // copy files into release folder
    let libDir = `./third_party/lib/${platform}/${architecture}`;
    let binDir = `./third_party/bin/${platform}/${architecture}`;
    let targetDir = `./build/Debug`;
    let sdkPath = process.env.VULKAN_SDK;
    let files = [];
    // add win32 runtime files
    if (platform === "win32") {
      // libs
      files.push(
        [`${libDir}/AppCore.lib`, targetDir + `/../`],
        [`${libDir}/Ultralight.lib`, targetDir + `/../`],
        [`${libDir}/UltralightCore.lib`, targetDir + `/../`],
        [`${libDir}/WebCore.lib`, targetDir + `/../`],
        //[`${libDir}/glfw3.lib`, targetDir + `/../`],
        //[`${libDir}/glfw3dll.lib`, targetDir + `/../`],
      );
      // dlls
      files.push(
        //[`${binDir}/glfw3.dll`, targetDir],
        //[`${binDir}/DirectXTK.dll`, targetDir],
        [`${binDir}/AppCore.dll`, targetDir],
        [`${binDir}/Ultralight.dll`, targetDir],
        [`${binDir}/UltralightCore.dll`, targetDir],
        [`${binDir}/WebCore.dll`, targetDir],
      );
      // pdbs
      files.push(
        [`${binDir}/AppCore.pdb`, targetDir],
        [`${binDir}/WebCore.pdb`, targetDir],
      );
    }
    let counter = 0;
    files.map(entry => {
      let source = entry[0];
      let target = entry[1];
      // copy single files
      let fileName = source.replace(/^.*[\\\/]/, "");
      let isFile = fileName.length > 0;
      if (isFile) target += "/" + fileName;
      // copy
      ncp(source, target, error => {
        process.stdout.write(`Copying ${source} -> ${target}\n`);
        if (error) {
          process.stderr.write(`Failed to copy ${source} -> ${target}\n`);
          throw error;
        }
      });
      if (counter++ >= files.length - 1) {
        process.stdout.write("Done!\n");
        resolve(true);
      }
    });
  });
};

function buildFiles() {
  process.stdout.write(`\nCompiling bindings..\n`);
  return new Promise(resolve => {
    let msargs = "";
    // add win32 vs version
    if (platform === "win32") {
      msargs += `--msvs_version ${msvsVersion}`;
    }
    let cmd = `node-gyp configure --debug && node-gyp build`;
    let shell = spawn(cmd, { shell: true, stdio: "inherit" }, { stdio: "pipe" });
    shell.on("exit", error => {
      if (!error) process.stdout.write("Done!\n");
      resolve(!error);
    });
  });
};

(async function run() {
  await copyFiles();
  let buildSuccess = await buildFiles();
  if (buildSuccess) {
    process.stdout.write(`\nCompilation succeeded!\n`);
  } else {
    process.stderr.write(`\nCompilation failed!`);
  }
})();
