#include <thread>
#include <iostream>
#include "../deps/include/samplerate.h"
#include "../deps/include/config.h"

#include <math.h>

#include "./node-libsamplerate.h"

void s24_to_float_array(const int *in, float *out, int len)
{
    while (len)
    {
        len--;
        out[len] = (float)(in[len] / (8.0 * 0x200000));
    };

    return;
}

void float_to_s24_array(const float *in, int *out, int len)
{
    double scaled_value;

    while (len)
    {
        len--;

        scaled_value = in[len] * (8.0 * 0x200000);
        if (scaled_value != 0)
        {
            double d = scaled_value;
        }
        if (scaled_value >= (1.0 * 0x7FFFFF))
        {
            out[len] = 0x7fffff;
            continue;
        };
        if (scaled_value <= (-8.0 * 0x200000))
        {
            out[len] = -1 - 0x7fffff;
            continue;
        };

        out[len] = lrint(scaled_value);
    };
}

Napi::FunctionReference SampleRateStream::constructor;

Napi::Object SampleRateStream::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);
    Napi::Function transform = DefineClass(env, "SampleRateStream",
                                           {InstanceMethod("transform", &SampleRateStream::Transform),
                                            InstanceMethod("setRatio", &SampleRateStream::SetRatio),
                                            InstanceMethod("reset", &SampleRateStream::Reset)});

    constructor = Napi::Persistent(transform);
    constructor.SuppressDestruct();

    exports.Set("SampleRateStream", transform);
    return exports;
}

SampleRateStream::SampleRateStream(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<SampleRateStream>(info)
{
    Napi::Env env = info.Env();
    Napi::Object inProps = info[0].As<Napi::Object>();
    Napi::Array propNames = inProps.GetPropertyNames();
    int len = propNames.Length();

    Napi::Object props = info.This().As<Napi::Object>();
    for (int i = 0; i < len; i++)
    {
        napi_value e;
        napi_get_element(env, propNames, i, &e);
        std::string key = Napi::String(env, e).Utf8Value();
        uint32_t value = inProps.Get(key).As<Napi::Number>().Uint32Value();
        props.Set(Napi::String::New(env, key), value);
    }
    uint32_t type = inProps.Get("type").As<Napi::Number>().Uint32Value();
    uint32_t channels = inProps.Get("channels").As<Napi::Number>().Uint32Value();
    uint32_t fromRate = inProps.Get("fromRate").As<Napi::Number>().Uint32Value();
    uint32_t toRate = inProps.Get("toRate").As<Napi::Number>().Uint32Value();
    double ratio = (double)toRate / (double)fromRate;
    this->data.src_ratio = ratio;
    int error;
    if ((this->src_state = src_new(type, channels, &error)) == NULL)
    {
        throw Napi::Error::New(info.Env(), src_strerror(error));
    };
}

Napi::Value SampleRateStream::Transform(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    void *inputBuffer;
    size_t lengthIn;
    napi_get_buffer_info(env, info[0].As<Napi::Buffer<char> >(), &inputBuffer, &lengthIn);

    Napi::Object props = info.This().As<Napi::Object>();

    uint32_t fromDepth = props.Get("fromDepth").As<Napi::Number>().Uint32Value();
    uint32_t toDepth = props.Get("toDepth").As<Napi::Number>().Uint32Value();
    uint32_t channels = props.Get("channels").As<Napi::Number>().Uint32Value();
    int error;

    // Frame length 2 for 16bit, 4 for 32bit and 24bit, for each channel
    unsigned int depth = fromDepth;
    if (fromDepth == 24)
    {
        depth = 32;
    }
    unsigned int inputFrames = (int)floor(lengthIn / (channels * (depth / 8)));
    unsigned int outputFrames = (this->data.src_ratio * inputFrames) + 1;
    unsigned int lengthOut = (int)floor(this->data.src_ratio * lengthIn);

    if (fromDepth == 16 && toDepth != 16)
    {
        lengthOut = lengthOut * 2;
    }

    float *dataOutFloat = new float[lengthOut];
    float *dataInFloat = new float[lengthIn];

    if (fromDepth == 16)
    {
        src_short_to_float_array((short *)inputBuffer, dataInFloat, inputFrames * 2);
    }
    if (fromDepth == 24)
    {
        s24_to_float_array((int *)inputBuffer, dataInFloat, inputFrames * 2);
    }
    if (fromDepth == 32)
    {
        src_int_to_float_array((int *)inputBuffer, dataInFloat, inputFrames * 2);
    }

    this->data.data_in = dataInFloat;
    this->data.data_out = dataOutFloat;
    this->data.input_frames = inputFrames;
    this->data.output_frames = outputFrames;
    this->data.end_of_input = 0;

    if ((error = src_process(this->src_state, &this->data)))
    {
        throw Napi::Error::New(info.Env(), src_strerror(error));
    };

    depth = toDepth;
    if (toDepth == 24)
    {
        depth = 32;
    }
    lengthOut = this->data.output_frames_gen * channels * (depth / 8);
    int *dataOut = new int[lengthOut];
    int inFramesUsed = this->data.input_frames_used;
    // int frameDiff = this->data.input_frames - this->data.input_frames_used;
    // if (frameDiff != 0)
    // {
    //     std::cout << "outframes differs from inframes by " << frameDiff << " output frames generated = " << this->data.output_frames_gen << " used =" << this->data.input_frames_used << std::endl;
    // }

    // if(this->data.output_frames_gen!=this->data.input_frames)
    // {
    //     std::cout <<"outframes gen = "<<data.output_frames_gen<<" in used "<<data.input_frames<<std::endl;
    // }

    if (toDepth == 16)
    {
        src_float_to_short_array(dataOutFloat, (short *)dataOut, lengthOut);
    }
    if (toDepth == 24)
    {
        float_to_s24_array(dataOutFloat, (int *)dataOut, lengthOut);
    }
    if (toDepth == 32)
    {
        src_float_to_int_array(dataOutFloat, (int *)dataOut, lengthOut);
    }

    return Napi::Buffer<char>::New(env, (char *)dataOut, lengthOut);
}

void SampleRateStream::SetRatio(const Napi::CallbackInfo &info)
{
    double ratio = info[0].As<Napi::Number>().DoubleValue();
    this->data.src_ratio = ratio;
}

void SampleRateStream::Reset(const Napi::CallbackInfo &info)
{
    int error;
    if ((error = src_reset(this->src_state)))
    {
        throw Napi::Error::New(info.Env(), src_strerror(error));
    };
}
