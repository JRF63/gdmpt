#include "openmpt_player.h"
#include <Engine.hpp>

using namespace godot;

constexpr int32_t SAMPLE_RATE = 44100;
constexpr real_t DEFAULT_BUFFER_LENGTH = 0.5;

const char *FINISHED = "finished";

// Prevents libopenmpt from writing to std::clog. The errors are being reported
// in the catch blocks using Godot macros.
struct Logger : public std::ostream {
	Logger() :
			std::ostream(NULL) {}
};

void OpenMPTPlayer::_ready() {
	gen.instance();
	gen->set_mix_rate(SAMPLE_RATE);
	set_buffer_length(DEFAULT_BUFFER_LENGTH);
	set_physics_process(false);
}

void OpenMPTPlayer::_physics_process(float delta) {
	fill_buffer();
}

void OpenMPTPlayer::load(String filename) {
	// Don't need to call `stop` on initialization
	if (module) {
		stop();
	}

	Ref<File> file;
	file.instance();
	if (file->open(filename, File::ModeFlags::READ) != Error::OK) {
		String message = "Couldn't open ";
		message += filename;
		ERR_PRINT(message);
		return;
	}

	this->filename = filename;

	// Needs to be accessible by `module_ext` even if `ModPlayer` gets
	// moved around. Static is the simplest but this could be added as a member
	// to `ModPlayer` if we're assuming a `Node` has a fixed location in the heap.
	static Logger logger;

	auto buf = file->get_buffer(file->get_len());
	auto read = buf.read();

	try {
		module.emplace(read.ptr(), buf.size(), logger);
	} catch (const openmpt::exception &e) {
		ERR_PRINT(e.what());
		return;
	}

	interactive = static_cast<openmpt::ext::interactive *>(module->get_interface(openmpt::ext::interactive_id));
	if (interactive == nullptr) {
		ERR_PRINT("Unable to access OpenMPT `interactive` extension");
	}

	volume_settings.clear();
	for (int32_t i = 0; i < get_num_channels(); i++) {
		volume_settings.push_back(get_channel_volume(i));
	}
}

String OpenMPTPlayer::get_filename() {
	return filename;
}

real_t OpenMPTPlayer::get_buffer_length() {
	return gen->get_buffer_length();
}

void OpenMPTPlayer::set_buffer_length(real_t seconds) {
	ERR_FAIL_COND(gen.is_null());
	gen->set_buffer_length(seconds);

	// Need to be reset so that the buffer length takes effect
	AudioStreamPlayer::set_stream(gen);
	playback = AudioStreamPlayer::get_stream_playback();
}

void OpenMPTPlayer::play() {
	ERR_FAIL_NULL(module);
	AudioStreamPlayer::play();
	set_physics_process(true);
}

void OpenMPTPlayer::seek(const real_t to_position) {
	ERR_FAIL_NULL(module);
	try {
		module->set_position_seconds(to_position);
	} catch (const openmpt::exception &e) {
		ERR_PRINT(e.what());
	}
}

void OpenMPTPlayer::stop() {
	ERR_FAIL_NULL(module);
	AudioStreamPlayer::stop();

	seek(0.0);

	// Clears the buffer.
	//
	// The `stop` method is inaccessible:
	// https://github.com/godotengine/godot/blob/3.5.3-stable/servers/audio/effects/audio_stream_generator.cpp#L171
	//
	// So doing `playback->clear_buffer();` would print an error saying it's
	// active.
	set_buffer_length(get_buffer_length());

	set_physics_process(false);
}

void OpenMPTPlayer::set_tempo(int tempo) {
	ERR_FAIL_NULL(interactive);
	try {
		interactive->set_current_tempo(tempo);
	} catch (const openmpt::exception &e) {
		ERR_PRINT(e.what());
	}
}

int OpenMPTPlayer::get_tempo() const {
	ERR_FAIL_NULL_V(module, 0);

	try {
		return module->get_current_tempo();
	} catch (const openmpt::exception &e) {
		ERR_PRINT(e.what());
		return 0;
	}
}

void OpenMPTPlayer::set_speed(int speed) {
	ERR_FAIL_NULL(interactive);
	try {
		interactive->set_current_speed(speed);
	} catch (const openmpt::exception &e) {
		ERR_PRINT(e.what());
	}
}

int OpenMPTPlayer::get_speed() const {
	ERR_FAIL_NULL_V(module, 0);

	try {
		return module->get_current_speed();
	} catch (const openmpt::exception &e) {
		ERR_PRINT(e.what());
		return 0;
	}
}

void OpenMPTPlayer::set_tempo_factor(double tempo_factor) {
	ERR_FAIL_NULL(interactive);
	try {
		interactive->set_tempo_factor(tempo_factor);
	} catch (const openmpt::exception &e) {
		ERR_PRINT(e.what());
	}
}

double OpenMPTPlayer::get_tempo_factor() const {
	ERR_FAIL_NULL_V(interactive, 1.0);

	try {
		return interactive->get_tempo_factor();
	} catch (const openmpt::exception &e) {
		ERR_PRINT(e.what());
		return 1.0;
	}
}

void OpenMPTPlayer::set_pitch_factor(double pitch_factor) {
	ERR_FAIL_NULL(interactive);
	try {
		interactive->set_pitch_factor(pitch_factor);
	} catch (const openmpt::exception &e) {
		ERR_PRINT(e.what());
	}
}

double OpenMPTPlayer::get_pitch_factor() const {
	ERR_FAIL_NULL_V(interactive, 1.0);

	try {
		return interactive->get_pitch_factor();
	} catch (const openmpt::exception &e) {
		ERR_PRINT(e.what());
		return 1.0;
	}
}

