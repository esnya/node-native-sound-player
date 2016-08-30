#include <exception>
#include <string>

namespace NativeSoundPlayer {
    struct DeviceOption {
        std::wstring id;
    };

    struct PlaySoundOption {
        bool useDefaultOutput;
        DeviceOption output;
    };

    void Init();
    void Play(const std::wstring& filename, const PlaySoundOption& option);

    class RuntimeError : std::exception {
    public:
        RuntimeError(const std::wstring& msg);
        virtual ~RuntimeError();
        virtual const wchar_t* what_w() const;
        virtual const std::wstring message() const;
    private:
        std::wstring _message;
    };
}
