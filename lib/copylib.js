const os = require("os");
const fs = require("fs");
const path = require("path");

const arch = os.arch();

switch (os.platform()) {
  case "darwin":
    if (arch == "x64") {
      copy("mac_x86_64");
    } else {
      copy("mac_arm64");
    }
    break;
  case "win32":
    if (arch == "x64") {
      copy("win_x86_64");
    }
    break;
  default:
    if (arch == "x64") {
      copy("linux_x86_64");
    } else if (arch == "arm64") {
      copy("linux_arm64");
    }
}

function copy(src) {
  fs.copyFileSync(
    path.resolve(__dirname, src, "libsamplerate.a"),
    path.resolve(__dirname, "libsamplerate.a")
  );
}
