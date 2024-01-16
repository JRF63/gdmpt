#ifndef MODPLAYER_H
#define MODPLAYER_H

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_playback.hpp>
#include <libopenmpt/libopenmpt_ext.hpp>
#include <memory>


namespace godot {

class AudioStreamGDMPTPlayback;

class AudioStreamGDMPT : public AudioStream {
    GDCLASS(AudioStreamGDMPT, AudioStream)

    friend class AudioStreamGDMPTPlayback;

    std::unique_ptr<openmpt::module_ext> module;
    openmpt::ext::interactive* interactive = nullptr;

   protected:
    static void _bind_methods();

   public:
    static Ref<AudioStreamGDMPT> load_from_buffer(const PackedByteArray &buffer);

    static Ref<AudioStreamPlayback> load_from_file(const String &path);

    void set_loop(bool enable);

	bool get_loop() const;

    void set_tempo_factor( double factor );

    double get_tempo_factor( ) const;

    virtual Ref<AudioStreamPlayback> _instantiate_playback() const override;

	virtual String _get_stream_name() const override;

	virtual double _get_length() const override;

	virtual bool _is_monophonic() const override;

	virtual double _get_bpm() const override;

	virtual int32_t _get_beat_count() const override;

    AudioStreamGDMPT();
};

class AudioStreamGDMPTPlayback : public AudioStreamPlayback {
    GDCLASS(AudioStreamGDMPTPlayback, AudioStreamPlayback);

    friend class AudioStreamGDMPT;

    Ref<AudioStreamGDMPT> stream;
    bool active = false;

   protected:
    static void _bind_methods();

   public:
    virtual void _start(double from_pos) override;

    virtual void _stop() override;

    virtual bool _is_playing() const override;

	virtual int32_t _get_loop_count() const;

	virtual double _get_playback_position() const;

	virtual void _seek(double position) override;

    virtual int32_t _mix(AudioFrame *buffer, double rate_scale, int32_t frames) override;

    // TODO: What is `_tag_used_streams` for?
	// virtual void _tag_used_streams();

    AudioStreamGDMPTPlayback();
};

}  // namespace godot

#endif
