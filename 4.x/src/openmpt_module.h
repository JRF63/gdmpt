#ifndef OPENMPT_MODULE_H
#define OPENMPT_MODULE_H

#include <libopenmpt/libopenmpt_ext.h>

#include <memory>
#include <mutex>

struct OpenMPTModuleExtDeleter {
	void operator()(openmpt_module_ext *p) { openmpt_module_ext_destroy(p); }
};

using ModuleExtUniquePtr =
		std::unique_ptr<openmpt_module_ext, OpenMPTModuleExtDeleter>;
using InteractiveUniquePtr =
		std::unique_ptr<openmpt_module_ext_interface_interactive>;

class OpenMPTModule {
	ModuleExtUniquePtr module;
	InteractiveUniquePtr interactive;
	mutable std::mutex mutex; // Needs to be accessed from `const` methods

public:
	void set_pointers(ModuleExtUniquePtr p_module, InteractiveUniquePtr p_interactive);

	bool is_null() const;

	int set_repeat_count(int32_t repeat_count);
	int32_t get_repeat_count() const;

	int set_tempo_factor(double factor);
	double get_tempo_factor() const;

	int set_pitch_factor(double factor);
	double get_pitch_factor() const;

	int set_interpolation_filter(int32_t filter);
	int get_interpolation_filter(int32_t *value) const;

	int32_t get_num_channels() const;

	int set_channel_volume(int32_t channel, double volume);
	double get_channel_volume(int32_t channel) const;

	double get_duration_seconds() const;

	double get_current_estimated_bpm() const;

	double set_position_seconds(double seconds);
	double get_position_seconds() const;

	size_t read_interleaved_float_stereo(int32_t sample_rate, size_t count, float *interleaved_stereo);
};

#endif