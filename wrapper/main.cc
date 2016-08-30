#include <nan.h>
#include <memory>
#include "wrapper.h"

using namespace std;
using namespace v8;
using namespace NativeSoundPlayer;

Local<Value> encode(const wstring& str) {
    return Nan::Encode(
        str.c_str(),
        str.length() * sizeof(wchar_t),
        Nan::Encoding::UCS2
    );
}

void WrapperPlay(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    if (info.Length() < 2) {
        Nan::ThrowTypeError("Wrong number of arguments");
        return;
    }

    if (!info[0]->IsString() || !info[1]->IsObject()) {
        Nan::ThrowTypeError("Wrong arguments");
        return;
    }

    wstring filename(reinterpret_cast<wchar_t*>(*String::Value(info[0])));

    PlaySoundOption option;
    const auto outputValue = info[1]->ToObject()->Get(Nan::New("output").ToLocalChecked());
    option.useDefaultOutput = !outputValue->IsObject();
    if (outputValue->IsObject()) {
        const auto outputObj = outputValue->ToObject();
        const auto outputId = outputObj->Get(Nan::New("id").ToLocalChecked())->ToString();
        option.output.id = wstring(reinterpret_cast<wchar_t*>(*String::Value(outputId)));
    }

    try {
        NativeSoundPlayer::Play(filename, option);
    } catch (const RuntimeError& e) {
        Nan::ThrowError(encode(e.message()));
    }
}

void WrapperGetDevices(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    try {
        vector<Device> devices;
        NativeSoundPlayer::GetDevices(devices);

        auto arr = Nan::New<v8::Array>();
        uint32_t i = 0;
        for (const auto& device : devices) {
            auto obj = Nan::New<v8::Object>();

            obj->Set(Nan::New("id").ToLocalChecked(), encode(device.id));
            obj->Set(Nan::New("name").ToLocalChecked(), encode(device.name));

            arr->Set(Nan::New(i++), obj);
        }

        info.GetReturnValue().Set(arr);
    } catch (const RuntimeError& e) {
        Nan::ThrowError(encode(e.message()));
    }
}

void Init(v8::Local<v8::Object> exports) {
    NativeSoundPlayer::Init();

    exports->Set(Nan::New("play").ToLocalChecked(),
        Nan::New<FunctionTemplate>(WrapperPlay)->GetFunction());

    exports->Set(Nan::New("getDevices").ToLocalChecked(),
        Nan::New<FunctionTemplate>(WrapperGetDevices)->GetFunction());
}

NODE_MODULE(wrapper, ::Init)
