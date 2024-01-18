#include "audio_stream_gdmpt.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <type_traits>

using namespace godot;

// Godot's sampling rate
// https://docs.godotengine.org/en/4.2/contributing/development/core_and_modules/custom_audiostreams.html
constexpr double SAMPLING_RATE = 44100.0;

Ref<AudioStreamGDMPT> AudioStreamGDMPT::load_from_buffer(
		const PackedByteArray &buffer) {
	Ref<AudioStreamGDMPT> stream;
	stream.instantiate();

	int error;

	// Returns a pointer that *must* be freed with `openmpt_module_ext_destroy`.
	// Code below is ensuring this using a `std::unique_ptr` with a custom
	// deleter. Presumably copies the buffer internally so we don't need to
	// store/copy `buffer`.
	auto module = openmpt_module_ext_create_from_memory(
			buffer.ptr(),
			static_cast<std::size_t>(buffer.size()),
			openmpt_log_func_silent,
			nullptr,
			openmpt_error_func_ignore,
			nullptr,
			&error,
			nullptr,
			nullptr);
	if (module == nullptr) {
		std::string msg = "Unable to create OpenMPT module from buffer: ";

		// Allocates a string that must be freed with `openmpt_free_string`.
		const char *err_msg = openmpt_error_string(error);

		if (err_msg == nullptr) {
			msg += "Out of memory while allocating string";
		} else {
			msg += err_msg;
			openmpt_free_string(err_msg);
		}
		ERR_FAIL_V_EDMSG(nullptr, msg.c_str());
	}
	stream->module = ModuleExtUniquePtr(module);

	stream->interactive =
			std::make_unique<openmpt_module_ext_interface_interactive>();
	if (stream->interactive == nullptr) {
		ERR_FAIL_V_EDMSG(nullptr,
				"Failed to allocate memory for "
				"`openmpt_module_ext_interface_interactive`");
	}

	error = openmpt_module_ext_get_interface(
			stream->module.get(),
			LIBOPENMPT_EXT_C_INTERFACE_INTERACTIVE,
			stream->interactive.get(),
			sizeof(openmpt_module_ext_interface_interactive));
	if (error == 0) {
		ERR_FAIL_V_EDMSG(nullptr, "Unable to get interface from module_ext");
	}

	return stream;
}

Ref<AudioStreamGDMPT> AudioStreamGDMPT::load_from_file(const String &path) {
	auto file_data = FileAccess::get_file_as_bytes(path);
	ERR_FAIL_COND_V_MSG(
			file_data.is_empty(), nullptr, "Cannot open file '" + path + "'.");
	return load_from_buffer(file_data);
}

void AudioStreamGDMPT::set_loop(bool enable) {
	ERR_FAIL_COND_V(module == nullptr, void());

	int32_t repeat_count;
	if (enable) {
		repeat_count = -1;
	} else {
		repeat_count = 0;
	}
	int error = openmpt_module_set_repeat_count(
			reinterpret_cast<openmpt_module *>(module.get()), repeat_count);
	if (error == 0) {
		ERR_FAIL_V_EDMSG(void(), "openmpt_module_set_repeat_count failed");
	}
}

bool AudioStreamGDMPT::get_loop() const {
	ERR_FAIL_COND_V(module == nullptr, false);

	// `openmpt_module_get_repeat_count` does not report errors
	int repeat_count = openmpt_module_get_repeat_count(
			reinterpret_cast<openmpt_module *>(module.get()));
	return repeat_count == -1;
}

void AudioStreamGDMPT::set_tempo_factor(double factor) {
	ERR_FAIL_COND_V(interactive == nullptr, void());

	int error = interactive->set_tempo_factor(module.get(), factor);
	if (error == 0) {
		ERR_FAIL_V_EDMSG(void(), "set_tempo_factor failed");
	}
}

double AudioStreamGDMPT::get_tempo_factor() const {
	ERR_FAIL_COND_V(interactive == nullptr, 0.0);

	// `get_tempo_factor` does not report errors
	return interactive->get_tempo_factor(module.get());
}

Ref<AudioStreamPlayback> AudioStreamGDMPT::_instantiate_playback() const {
	ERR_FAIL_COND_V(module == nullptr, nullptr);

	Ref<AudioStreamGDMPTPlayback> playback;
	playback.instantiate();

	playback->stream = Ref<AudioStreamGDMPT>(this);
	playback->active = false;

	return playback;
}

String AudioStreamGDMPT::_get_stream_name() const { return ""; }

