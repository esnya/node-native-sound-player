#include "wrapper.h"

using namespace std;

namespace NativeSoundPlayer {
    RuntimeError::RuntimeError(const wstring& msg) {
        _message = msg;
    }

    RuntimeError::~RuntimeError() {
    }

    const wchar_t* RuntimeError::what_w() const {
        return _message.c_str();
    }

    const wstring RuntimeError::message() const {
        return _message;
    }
}
