#include "file.h"
#include "fileadapter.h"

#include <sndfile.h>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace miniaudioengine
{

// =============================================================================
// File::Impl — defined here so sndfile headers stay out of the public API
// =============================================================================

struct File::Impl
{
  File::eFileType file_type;
  std::filesystem::path filepath;
  adapters::FileAdapter file_adapter;
};

// =============================================================================
// File — member implementations
// =============================================================================

File::File() : framework::IInputOutput(framework::eInputOutputType::File) {}

File::File(std::unique_ptr<Impl> impl) : framework::IInputOutput(framework::eInputOutputType::File),
                                         p_impl(std::move(impl)) {}

File::~File() = default;

File::eFileType File::get_file_type() const { return p_impl->file_type; }

std::filesystem::path File::get_filepath() const { return p_impl->filepath; }

std::string File::get_filename() const { return p_impl->filepath.filename().string(); }

unsigned int File::get_total_frames() const
{
  if (p_impl->file_type != eFileType::Wav) return 0u;
  return static_cast<unsigned int>(p_impl->file_adapter.get_info().frames);
}

unsigned int File::get_bits_per_sample() const
{
  if (p_impl->file_type != eFileType::Wav) return 0u;
  const int fmt = p_impl->file_adapter.get_info().format & SF_FORMAT_SUBMASK;
  switch (fmt)
  {
    case SF_FORMAT_PCM_16: return 16u;
    case SF_FORMAT_PCM_24: return 24u;
    case SF_FORMAT_PCM_32: return 32u;
    case SF_FORMAT_FLOAT:  return 32u;
    case SF_FORMAT_DOUBLE: return 64u;
    default:               return 0u;
  }
}

unsigned int File::get_sample_rate() const
{
  if (p_impl->file_type != eFileType::Wav) return 0u;
  return static_cast<unsigned int>(p_impl->file_adapter.get_info().samplerate);
}

unsigned int File::get_channels() const
{
  if (p_impl->file_type != eFileType::Wav) return 0u;
  return static_cast<unsigned int>(p_impl->file_adapter.get_info().channels);
}

double File::get_duration_seconds() const
{
  if (p_impl->file_type != eFileType::Wav) return 0.0;
  const unsigned int sr = get_sample_rate();
  return sr > 0 ? static_cast<double>(get_total_frames()) / static_cast<double>(sr) : 0.0;
}

std::string File::get_format_string() const
{
  if (p_impl->file_type != eFileType::Wav) return {};
  switch (p_impl->file_adapter.get_info().format & SF_FORMAT_TYPEMASK)
  {
    case SF_FORMAT_WAV:  return "WAV";
    case SF_FORMAT_AIFF: return "AIFF";
    case SF_FORMAT_FLAC: return "FLAC";
    default:             return "Unknown";
  }
}

bool File::is_stream_open()
{
  return p_impl->file_adapter.is_stream_open();
}

bool File::close_stream()
{
  return p_impl->file_adapter.close_stream();
}

bool File::open_stream(const framework::StreamConfig &config)
{
  framework::StreamConfig stream_config = config;
  stream_config.direction = get_direction();
  return p_impl->file_adapter.open_stream(get_filepath(), stream_config);
}

std::string File::to_string() const
{
  if (p_impl->file_type == eFileType::Wav)
  {
    return "File(Type=Wav"
           ", Path=" + p_impl->filepath.string() +
           ", TotalFrames=" + std::to_string(get_total_frames()) +
           ", DurationSeconds=" + std::to_string(get_duration_seconds()) +
           ", Format=" + get_format_string() +
           ", SampleRate=" + std::to_string(get_sample_rate()) +
           ", BitsPerSample=" + std::to_string(get_bits_per_sample()) +
           ", Channels=" + std::to_string(get_channels()) + ")";
  }
  return "File(Type=Midi, Path=" + p_impl->filepath.string() + ")";
}

// =============================================================================
// FileHandleFactory
// =============================================================================

FilePtr FileHandleFactory::make_wav(const std::filesystem::path& path)
{
  auto impl = std::make_unique<File::Impl>();
  impl->file_type = File::eFileType::Wav;
  impl->filepath  = path;

  // Read the format up front so sample rate, channels and frame count are
  // available before a stream is opened.
  if (!impl->file_adapter.probe(path))
  {
    return nullptr;
  }

  return FilePtr(new File(std::move(impl)));
}

FilePtr FileHandleFactory::make_midi(const std::filesystem::path& path)
{
  auto impl = std::make_unique<File::Impl>();
  impl->file_type = File::eFileType::Midi;
  impl->filepath  = path;
  return FilePtr(new File(std::move(impl)));
}

} // namespace miniaudioengine
