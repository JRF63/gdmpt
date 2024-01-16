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

int OpenMPTModule::set_repeat_count(int32_t repeat_count) {
	const std::lock_guard<std::mutex> lock(mutex);

	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	return openmpt_module_set_repeat_count(module_ptr, repeat_count);
}

int32_t OpenMPTModule::get_repeat_count() const {
	const std::lock_guard<std::mutex> lock(mutex);

	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	return openmpt_module_get_repeat_count(module_ptr);
}

int OpenMPTModule::set_tempo_factor(double factor) {
	const std::lock_guard<std::mutex> lock(mutex);

	return interactive->set_tempo_factor(module.get(), factor);
}

double OpenMPTModule::get_tempo_factor() const {
	const std::lock_guard<std::mutex> lock(mutex);

	return interactive->get_tempo_factor(module.get());
}

int OpenMPTModule::set_pitch_factor(double factor) {
	const std::lock_guard<std::mutex> lock(mutex);

	return interactive->set_pitch_factor(module.get(), factor);
}

double OpenMPTModule::get_pitch_factor() const {
	const std::lock_guard<std::mutex> lock(mutex);

	return interactive->get_pitch_factor(module.get());
}

int OpenMPTModule::set_interpolation_filter(int32_t filter) {
	const std::lock_guard<std::mutex> lock(mutex);

	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	return openmpt_module_set_render_param(
			module_ptr,
			OPENMPT_MODULE_RENDER_INTERPOLATIONFILTER_LENGTH,
			filter);
}

int OpenMPTModule::get_interpolation_filter(int32_t *value) const {
	const std::lock_guard<std::mutex> lock(mutex);

	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	return openmpt_module_get_render_param(
			module_ptr,
			OPENMPT_MODULE_RENDER_INTERPOLATIONFILTER_LENGTH,
			value);
}

int32_t OpenMPTModule::get_num_channels() const {
	const std::lock_guard<std::mutex> lock(mutex);

	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	return openmpt_module_get_num_channels(module_ptr);
}

int OpenMPTModule::set_channel_volume(int32_t channel, double volume) {
	const std::lock_guard<std::mutex> lock(mutex);

	return interactive->set_channel_volume(module.get(), channel, volume);
}

double OpenMPTModule::get_channel_volume(int32_t channel) const {
	const std::lock_guard<std::mutex> lock(mutex);

	return interactive->get_channel_volume(module.get(), channel);
}

double OpenMPTModule::get_duration_seconds() const {
	const std::lock_guard<std::mutex> lock(mutex);

	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	return openmpt_module_get_duration_seconds(module_ptr);
}

double OpenMPTModule::get_current_estimated_bpm() const {
	const std::lock_guard<std::mutex> lock(mutex);

	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	return openmpt_module_get_current_estimated_bpm(module_ptr);
}

double OpenMPTModule::set_position_seconds(double seconds) {
	const std::lock_guard<std::mutex> lock(mutex);

	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	return openmpt_module_set_position_seconds(module_ptr, seconds);
}

double OpenMPTModule::get_position_seconds() const {
	const std::lock_guard<std::mutex> lock(mutex);

	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	return openmpt_module_get_position_seconds(module_ptr);
}

size_t OpenMPTModule::read_interleaved_float_stereo(int32_t sample_rate, size_t count, float *interleaved_stereo) {
	const std::lock_guard<std::mutex> lock(mutex);

	auto module_ptr = reinterpret_cast<openmpt_module *>(module.get());
	return openmpt_module_read_interleaved_float_stereo(module_ptr, sample_rate, count, interleaved_stereo);
}
