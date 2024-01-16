#include "audio_stream_gdmpt.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <type_traits>

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// Godot's sampling rate
// https://docs.godotengine.org/en/4.2/contributing/development/core_and_modules/custom_audiostreams.html
constexpr double SAMPLING_RATE = 44100.0;

const char *LOOPING_SIGNAL = "looped";

struct OpenMPTStringDeleter {
	void operator()(const char *p) { openmpt_free_string(p); }
};

using OpenMPTString =
		std::unique_ptr<const char, OpenMPTStringDeleter>;

#define OPENMPT_ERR_FAIL_V_EDMSG(obj, m_retval)   \
	auto err_msg = obj->pop_last_openmpt_error(); \
	ERR_FAIL_COND_V_EDMSG(err_msg.has_value(), m_retval, err_msg.value())

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
			AudioStreamGDMPT::error_func,
			stream.ptr(),
			&error,
			nullptr,
			nullptr);
	if (ptr == nullptr) {
		String msg = "Unable to create OpenMPT module from buffer: ";
		auto err_msg = OpenMPTString(openmpt_error_string(error));

		if (err_msg == nullptr) {
			msg += "Out of memory while allocating string";
		} else {
			msg += err_msg.get();
		}
		ERR_FAIL_V_EDMSG(nullptr, msg);
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

	for (int32_t i = 0; i < stream->get_num_channels(); i++) {
		stream->volume_settings.push_back(stream->get_channel_volume(i));
	}

	return stream;
}

Ref<AudioStreamGDMPT> AudioStreamGDMPT::load_from_file(const String &path) {
	auto file_data = FileAccess::get_file_as_bytes(path);
	ERR_FAIL_COND_V_EDMSG(
			file_data.is_empty(), nullptr, "Cannot open file '" + path + "'.");
	auto stream = load_from_buffer(file_data);
	stream->filename = path;
	return stream;
}

String AudioStreamGDMPT::get_filename() const {
	return filename;
}

void AudioStreamGDMPT::set_loop(bool enable) {
	loop = enable;
}

bool AudioStreamGDMPT::get_loop() const {
	return loop;
}

void AudioStreamGDMPT::set_tempo_factor(double factor) {
	ERR_FAIL_COND(module.is_null());

	module.set_tempo_factor(factor);
	OPENMPT_ERR_FAIL_V_EDMSG(this, void());
}

double AudioStreamGDMPT::get_tempo_factor() const {
	// This gets called by Godot to check for the default value
	if (module.is_null()) {
		return 1.0;
	}

	auto tempo_factor = module.get_tempo_factor();
	OPENMPT_ERR_FAIL_V_EDMSG(this, 0.0);
	return tempo_factor;
}

void AudioStreamGDMPT::set_pitch_factor(double factor) {
	ERR_FAIL_COND(module.is_null());

	module.set_pitch_factor(factor);
	OPENMPT_ERR_FAIL_V_EDMSG(this, void());
}

double AudioStreamGDMPT::get_pitch_factor() const {
	// Default value
	if (module.is_null()) {
		return 1.0;
	}

	auto pitch_factor = module.get_pitch_factor();
	OPENMPT_ERR_FAIL_V_EDMSG(this, 0.0);
	return pitch_factor;
}

void AudioStreamGDMPT::set_interpolation_filter(InterpolationFilter filter) {
	ERR_FAIL_COND(module.is_null());

	module.set_interpolation_filter(filter);
	OPENMPT_ERR_FAIL_V_EDMSG(this, void());
}

AudioStreamGDMPT::InterpolationFilter AudioStreamGDMPT::get_interpolation_filter() const {
	// Default value
	if (module.is_null()) {
		return InterpolationFilter::DEFAULT_INTERPOLATION;
	}

	int32_t value;
	module.get_interpolation_filter(&value);
	OPENMPT_ERR_FAIL_V_EDMSG(this, InterpolationFilter::DEFAULT_INTERPOLATION);
	return static_cast<InterpolationFilter>(value);
}

int32_t AudioStreamGDMPT::get_num_channels() const {
	ERR_FAIL_COND_V(module.is_null(), 0);

	auto num_channels = module.get_num_channels();
	OPENMPT_ERR_FAIL_V_EDMSG(this, 0);
	return num_channels;
}

void AudioStreamGDMPT::set_channel_volume(int32_t channel, double volume) {
	ERR_FAIL_COND(module.is_null());

	module.set_channel_volume(channel, volume);
	OPENMPT_ERR_FAIL_V_EDMSG(this, void());
	volume_settings[channel] = volume;
}

double AudioStreamGDMPT::get_channel_volume(int32_t channel) const {
	ERR_FAIL_COND_V(module.is_null(), 0.0);

	auto volume = module.get_channel_volume(channel);
	OPENMPT_ERR_FAIL_V_EDMSG(this, 0.0);
	return volume;
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
	OPENMPT_ERR_FAIL_V_EDMSG(this, 0.0);
	return length;
}

bool AudioStreamGDMPT::_is_monophonic() const {
	// `AudioStreamGDMPT` is stereo
	return false;
}

double AudioStreamGDMPT::_get_bpm() const {
	ERR_FAIL_COND_V(module.is_null(), 0.0);

	auto bpm = module.get_current_estimated_bpm();
	OPENMPT_ERR_FAIL_V_EDMSG(this, 0.0);
	return bpm;
}

int32_t AudioStreamGDMPT::_get_beat_count() const {
	// Unsupported
	return 0;
}

void AudioStreamGDMPT::emit_looping_signal() {
	// Signal was registered on this class so it has to be sent from here too
	emit_signal(LOOPING_SIGNAL);
}

