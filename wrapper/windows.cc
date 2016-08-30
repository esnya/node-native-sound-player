#include "wrapper.h"

#define UNICODE

#include <algorithm>
#include <iterator>
#include <comdef.h>
#include <wrl/client.h>
#include <objbase.h>
#include <mmdeviceapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <mfapi.h>
#include <comdef.h>
#include <functiondiscoverykeys_devpkey.h>

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")

using namespace std;
using namespace Microsoft::WRL;

namespace NativeSoundPlayer {
    // Throw If Failed
    inline void tif(HRESULT hr) {
        if (FAILED(hr)) {
            const _com_error error(hr);
            throw RuntimeError(wstring(error.ErrorMessage()));
        }
    }

    namespace Win {
        ComPtr<IMMDeviceCollection> CreateAudioEndpointCollection() {
            ComPtr<IMMDeviceEnumerator> enumerator;
            tif(CoCreateInstance(
                __uuidof(MMDeviceEnumerator),
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(&enumerator)
            ));

            ComPtr<IMMDeviceCollection> collection;
            tif(enumerator->EnumAudioEndpoints(
                EDataFlow::eRender,
                DEVICE_STATE_ACTIVE,
                &collection
            ));

            return collection;
        }

        void CreateDeviceVector(
            vector< ComPtr<IMMDevice> >& devices
        ) {
            const auto collection = CreateAudioEndpointCollection();

            UINT count;
            tif(collection->GetCount(&count));

            devices.clear();
            devices.reserve(count);
            for (UINT i = 0; i < count; i++) {
                ComPtr<IMMDevice> device;

                tif(collection->Item(i, &device));

                devices.push_back(device);
            }
        }

        void CreateDeviceStruct(
            vector<Device>& dst
        ) {
            vector< ComPtr<IMMDevice> > src;
            CreateDeviceVector(src); 

            dst.clear();
            dst.reserve(src.size());

            transform(
                src.begin(), src.end(),
                back_inserter(dst),
                [](ComPtr<IMMDevice> device) {
                    Device result;

                    LPWSTR id;
                    tif(device->GetId(&id));
                    result.id = wstring(id);
                    CoTaskMemFree(id);

                    ComPtr<IPropertyStore> props;
                    tif(device->OpenPropertyStore(STGM_READ, &props));

                    PROPVARIANT name;
                    PropVariantInit(&name);

                    tif(props->GetValue(PKEY_Device_FriendlyName, &name));
                    result.name = wstring(name.pwszVal);

                    PropVariantClear(&name);

                    return result;
                }
            );
        }

        ComPtr<IMFMediaSource> CreateMediaSource(const wstring& url) {
            ComPtr<IMFSourceResolver> sourceResolver;
            tif(MFCreateSourceResolver(&sourceResolver));

	        MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;
            ComPtr<IUnknown> mediaSourceUnknown;            
            tif(sourceResolver->CreateObjectFromURL(
                url.c_str(),
                MF_RESOLUTION_MEDIASOURCE,
                nullptr,
                &ObjectType,
                &mediaSourceUnknown));

            ComPtr<IMFMediaSource> mediaSource;
            tif(mediaSourceUnknown.Get()->QueryInterface(IID_PPV_ARGS(&mediaSource)));

            return mediaSource;
        }

        ComPtr<IMFPresentationDescriptor> CreatePresentationDescriptor(
            ComPtr<IMFMediaSource> source
        ) {
            ComPtr<IMFPresentationDescriptor> pd;
            tif(source->CreatePresentationDescriptor(&pd));

            return pd;
        }

        ComPtr<IMFActivate> CreateMediaSinkActivate() {
            ComPtr<IMFActivate> activate;
            tif(MFCreateAudioRendererActivate(&activate));

            return activate;
        }

        ComPtr<IMFMediaSink> CreateMediaSink(
            const wstring& deviceId
        ) {
            ComPtr<IMFAttributes> attributes;
            tif(MFCreateAttributes(&attributes, 1));
            tif(attributes->SetString(MF_AUDIO_RENDERER_ATTRIBUTE_ENDPOINT_ID, deviceId.c_str()));

            ComPtr<IMFMediaSink> mediaSink;
            tif(MFCreateAudioRenderer(attributes.Get(), &mediaSink));

            return mediaSink;
        }     

        ComPtr<IMFStreamSink> CreateStreamSink(
            ComPtr<IMFMediaSink> mediaSink
        ) {
            ComPtr<IMFStreamSink> streamSink;
            tif(mediaSink->GetStreamSinkByIndex(0, &streamSink));

            return streamSink;
        }

        ComPtr<IMFStreamDescriptor> CreateStreamDescriptor(
            ComPtr<IMFPresentationDescriptor> pd,
            DWORD streamId,
            BOOL& selected
        ) {
            ComPtr<IMFStreamDescriptor> sd;
            tif(pd->GetStreamDescriptorByIndex(streamId, &selected, &sd));
            
            return sd;
        }

