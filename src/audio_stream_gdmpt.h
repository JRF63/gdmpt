#ifndef AUDIO_STREAM_GDMPT_H
#define AUDIO_STREAM_GDMPT_H

#include "openmpt_module.h"

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_playback_resampled.hpp>

namespace godot {

class AudioStreamGDMPTPlayback;

class AudioStreamGDMPT : public AudioStream {
	GDCLASS(AudioStreamGDMPT, AudioStream)

	friend class AudioStreamGDMPTPlayback;

	OpenMPTModule module;

protected:
	static void _bind_methods();

public:
	static Ref<AudioStreamGDMPT> load_from_buffer(
			const PackedByteArray &buffer);

	static Ref<AudioStreamGDMPT> load_from_file(const String &path);

	void set_loop(bool enable);

	bool get_loop() const;

	void set_tempo_factor(double factor);

	double get_tempo_factor() const;

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

protected:
	static void _bind_methods();

public:
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

#endif
