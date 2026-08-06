#ifndef __MIDI_BACKEND_H__
#define __MIDI_BACKEND_H__

namespace miniaudioengine::framework
{

class IMidiBackend
{
public:
    virtual ~IMidiBackend() = default;
};

} // namespace miniaudioengine::framework

#endif // __MIDI_BACKEND_H__