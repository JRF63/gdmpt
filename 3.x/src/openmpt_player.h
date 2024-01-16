#ifndef OPENMPT_PLAYER
#define OPENMPT_PLAYER

#include <memory>
#include <optional>
#include <type_traits>

#include <AudioStreamGenerator.hpp>
#include <AudioStreamGeneratorPlayback.hpp>
#include <AudioStreamPlayer.hpp>
#include <File.hpp>
#include <Godot.hpp>
#include <PoolArrays.hpp>

#include <libopenmpt/libopenmpt_ext.hpp>

namespace godot {

class OpenMPTPlayer : public AudioStreamPlayer {
	GODOT_CLASS(OpenMPTPlayer, AudioStreamPlayer);

private:
	String filename;
	bool loop = false;
	std::vector<double> volume_settings;

	std::optional<openmpt::module_ext> module;
	openmpt::ext::interactive *interactive = nullptr;

	Ref<AudioStreamGenerator> gen;
	Ref<AudioStreamGeneratorPlayback> playback;
	PoolVector2Array write_buffer;

	void fill_buffer();

public:
	OpenMPTPlayer();

	static void _register_methods();

	void _init();

	void _ready();

	void _physics_process(float delta);

	void load(String filename);
	String get_filename();

	real_t get_buffer_length();
	void set_buffer_length(real_t seconds);

	// `AudioStreamPlayer` overrides

	void play();

	void seek(const real_t to_position);

	void stop();

	// OpenMPT methods

	void set_tempo(int tempo);
	int get_tempo() const;

	void set_speed(int speed);
	int get_speed() const;

	void set_tempo_factor(double tempo_factor);
	double get_tempo_factor() const;

	void set_pitch_factor(double pitch_factor);
	double get_pitch_factor() const;

	void set_loop(bool enable);
	bool get_loop() const;

	int32_t get_num_channels() const;

	void set_channel_volume(int32_t channel, double volume);
	double get_channel_volume(int32_t channel) const;

	void set_interpolation_filter(int32_t value);
	int32_t get_interpolation_filter() const;
};

} //namespace godot

#endif