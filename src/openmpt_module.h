#ifndef OPENMPT_MODULE_H
#define OPENMPT_MODULE_H

#include <libopenmpt/libopenmpt_ext.h>

#include <memory>
#include <mutex>
#include <optional>

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

	void error_clear();

	int set_repeat_count(int32_t repeat_count);
	std::optional<int32_t> get_repeat_count() const;

	int set_tempo_factor(double factor);
	std::optional<double> get_tempo_factor() const;

	std::optional<double> get_duration_seconds() const;

	std::optional<double> get_current_estimated_bpm() const;

	std::optional<double> set_position_seconds(double seconds);
	std::optional<double> get_position_seconds() const;

	std::optional<size_t> read_interleaved_float_stereo(int32_t sample_rate, size_t count, float *interleaved_stereo);
};

#endif