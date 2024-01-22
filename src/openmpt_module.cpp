#include "openmpt_module.h"

void OpenMPTModule::set_pointers(ModuleExtUniquePtr p_module, InteractiveUniquePtr p_interactive) {
	const std::lock_guard<std::mutex> lock(mutex);

	module.swap(p_module);
	interactive.swap(p_interactive);
}

bool OpenMPTModule::is_null() const {
	const std::lock_guard<std::mutex> lock(mutex);

	return module == nullptr;
}

void OpenMPTModule::error_clear() {
	const std::lock_guard<std::mutex> lock(mutex);

	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	openmpt_module_error_clear(module_ptr);
}

int OpenMPTModule::set_repeat_count(int32_t repeat_count) {
	const std::lock_guard<std::mutex> lock(mutex);

	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	return openmpt_module_set_repeat_count(module_ptr, repeat_count);
}

std::optional<int32_t> OpenMPTModule::get_repeat_count() const {
	const std::lock_guard<std::mutex> lock(mutex);
	
	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	int repeat_count = openmpt_module_get_repeat_count(module_ptr);

	int error = openmpt_module_error_get_last(module_ptr);
	if (error == OPENMPT_ERROR_OK) {
		return repeat_count;
	} else {
		return std::nullopt;
	}
}

int OpenMPTModule::set_tempo_factor(double factor) {
	const std::lock_guard<std::mutex> lock(mutex);

	return interactive->set_tempo_factor(module.get(), factor);
}

std::optional<double> OpenMPTModule::get_tempo_factor() const {
	const std::lock_guard<std::mutex> lock(mutex);
	
	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	double tempo_factor = interactive->get_tempo_factor(module.get());

	int error = openmpt_module_error_get_last(module_ptr);
	if (error == OPENMPT_ERROR_OK) {
		return tempo_factor;
	} else {
		return std::nullopt;
	}
}

std::optional<double> OpenMPTModule::get_duration_seconds() const {
	const std::lock_guard<std::mutex> lock(mutex);
	
	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	double duration = openmpt_module_get_duration_seconds(module_ptr);

	int error = openmpt_module_error_get_last(module_ptr);
	if (error == OPENMPT_ERROR_OK) {
		return duration;
	} else {
		return std::nullopt;
	}
}

std::optional<double> OpenMPTModule::get_current_estimated_bpm() const {
	const std::lock_guard<std::mutex> lock(mutex);
	
	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	double bpm = openmpt_module_get_current_estimated_bpm(module_ptr);

	int error = openmpt_module_error_get_last(module_ptr);
	if (error == OPENMPT_ERROR_OK) {
		return bpm;
	} else {
		return std::nullopt;
	}
}

std::optional<double> OpenMPTModule::set_position_seconds(double seconds) {
	const std::lock_guard<std::mutex> lock(mutex);
	
	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	double position = openmpt_module_set_position_seconds(module_ptr, seconds);

	int error = openmpt_module_error_get_last(module_ptr);
	if (error == OPENMPT_ERROR_OK) {
		return position;
	} else {
		return std::nullopt;
	}
}

std::optional<double> OpenMPTModule::get_position_seconds() const {
	const std::lock_guard<std::mutex> lock(mutex);
	
	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	double position = openmpt_module_get_position_seconds(module_ptr);

	int error = openmpt_module_error_get_last(module_ptr);
	if (error == OPENMPT_ERROR_OK) {
		return position;
	} else {
		return std::nullopt;
	}
}

std::optional<size_t> OpenMPTModule::read_interleaved_float_stereo(int32_t sample_rate, size_t count, float *interleaved_stereo) {
	const std::lock_guard<std::mutex> lock(mutex);
	
	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	size_t frames_rendered = openmpt_module_read_interleaved_float_stereo(module_ptr, sample_rate, count, interleaved_stereo);

	int error = openmpt_module_error_get_last(module_ptr);
	if (error == OPENMPT_ERROR_OK) {
		return frames_rendered;
	} else {
		return std::nullopt;
	}
}
