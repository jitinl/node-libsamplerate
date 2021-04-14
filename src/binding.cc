#include <napi.h>
#include "node-libsamplerate.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    SampleRateStream::Init(env, exports);
    return exports;
}

NODE_API_MODULE(addon, Init)