double AudioStreamGDMPT::_get_length() const {
	ERR_FAIL_COND_V(module == nullptr, 0.0);

	// `openmpt_module_get_duration_seconds` does not report errors
	double length = openmpt_module_get_duration_seconds(
			reinterpret_cast<openmpt_module *>(module.get()));

	// `openmpt_module_get_duration_seconds` returns infinity "if the pattern
	// data is too complex". We can return 0.0 here to signal that we don't
	// support `get_length` according to comment here:
	//
	// https://github.com/godotengine/godot/blob/9e65c5c0f4f8944d17fc7f5b05682206e9348d81/modules/vorbis/audio_stream_ogg_vorbis.h#L151
	if (std::isfinite(length)) {
		return length;
	} else {
		return 0.0;
	}
}

bool AudioStreamGDMPT::_is_monophonic() const {
	// `AudioStreamGDMPT` is stereo
	return false;
}

double AudioStreamGDMPT::_get_bpm() const {
	ERR_FAIL_COND_V(module == nullptr, 0.0);

	// `openmpt_module_get_current_estimated_bpm` does not report errors
	return openmpt_module_get_current_estimated_bpm(
			reinterpret_cast<openmpt_module *>(module.get()));
}

int32_t AudioStreamGDMPT::_get_beat_count() const { return 0; }

void AudioStreamGDMPT::_bind_methods() {
	ClassDB::bind_static_method("AudioStreamGDMPT",
			D_METHOD("load_from_buffer", "buffer"),
			&AudioStreamGDMPT::load_from_buffer);
	ClassDB::bind_static_method("AudioStreamGDMPT",
			D_METHOD("load_from_file", "path"),
			&AudioStreamGDMPT::load_from_file);

	ClassDB::bind_method(D_METHOD("set_loop", "enable"),
			&AudioStreamGDMPT::set_loop);
	ClassDB::bind_method(D_METHOD("get_loop"), &AudioStreamGDMPT::get_loop);

	ClassDB::bind_method(D_METHOD("set_tempo_factor", "factor"),
			&AudioStreamGDMPT::set_tempo_factor);
	ClassDB::bind_method(D_METHOD("get_tempo_factor"),
			&AudioStreamGDMPT::get_tempo_factor);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop"), "set_loop", "get_loop");
}

AudioStreamGDMPT::AudioStreamGDMPT() {}

////////////////

void AudioStreamGDMPTPlayback::_start(double from_pos) {
	active = true;
	_seek(from_pos);
}

void AudioStreamGDMPTPlayback::_stop() { active = false; }

bool AudioStreamGDMPTPlayback::_is_playing() const { return active; }

int32_t AudioStreamGDMPTPlayback::_get_loop_count() const {
	// No easy way to keep track of the number of loops
	return 0;
}

double AudioStreamGDMPTPlayback::_get_playback_position() const {
	ERR_FAIL_COND_V(stream == nullptr, 0.0);

	// `openmpt_module_get_position_seconds` does not report errors
	return openmpt_module_get_position_seconds(
			reinterpret_cast<openmpt_module *>(stream->module.get()));
}

void AudioStreamGDMPTPlayback::_seek(double position) {
	ERR_FAIL_COND_V(stream == nullptr, void());

	// `openmpt_module_set_position_seconds` does not report errors
	openmpt_module_set_position_seconds(
			reinterpret_cast<openmpt_module *>(stream->module.get()), position);
}

int32_t AudioStreamGDMPTPlayback::_mix_resampled(AudioFrame *dst_buffer,
		int32_t frame_count) {
	// Check if `dst_buffer` and the input to
	// `openmpt_module_read_interleaved_float_stereo` have the same alignment.
	//
	// Usually not important for x86/x64 but writing to non-aligned memory
	// would segfault in ARM.
	static_assert(std::alignment_of<AudioFrame>::value ==
			std::alignment_of<float>::value);

	return openmpt_module_read_interleaved_float_stereo(
			reinterpret_cast<openmpt_module *>(stream->module.get()),
			static_cast<int32_t>(SAMPLING_RATE),
			static_cast<size_t>(frame_count),
			reinterpret_cast<float *>(dst_buffer));
}

double AudioStreamGDMPTPlayback::_get_stream_sampling_rate() const {
	return SAMPLING_RATE;
}

void AudioStreamGDMPTPlayback::_bind_methods() {
	// Purposely empty
}

AudioStreamGDMPTPlayback::AudioStreamGDMPTPlayback() {}

////////////////
