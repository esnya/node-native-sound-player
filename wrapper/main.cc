#include <nan.h>
#include <memory>
#include "wrapper.h"

using namespace std;
using namespace v8;
using namespace NativeSoundPlayer;

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
        Local<String> msg;
        const auto ptr = reinterpret_cast<const uint16_t*>(e.what_w());
        Nan::ThrowError(Nan::Encode(ptr, e.message().length(), Nan::Encoding::UCS2));
    }
}

void Init(v8::Local<v8::Object> exports) {
    NativeSoundPlayer::Init();

    exports->Set(Nan::New("play").ToLocalChecked(),
        Nan::New<FunctionTemplate>(WrapperPlay)->GetFunction());
}

NODE_MODULE(wrapper, ::Init)
