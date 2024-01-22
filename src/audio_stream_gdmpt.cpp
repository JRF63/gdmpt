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
	auto ptr = openmpt_module_ext_create_from_memory(
			buffer.ptr(),
			static_cast<std::size_t>(buffer.size()),
			openmpt_log_func_silent,
			nullptr,
			openmpt_error_func_ignore,
			nullptr,
			&error,
			nullptr,
			nullptr);
	if (ptr == nullptr) {
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
	auto module = ModuleExtUniquePtr(ptr);

	auto interactive =
			std::make_unique<openmpt_module_ext_interface_interactive>();
	if (interactive == nullptr) {
		ERR_FAIL_V_EDMSG(nullptr,
				"Failed to allocate memory for "
				"`openmpt_module_ext_interface_interactive`");
	}

	error = openmpt_module_ext_get_interface(
			module.get(),
			LIBOPENMPT_EXT_C_INTERFACE_INTERACTIVE,
			interactive.get(),
			sizeof(openmpt_module_ext_interface_interactive));
	if (error == 0) {
		ERR_FAIL_V_EDMSG(nullptr, "Unable to get interface from module_ext");
	}

	stream->module.set_pointers(std::move(module), std::move(interactive));

	return stream;
}

Ref<AudioStreamGDMPT> AudioStreamGDMPT::load_from_file(const String &path) {
	auto file_data = FileAccess::get_file_as_bytes(path);
	ERR_FAIL_COND_V_EDMSG(
			file_data.is_empty(), nullptr, "Cannot open file '" + path + "'.");
	return load_from_buffer(file_data);
}

void AudioStreamGDMPT::set_loop(bool enable) {
	ERR_FAIL_COND(module.is_null());

	int32_t repeat_count;
	if (enable) {
		repeat_count = -1;
	} else {
		repeat_count = 0;
	}
	int error = module.set_repeat_count(repeat_count);
	if (error == 0) {
		ERR_FAIL_EDMSG("`openmpt_module_set_repeat_count` failed");
	}
}

bool AudioStreamGDMPT::get_loop() const {
	ERR_FAIL_COND_V(module.is_null(), false);

	auto repeat_count = module.get_repeat_count();
	if (repeat_count) {
		return repeat_count.value() == -1;
	} else {
		ERR_FAIL_V_EDMSG(false, "`openmpt_module_get_repeat_count` failed");
	}
}

void AudioStreamGDMPT::set_tempo_factor(double factor) {
	ERR_FAIL_COND(module.is_null());

	int error = module.set_tempo_factor(factor);
	if (error == 0) {
		ERR_FAIL_EDMSG("`set_tempo_factor` failed");
	}
}

double AudioStreamGDMPT::get_tempo_factor() const {
	ERR_FAIL_COND_V(module.is_null(), 0.0);

	auto tempo_factor = module.get_tempo_factor();
	if (tempo_factor) {
		return tempo_factor.value();
	} else {
		ERR_FAIL_V_EDMSG(0.0, "`get_tempo_factor` failed");
	}
}

Ref<AudioStreamPlayback> AudioStreamGDMPT::_instantiate_playback() const {
	ERR_FAIL_COND_V(module.is_null(), nullptr);

	Ref<AudioStreamGDMPTPlayback> playback;
	playback.instantiate();

	playback->stream = Ref<AudioStreamGDMPT>(this);
	playback->active = false;

	return playback;
}

String AudioStreamGDMPT::_get_stream_name() const { return ""; }

double AudioStreamGDMPT::_get_length() const {
	ERR_FAIL_COND_V(module.is_null(), 0.0);

	auto length = module.get_duration_seconds();
	if (length) {
		// `openmpt_module_get_duration_seconds` returns infinity "if the pattern
		// data is too complex". We can return 0.0 here to signal that we don't
		// support `get_length` according to comment here:
		//
		// https://github.com/godotengine/godot/blob/9e65c5c0f4f8944d17fc7f5b05682206e9348d81/modules/vorbis/audio_stream_ogg_vorbis.h#L151
		if (std::isfinite(length.value())) {
			return length.value();
		} else {
			return 0.0;
		}
	} else {
		ERR_FAIL_V_EDMSG(0.0, "`openmpt_module_get_duration_seconds` failed");
	}
}

bool AudioStreamGDMPT::_is_monophonic() const {
	// `AudioStreamGDMPT` is stereo
	return false;
}

double AudioStreamGDMPT::_get_bpm() const {
	ERR_FAIL_COND_V(module.is_null(), 0.0);

	auto bpm = module.get_current_estimated_bpm();
	if (bpm) {
		return bpm.value();
	} else {
		ERR_FAIL_V_EDMSG(0, "`openmpt_module_get_current_estimated_bpm` failed");
	}
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

	auto position = stream->module.get_position_seconds();
	if (position) {
		return position.value();
	} else {
		ERR_FAIL_V_EDMSG(0.0, "`openmpt_module_get_position_seconds` failed");
	}
}

void AudioStreamGDMPTPlayback::_seek(double position) {
	ERR_FAIL_COND(stream == nullptr);

	auto new_position = stream->module.set_position_seconds(position);
	if (!new_position) {
		ERR_FAIL_EDMSG("`openmpt_module_set_position_seconds` failed");
	}
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

	auto frames_rendered = stream->module.read_interleaved_float_stereo(
			static_cast<int32_t>(SAMPLING_RATE),
			static_cast<size_t>(frame_count),
			reinterpret_cast<float *>(dst_buffer));
	if (frames_rendered) {
		return frames_rendered.value();
	} else {
		ERR_FAIL_V_EDMSG(0, "`openmpt_module_read_interleaved_float_stereo` failed");
	}
}

double AudioStreamGDMPTPlayback::_get_stream_sampling_rate() const {
	return SAMPLING_RATE;
}

void AudioStreamGDMPTPlayback::_bind_methods() {
	// Purposely empty
}

AudioStreamGDMPTPlayback::AudioStreamGDMPTPlayback() {}

////////////////