void OpenMPTPlayer::set_loop(bool enable) {
	loop = enable;
}

bool OpenMPTPlayer::get_loop() const {
	return loop;
}

int32_t OpenMPTPlayer::get_num_channels() const {
	ERR_FAIL_NULL_V(module, 0);

	try {
		return module->get_num_channels();
	} catch (const openmpt::exception &e) {
		ERR_PRINT(e.what());
		return 0;
	}
}

void OpenMPTPlayer::set_channel_volume(int32_t channel, double volume) {
	ERR_FAIL_NULL(interactive);

	try {
		interactive->set_channel_volume(channel, volume);
		volume_settings[channel] = volume;
	} catch (const openmpt::exception &e) {
		ERR_PRINT(e.what());
	}
}

double OpenMPTPlayer::get_channel_volume(int32_t channel) const {
	ERR_FAIL_NULL_V(interactive, 0.0);

	try {
		return interactive->get_channel_volume(channel);
	} catch (const openmpt::exception &e) {
		ERR_PRINT(e.what());
		return 0.0;
	}
}

void OpenMPTPlayer::set_interpolation_filter(int32_t value) {
	ERR_FAIL_NULL(module);

	try {
		int param = openmpt::module::render_param::RENDER_INTERPOLATIONFILTER_LENGTH;
		module->set_render_param(param, value);
	} catch (const openmpt::exception &e) {
		ERR_PRINT(e.what());
	}
}

int32_t OpenMPTPlayer::get_interpolation_filter() const {
	ERR_FAIL_NULL_V(module, 0);

	try {
		int param = openmpt::module::render_param::RENDER_INTERPOLATIONFILTER_LENGTH;
		return module->get_render_param(param);
	} catch (const openmpt::exception &e) {
		ERR_PRINT(e.what());
		return 0;
	}
}

void OpenMPTPlayer::fill_buffer() {
	// Compile time check if `write_buffer` and the input to
	// `read_interleaved_stereo` have the same alignment.
	//
	// Usually not important for x86/x64 but writing to non-aligned memory
	// would segfault in ARM.
	static_assert(std::alignment_of<Vector2>::value ==
			std::alignment_of<float>::value);

	ERR_FAIL_NULL(module);
	ERR_FAIL_COND(playback.is_null());

	if (!AudioStreamPlayer::is_playing()) {
		return;
	}

	auto num_frames = playback->get_frames_available();

	if (num_frames > 0) {
		// Also sets the number of frames that will be passed to `push_buffer` below
		write_buffer.resize(num_frames);

		try {
			for (;;) {
				int frames_rendered;
				{
					auto write = write_buffer.write();
					// `read_interleaved_stereo` doesn't seem to throw any exceptions
					frames_rendered = module->read_interleaved_stereo(SAMPLE_RATE, num_frames, reinterpret_cast<float *>(write.ptr()));
				}

				bool end_of_song = frames_rendered == 0;
				if (end_of_song) {
					emit_signal(FINISHED);

					if (loop) {
						module->set_position_seconds(0.0);

						// `set_position_seconds` resets the volume
						for (int i = 0; i < volume_settings.size(); i++) {
							set_channel_volume(i, volume_settings[i]);
						}
						continue;
					}
				}

				// Need to trim the buffer if `frames_rendered` < `num_frames`
				write_buffer.resize(frames_rendered);

				playback->push_buffer(write_buffer);

				break;
			}
		} catch (const openmpt::exception &e) {
			ERR_PRINT(e.what());
		}
	}
}

OpenMPTPlayer::OpenMPTPlayer() {}

// Required even if empty
void OpenMPTPlayer::_init() {}

void OpenMPTPlayer::_register_methods() {
	register_method("_ready", &OpenMPTPlayer::_ready);
	register_method("_physics_process", &OpenMPTPlayer::_physics_process);
	register_method("load", &OpenMPTPlayer::load);
	register_method("get_filename", &OpenMPTPlayer::get_filename);
	register_method("play", &OpenMPTPlayer::play);
	register_method("seek", &OpenMPTPlayer::seek);
	register_method("stop", &OpenMPTPlayer::stop);
	register_method("get_num_channels", &OpenMPTPlayer::get_num_channels);
	register_method("set_channel_volume", &OpenMPTPlayer::set_channel_volume);
	register_method("get_channel_volume", &OpenMPTPlayer::get_channel_volume);

	register_property<OpenMPTPlayer, int>("tempo", &OpenMPTPlayer::set_tempo, &OpenMPTPlayer::get_tempo, 0);
	register_property<OpenMPTPlayer, int>("speed", &OpenMPTPlayer::set_speed, &OpenMPTPlayer::get_speed, 0);
	register_property<OpenMPTPlayer, double>("tempo_factor", &OpenMPTPlayer::set_tempo_factor, &OpenMPTPlayer::get_tempo_factor, 1.0);
	register_property<OpenMPTPlayer, double>("pitch_factor", &OpenMPTPlayer::set_pitch_factor, &OpenMPTPlayer::get_pitch_factor, 1.0);
	register_property<OpenMPTPlayer, real_t>("buffer_length", &OpenMPTPlayer::set_buffer_length, &OpenMPTPlayer::get_buffer_length, DEFAULT_BUFFER_LENGTH);
	register_property<OpenMPTPlayer, bool>("loop", &OpenMPTPlayer::set_loop, &OpenMPTPlayer::get_loop, false);
	register_property<OpenMPTPlayer, int32_t>("interpolation_filter", &OpenMPTPlayer::set_interpolation_filter, &OpenMPTPlayer::get_interpolation_filter, 0);

	register_signal<OpenMPTPlayer>(FINISHED);
}