std::optional<String> AudioStreamGDMPT::pop_last_openmpt_error() const {
	if (openmpt_error == OPENMPT_ERROR_OK) {
		return std::nullopt;
	} else {
		auto err_msg = OpenMPTString(openmpt_error_string(openmpt_error));

		// Also clear the error so we don't keep returning it
		openmpt_error = OPENMPT_ERROR_OK;

		return String(err_msg.get());
	}
}

int AudioStreamGDMPT::error_func(int error, void *ptr) {
	auto stream = reinterpret_cast<AudioStreamGDMPT *>(ptr);
	stream->openmpt_error = error;
	return OPENMPT_ERROR_FUNC_RESULT_NONE;
}

void AudioStreamGDMPT::_bind_methods() {
	ClassDB::bind_static_method("AudioStreamGDMPT",
			D_METHOD("load_from_buffer", "buffer"),
			&AudioStreamGDMPT::load_from_buffer);
	ClassDB::bind_static_method("AudioStreamGDMPT",
			D_METHOD("load_from_file", "path"),
			&AudioStreamGDMPT::load_from_file);

	ClassDB::bind_method(D_METHOD("get_filename"), &AudioStreamGDMPT::get_filename);

	ClassDB::bind_method(D_METHOD("set_loop", "enable"),
			&AudioStreamGDMPT::set_loop);
	ClassDB::bind_method(D_METHOD("get_loop"), &AudioStreamGDMPT::get_loop);

	ClassDB::bind_method(D_METHOD("set_tempo_factor", "factor"),
			&AudioStreamGDMPT::set_tempo_factor);
	ClassDB::bind_method(D_METHOD("get_tempo_factor"),
			&AudioStreamGDMPT::get_tempo_factor);

	ClassDB::bind_method(D_METHOD("set_pitch_factor", "factor"),
			&AudioStreamGDMPT::set_pitch_factor);
	ClassDB::bind_method(D_METHOD("get_pitch_factor"),
			&AudioStreamGDMPT::get_pitch_factor);

	ClassDB::bind_method(D_METHOD("set_interpolation_filter", "filter"),
			&AudioStreamGDMPT::set_interpolation_filter);
	ClassDB::bind_method(D_METHOD("get_interpolation_filter"),
			&AudioStreamGDMPT::get_interpolation_filter);

	ClassDB::bind_method(D_METHOD("get_num_channels"),
			&AudioStreamGDMPT::get_num_channels);

	ClassDB::bind_method(D_METHOD("set_channel_volume", "channel", "volume"),
			&AudioStreamGDMPT::set_channel_volume);
	ClassDB::bind_method(D_METHOD("get_channel_volume", "channel"),
			&AudioStreamGDMPT::get_channel_volume);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop"), "set_loop", "get_loop");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "tempo_factor"), "set_tempo_factor", "get_tempo_factor");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "pitch_factor"), "set_pitch_factor", "get_pitch_factor");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "interpolation_filter"), "set_interpolation_filter", "get_interpolation_filter");

	ADD_SIGNAL(MethodInfo(LOOPING_SIGNAL));

	BIND_ENUM_CONSTANT(DEFAULT_INTERPOLATION);
	BIND_ENUM_CONSTANT(NO_INTERPOLATION);
	BIND_ENUM_CONSTANT(LINEAR_INTERPOLATION);
	BIND_ENUM_CONSTANT(CUBIC_INTERPOLATION);
	BIND_ENUM_CONSTANT(SINC_INTERPOLATION);
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
	return loops;
}

double AudioStreamGDMPTPlayback::_get_playback_position() const {
	ERR_FAIL_NULL_V(stream, 0.0);
	ERR_FAIL_COND_V(stream->module.is_null(), 0.0);

	auto position = stream->module.get_position_seconds();
	OPENMPT_ERR_FAIL_V_EDMSG(stream, 0.0);
	return position;
}

void AudioStreamGDMPTPlayback::_seek(double position) {
	ERR_FAIL_NULL(stream);
	ERR_FAIL_COND(stream->module.is_null());

	auto new_position = stream->module.set_position_seconds(position);
	OPENMPT_ERR_FAIL_V_EDMSG(stream, void());
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

	ERR_FAIL_NULL_V(stream, 0);
	ERR_FAIL_COND_V(stream->module.is_null(), 0);

	// Guard against potential infinite loop
	int loop_guard = 0;

	int32_t total_rendered = 0;
	int32_t remaining_frames = frame_count;

	while (total_rendered < frame_count && loop_guard < 3) {
		loop_guard++;
		auto frames_rendered = stream->module.read_interleaved_float_stereo(
				static_cast<int32_t>(SAMPLING_RATE),
				static_cast<size_t>(remaining_frames),
				reinterpret_cast<float *>(dst_buffer + total_rendered));
		OPENMPT_ERR_FAIL_V_EDMSG(stream, 0);

		total_rendered += frames_rendered;
		remaining_frames -= frames_rendered;

		bool end_of_song = frames_rendered == 0;
		if (end_of_song && stream->loop) {
			loops++;
			_seek(0.0);
			// `set_position_seconds` resets the volume
			for (int i = 0; i < stream->volume_settings.size(); i++) {
				stream->set_channel_volume(i, stream->volume_settings[i]);
			}
			stream->emit_looping_signal();
		}
	}
	return total_rendered;
}

double AudioStreamGDMPTPlayback::_get_stream_sampling_rate() const {
	return SAMPLING_RATE;
}

void AudioStreamGDMPTPlayback::_bind_methods() {
	// Purposely empty
}

AudioStreamGDMPTPlayback::AudioStreamGDMPTPlayback() {}

////////////////
