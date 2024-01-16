#include "audio_stream_gdmpt.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/classes/file_access.hpp>

#include <type_traits>

using namespace godot;

// Does nothing. Could hook to Godot's logging functions.
struct Logger: public std::ostream {
    Logger() : std::ostream(NULL) {}
};

Ref<AudioStreamGDMPT> AudioStreamGDMPT::load_from_buffer(
    const PackedByteArray& buffer) {
    Ref<AudioStreamGDMPT> stream;

    // Needs to be accessible by `module_ext` even if `AudioStreamGDMPT` gets
    // moved around. Static is the simplest but this could be added as a member to
    // `AudioStreamGDMPT` if we're assuming a `Ref<AudioStreamGDMPT>` has a fixed
    // location in the heap.
    static Logger logger;

    std::string msg = "Unable to create OpenMPT module from buffer: ";

    try {
        // This presumably copies the buffer internally
        stream->module = std::make_unique<openmpt::module_ext>(buffer.ptr(), static_cast<std::size_t>(buffer.size()), logger);

        stream->interactive = static_cast<openmpt::ext::interactive*>(stream->module->get_interface(openmpt::ext::interactive_id));

        if (stream->interactive == nullptr) {
            msg += "`get_interface` returned a `nullptr`";
            ERR_FAIL_V_EDMSG(nullptr, msg.c_str());
        }
    } catch (const openmpt::exception& e) {
        msg += e.what();
        ERR_FAIL_V_EDMSG(nullptr, msg.c_str());
    }

    return stream;
}

Ref<AudioStreamPlayback> AudioStreamGDMPT::load_from_file(
    const String& path) {
    auto file_data = FileAccess::get_file_as_bytes(path);
	ERR_FAIL_COND_V_MSG(file_data.is_empty(), nullptr, "Cannot open file '" + path + "'.");
	return load_from_buffer(file_data);
}

void AudioStreamGDMPT::set_loop(bool enable) {
    if (enable) {
        module->set_repeat_count(-1);
    } else {
        module->set_repeat_count(0);
    }
}

bool AudioStreamGDMPT::get_loop() const {
    return module->get_repeat_count() == -1;
}

void AudioStreamGDMPT::set_tempo_factor(double factor) {
    interactive->set_tempo_factor(factor);
}

double AudioStreamGDMPT::get_tempo_factor() const {
    return interactive->get_tempo_factor();
}

Ref<AudioStreamPlayback> AudioStreamGDMPT::_instantiate_playback() const {
    Ref<AudioStreamGDMPTPlayback> playback;

    ERR_FAIL_COND_V(module == nullptr, nullptr);

    playback.instantiate();
    playback->stream = Ref<AudioStreamGDMPT>(this);
    playback->active = false;

    return playback;
}

String AudioStreamGDMPT::_get_stream_name() const {
    return "";
}

double AudioStreamGDMPT::_get_length() const {
    double length = module->get_duration_seconds();

    // `get_duration_seconds` returns infinity "if the pattern data is too
    // complex". We can return 0.0 here to signal that we don't support
    // `get_length` according to comment here:
    //
    // https://github.com/godotengine/godot/blob/9e65c5c0f4f8944d17fc7f5b05682206e9348d81/modules/vorbis/audio_stream_ogg_vorbis.h#L151
    if (std::isfinite(length)) {
        return length;
    } else {
        return 0.0;
    }
}

bool AudioStreamGDMPT::_is_monophonic() const {
    return false;
}

double AudioStreamGDMPT::_get_bpm() const {
    return module->get_current_estimated_bpm();
}

int32_t AudioStreamGDMPT::_get_beat_count() const {
    return 0;
}

void AudioStreamGDMPT::_bind_methods() {
    ClassDB::bind_static_method("AudioStreamGDMPT", D_METHOD("load_from_buffer", "buffer"), &AudioStreamGDMPT::load_from_buffer);
	ClassDB::bind_static_method("AudioStreamGDMPT", D_METHOD("load_from_file", "path"), &AudioStreamGDMPT::load_from_file);

    ClassDB::bind_method(D_METHOD("set_loop", "enable"), &AudioStreamGDMPT::set_loop);
	ClassDB::bind_method(D_METHOD("get_loop"), &AudioStreamGDMPT::get_loop);

    ClassDB::bind_method(D_METHOD("set_tempo_factor", "factor"), &AudioStreamGDMPT::set_tempo_factor);
	ClassDB::bind_method(D_METHOD("get_tempo_factor"), &AudioStreamGDMPT::get_tempo_factor);

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop"), "set_loop", "get_loop");
}

AudioStreamGDMPT::AudioStreamGDMPT() {}

////////////////

void AudioStreamGDMPTPlayback::_start(double from_pos) {
    active = true;
    _seek(from_pos);
}

void AudioStreamGDMPTPlayback::_stop() {
    active = false;
}

bool AudioStreamGDMPTPlayback::_is_playing() const {
    return active;
}

int32_t AudioStreamGDMPTPlayback::_get_loop_count() const {
    // No easy way to keep track of the number of loops
    return 0;
}

double AudioStreamGDMPTPlayback::_get_playback_position() const {
    return stream->module->get_position_seconds();
}

void AudioStreamGDMPTPlayback::_seek(double position) {
    stream->module->set_position_seconds(position);
}

int32_t AudioStreamGDMPTPlayback::_mix(AudioFrame* buffer,
                                       double rate_scale,
                                       int32_t frames) {
    // Assuming the pointer is also aligned properly.
    // Usually not important for x86/x64 but writing to non-aligned memory
    // would segfault in ARM.
    static_assert(std::alignment_of<AudioFrame>::value == 4);

    return stream->module->read_interleaved_stereo(
        static_cast<int32_t>(rate_scale),
        static_cast<size_t>(frames),
        reinterpret_cast<float*>(buffer));
}

void AudioStreamGDMPTPlayback::_bind_methods() {
    // Purposely empty
}

AudioStreamGDMPTPlayback::AudioStreamGDMPTPlayback() {}

////////////////

