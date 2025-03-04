# Fork of node-libsamplerate with prebuilt binaries and bug fixes


## Installation

    npm install jitinl/node-libsamplerate

## Bug fixes

- it is possible to use multiple libsamplerate streams in a chain. For example `resampler1.pipe(mystream).pipe(resampler2)`. This does not work in the original implementation as there's only a single options structure shared throughout the library, so the options applied to `resampler1` would end up applying to `resampler1` as well.
- uses node-gyp instead of cmakejs
- comes with prebuilt binaries
- uses the latest version of libsamplerate

## Development


To update or add prebuilt binaries for your platform:

- Build the latest [libsamplerate](https://github.com/erikd/libsamplerate) code.  
- Copy over the `libsamplerate.a` file to the folder corresponding to your platform in the `/lib` folder in this project directory. If there's no folder corresponding to your platform, make one (refer to `lib/copylib.js` for the folder name ). Otherwise just place the `libsamplerate.a` file directly in the `lib` folder.

- to now make prebuilt binaries for your platform, 

    npm run prebuild

- To upload them as github releases,
    
    export GITHUB_TOKEN=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    
    npm run upload



# node-libsamplerate

ABI stable native implementation of libsamplerate as a Transform stream. Built from the latest [libsamplerate](https://github.com/erikd/libsamplerate) code. Uses [N-API](https://nodejs.org/api/n-api.html), [node-addon-api](https://www.npmjs.com/package/node-addon-api) and [cmake-js](https://www.npmjs.com/package/cmake-js). This module has no external dependencies.

## Introduction

Allows the upsampling/downsampling and/or upconverting/downconverting to/from arbitrary sample rates and to/from 16 or 32 bits per sample. Tested on Linux (x64 and arm), Windows and MacOS.
This module uses the "Full Api" detailed [here](http://www.mega-nerd.com/libsamplerate/api_full.html)

## Install

```shell
npm install node-libsamplerate --save
```

Requires `cmake` and a valid toolchain to build.

For Windows, install the Visual C++ build tools and download cmake from [cmake.org](https://cmake.org/download/). Or install Visual Studio with full c++ support.

## Usage

Include module;
`const SampleRate = require('node-libsamplerate');`

Instantiate:
`const resample = new SampleRate(options);`
where options is an object of the form:

```javascript
let options = {
    // Value can be from 0 to 4 or using enum. 0 is the best quality and the slowest.
    type: SampleRate.SRC_SINC_MEDIUM_QALITY,
    // Stereo
    channels: 2,
    // Sample rate of source
    fromRate: 48000,
    // bit depth of source. Valid values: 16 or 32
    fromDepth: 16,
    // Desired sample rate
    toRate: 44100,
    // Desired bit depth. Valid values: 16 or 32
    toDepth: 16
}
```

Input audio data should be signed integers (e.g. S16_LE or S32_LE). Output will also be signed integers. Floating point input/output is not yet supported.
Input should be from a readable stream, output should be to a writable stream: e.g.

```javascript
const fs = require('fs');
let rs = fs.createReadStream('input.pcm');
let ws = fs.createWriteStream('output.pcm');

rs.pipe(resample).pipe(ws);
```

NOTE: if reading from a WAV file, start the read at 44 bytes to avoid the wav header (`{start:44}` for `fs.createReadStream`).
If recording from `arecord`, `sox` or similar, use format `raw`.

### Altering playback speed

This is possible by modifying the converter ratio, which can be done on the fly using the `setRatio` method. e.g. `resample.setRatio(0.5)`. Depending upon timing, this will most likely take effect at the next buffer load. If fine grained modifications to playback speed are required, then a small value for `highWaterMark` on the input stream will be needed.