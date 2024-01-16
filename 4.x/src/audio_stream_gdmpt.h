#ifndef AUDIO_STREAM_GDMPT_H
#define AUDIO_STREAM_GDMPT_H

#include "openmpt_module.h"

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_playback_resampled.hpp>

#include <optional>

namespace godot {

// Forward declaration to be able to add as a friend class
class AudioStreamGDMPTPlayback;

class AudioStreamGDMPT : public AudioStream {
	GDCLASS(AudioStreamGDMPT, AudioStream)

	friend class AudioStreamGDMPTPlayback;

	OpenMPTModule module;
	String filename;
	bool loop = false;
	std::vector<double> volume_settings;
	mutable int openmpt_error = OPENMPT_ERROR_OK; // mutable to access from const

	// Emits the signal. This is called from `AudioStreamGDMPTPlayback` but the
	// itself has to be emitted by `AudioStreamGDMPT`
	void emit_looping_signal();

	// Retrieves and clears the last OpenMPT error. Returns a `nullptr` if
	// there is no error.
	std::optional<String> pop_last_openmpt_error() const;

	// OpenMPT error func used to store the error for later use
	static int error_func(int error, void *ptr);

protected:
	static void _bind_methods();

public:
	enum InterpolationFilter {
		DEFAULT_INTERPOLATION = 0,
		NO_INTERPOLATION = 1,
		LINEAR_INTERPOLATION = 2,
		CUBIC_INTERPOLATION = 4,
		SINC_INTERPOLATION = 8
	};

	static Ref<AudioStreamGDMPT> load_from_buffer(
			const PackedByteArray &buffer);

	static Ref<AudioStreamGDMPT> load_from_file(const String &path);

	String get_filename() const;

	void set_loop(bool enable);
	bool get_loop() const;

	void set_tempo_factor(double factor);
	double get_tempo_factor() const;

	void set_pitch_factor(double factor);
	double get_pitch_factor() const;

	void set_interpolation_filter(InterpolationFilter filter);
	InterpolationFilter get_interpolation_filter() const;

	int32_t get_num_channels() const;

	void set_channel_volume(int32_t channel, double volume);
	double get_channel_volume(int32_t channel) const;

	// Overrides

	virtual Ref<AudioStreamPlayback> _instantiate_playback() const override;

	virtual String _get_stream_name() const override;

	virtual double _get_length() const override;

	virtual bool _is_monophonic() const override;

	virtual double _get_bpm() const override;

	virtual int32_t _get_beat_count() const override;

	AudioStreamGDMPT();
};

class AudioStreamGDMPTPlayback : public AudioStreamPlaybackResampled {
	GDCLASS(AudioStreamGDMPTPlayback, AudioStreamPlaybackResampled);

	friend class AudioStreamGDMPT;

	Ref<AudioStreamGDMPT> stream;
	bool active = false;
	int32_t loops = 0;

protected:
	static void _bind_methods();

public:
	// Overrides
	virtual void _start(double from_pos) override;

	virtual void _stop() override;

	virtual bool _is_playing() const override;

	virtual int32_t _get_loop_count() const override;

	virtual double _get_playback_position() const override;

	virtual void _seek(double position) override;

	virtual int32_t _mix_resampled(AudioFrame *dst_buffer, int32_t frame_count) override;

	virtual double _get_stream_sampling_rate() const override;

	AudioStreamGDMPTPlayback();
};

} // namespace godot

VARIANT_ENUM_CAST(AudioStreamGDMPT::InterpolationFilter);

#endif
