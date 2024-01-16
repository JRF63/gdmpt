extends Node2D

onready var player = get_node("AudioStreamPlayer")
onready var tempo_slider = get_node("SliderContainer/TempoSlider")
onready var pitch_slider = get_node("SliderContainer/PitchSlider")
onready var channels_container = get_node("ChannelsContainer")
onready var play_button = get_node("PlayButton")
onready var pause_button = get_node("PauseButton")
onready var stop_button = get_node("StopButton")
onready var interpolation_button = get_node("InterpolationButton")

var num_loops = 0

# - 0: internal default
# - 1: no interpolation (zero order hold)
# - 2: linear interpolation
# - 4: cubic interpolation
# - 8: windowed sinc with 8 taps
var interpolation_values = [0, 1, 2, 4, 8]

func _ready():
	# Must access properties after `load`
	player.load("res://bananasplit.mod")

	player.loop = true

	# Keep above `2048 / AudioServer.get_mix_rate()` to avoid audio glitches
	player.buffer_length = 2048 / AudioServer.get_mix_rate()
	
	player.connect("finished", self, "_on_song_finished")
	
	tempo_slider.connect("value_changed", self, "_on_tempo_changed")
	pitch_slider.connect("value_changed", self, "_on_pitch_changed")
	
	recreate_channel_toggles()
		
	play_button.connect("pressed", self, "_on_play")
	pause_button.connect("pressed", self, "_on_pause")
	stop_button.connect("pressed", self, "_on_stop")
	
	for filter in ["Default", "None", "Linear", "Cubic", "Sinc"]:
		interpolation_button.add_item(filter)
	interpolation_button.connect("item_selected", self, "_on_filter_selected")

func recreate_channel_toggles():
	var children = []
	for child in channels_container.get_children():
		children.append(child)
	for child in children:
		channels_container.remove_child(child)
		
	for c in range(player.get_num_channels()):
		var margin_value = 5
		var button_size = 60
		
		var margin = MarginContainer.new()
		
		margin.add_constant_override("margin_top", margin_value)
		margin.add_constant_override("margin_left", margin_value)
		margin.add_constant_override("margin_bottom", margin_value)
		margin.add_constant_override("margin_right", margin_value)

		var button = Button.new()
		button.toggle_mode = true
		button.pressed = true
		button.text = "%d" % c
		button.set_custom_minimum_size(Vector2(button_size, button_size))
		button.connect("toggled", self, "_on_channel_toggled", [c])
		
		margin.add_child(button)
		channels_container.add_child(margin)
	
func _on_song_finished():
	num_loops += 1
	print("%s number of loops: %d" % [player.get_filename(), num_loops])
	
func _on_tempo_changed(value):
	player.tempo_factor = value
	
func _on_pitch_changed(value):
	player.pitch_factor = value
	
func _on_channel_toggled(button_pressed, channel):
	var volume = 0.0
	if button_pressed:
		volume = 1.0
	player.set_channel_volume(channel, volume)
	
func _on_play():
	player.play()
	
func _on_pause():
	player.stream_paused = not player.stream_paused
	
func _on_stop():
	player.stop()
	
func _on_filter_selected(index):
	player.interpolation_filter = interpolation_values[index]
	
func _unhandled_key_input(event):
	if event is InputEventKey and event.pressed:
		if event.scancode == KEY_1:
			# Jump to end of bananasplit.mod
			player.seek(95.0)
