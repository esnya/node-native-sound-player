#include <exception>
#include <string>
#include <vector>

namespace NativeSoundPlayer {
    struct DeviceOption {
        std::wstring id;
    };

    struct PlaySoundOption {
        bool useDefaultOutput;
        DeviceOption output;
    };

    struct Device {
        std::wstring id;
        std::wstring name;
    };

    void Init();
    void GetDevices(std::vector<Device>& devices);
    void* Play(const std::wstring& filename, const PlaySoundOption& option);
    void Stop(void* handle);
    bool GetIsPlaying(void* handle);
    void Release(void* handle);

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