        GUID GetMediaType(ComPtr<IMFStreamDescriptor> sourceSD) {
            ComPtr<IMFMediaTypeHandler> handler;
            tif(sourceSD->GetMediaTypeHandler(&handler));

            GUID majorType;
            tif(handler ->GetMajorType(&majorType));

            return majorType;
        }

        ComPtr<IMFTopologyNode> CreateSourceNode(
            ComPtr<IMFMediaSource> source,
            ComPtr<IMFPresentationDescriptor> pd,
            ComPtr<IMFStreamDescriptor> sd
        ) {
            ComPtr<IMFTopologyNode> node;
            tif(MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &node));

            tif(node->SetUnknown(MF_TOPONODE_SOURCE, source.Get()));
            tif(node->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pd.Get()));
            tif(node->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, sd.Get()));

            return node;
        }

        template <class ISink>
        ComPtr<IMFTopologyNode> _CreateOutputNode(
            ComPtr<ISink> sink,
            DWORD streamId
        ) {
            ComPtr<IMFTopologyNode> node;
            tif(MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &node));

            tif(node->SetObject(sink.Get()));
            tif(node->SetUINT32(MF_TOPONODE_STREAMID, streamId));
            tif(node->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE));
            
            return node;
        }
        ComPtr<IMFTopologyNode> CreateOutputNode(
            ComPtr<IMFActivate> sink,
            DWORD streamId
        ) {
            return _CreateOutputNode(sink, streamId);
        }
        ComPtr<IMFTopologyNode> CreateOutputNode(
            ComPtr<IMFStreamSink> sink,
            DWORD streamId
        ) {
            return _CreateOutputNode(sink, streamId);
        }

        template <class T>
        ComPtr<IMFTopology> _CreatePlayBackTopology(
            ComPtr<IMFMediaSource> mediaSource,
            ComPtr<T> sink
        ) {
            ComPtr<IMFTopology> topology;
            tif(MFCreateTopology(&topology));

            const auto pd = CreatePresentationDescriptor(mediaSource);

            DWORD numOfStreams;
            tif(pd->GetStreamDescriptorCount(&numOfStreams));
            for (DWORD streamId = 0; streamId < numOfStreams; streamId++) {
                BOOL selected = FALSE;

                const auto sd = CreateStreamDescriptor(pd, streamId, selected);

                if (!selected) continue;

                if (GetMediaType(sd) != MFMediaType_Audio) {
                    tif(MF_E_INVALIDMEDIATYPE);
                }

                const auto sourceNode = CreateSourceNode(mediaSource, pd, sd);
                tif(topology->AddNode(sourceNode.Get()));

                const auto outputNode = CreateOutputNode(sink, streamId);
                tif(topology->AddNode(outputNode.Get()));

                tif(sourceNode->ConnectOutput(0, outputNode.Get(), 0));
            }

            return topology;
        }
        ComPtr<IMFTopology> CreatePlayBackTopology(
            ComPtr<IMFMediaSource> mediaSource
        ) {
            const auto activate = CreateMediaSinkActivate();
            
            return _CreatePlayBackTopology(mediaSource, activate);
        }
        ComPtr<IMFTopology> CreatePlayBackTopology(
            ComPtr<IMFMediaSource> mediaSource,
            const wstring& deviceId
        ) {
            const auto mediaSink = CreateMediaSink(deviceId);
            const auto streamSink = CreateStreamSink(mediaSink);

            return _CreatePlayBackTopology(mediaSource, streamSink);
        }

        ComPtr<IMFMediaSession> CreateMediaSession(
            ComPtr<IMFTopology> topology
        ) {
            ComPtr<IMFMediaSession> mediaSession;
            tif(MFCreateMediaSession(nullptr, &mediaSession));

            mediaSession->SetTopology(0, topology.Get());

            return mediaSession;
        }

        void StartMediaSession(ComPtr<IMFMediaSession> mediaSession) {
            PROPVARIANT start;
            PropVariantInit(&start);

            tif(mediaSession->Start(&GUID_NULL, &start));

            PropVariantClear(&start);
        }

        void WatchMediaSessionEvent(ComPtr<IMFMediaSession> mediaSession) {
            while (1) {
                ComPtr<IMFMediaEvent> event;
                tif(mediaSession->GetEvent(0, &event));

                MediaEventType type;
                tif(event->GetType(&type));

                switch (type)
                {
                case MESessionEnded:
                    return;
                default:
                    break;
                }
            }
        }
    }

    void Init() {
        CoInitialize(nullptr);
        MFStartup(MF_VERSION);
    }

    void GetDevices(vector<Device>& devices) {
        Win::CreateDeviceStruct(devices);
    }

    void Play(const std::wstring& filename, const PlaySoundOption& option) {
        const auto mediaSource = Win::CreateMediaSource(filename);
        const auto topology = option.useDefaultOutput
            ? Win::CreatePlayBackTopology(mediaSource)
            : Win::CreatePlayBackTopology(mediaSource, option.output.id);
        const auto mediaSession = Win::CreateMediaSession(topology);

        Win::StartMediaSession(mediaSession);
        Win::WatchMediaSessionEvent(mediaSession);
    }
}