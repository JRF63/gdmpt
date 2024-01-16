extends Node2D

const MUSIC_FILE: String = "res://bananasplit.mod"
const APPROX_END: float = 94.0
const INTERP_VALUES: Array[int] = [
	AudioStreamGDMPT.DEFAULT_INTERPOLATION,
	AudioStreamGDMPT.NO_INTERPOLATION,
	AudioStreamGDMPT.LINEAR_INTERPOLATION,
	AudioStreamGDMPT.CUBIC_INTERPOLATION,
	AudioStreamGDMPT.SINC_INTERPOLATION
]
const INTERP_LABELS: Array[String] = [
	"Default",
	"None",
	"Linear",
	"Cubic",
	"Sinc"
]

@onready var player: AudioStreamPlayer = get_node("AudioStreamPlayer")
var loops: int = 0

func _ready():
	var stream = AudioStreamGDMPT.load_from_file(MUSIC_FILE)
	player.stream = stream
	player.stream.loop = true
	
	for i in range(stream.get_num_channels()):
		
		const BUTTON_SIZE: int = 50
		const MARGIN_VALUE: int = 5
		
		var button = Button.new()
		button.text = "%d" % i
		button.custom_minimum_size = Vector2(BUTTON_SIZE, BUTTON_SIZE)
		button.toggle_mode = true
		button.button_pressed = true
		button.toggled.connect(_on_channel_toggled.bind(i))
		
		var margin = MarginContainer.new()
		margin.add_theme_constant_override("margin_top", MARGIN_VALUE)
		margin.add_theme_constant_override("margin_left", MARGIN_VALUE)
		margin.add_theme_constant_override("margin_bottom", MARGIN_VALUE)
		margin.add_theme_constant_override("margin_right", MARGIN_VALUE)

		margin.add_child(button)
		
		$EnabledChannelsContainer.add_child(margin)
	
	stream.looped.connect(_on_song_loop)
	player.finished.connect(_on_song_end)
	
	$SlidersContainer/TempoSlider.value_changed.connect(_on_tempo_changed)
	$SlidersContainer/PitchSlider.value_changed.connect(_on_pitch_changed)
	
	for filter in INTERP_LABELS:
		$InterpolationButton.add_item(filter)
	$InterpolationButton.item_selected.connect(_on_filter_selected)
	
	$PlayButton.pressed.connect(_on_play)
	$PauseButton.pressed.connect(_on_pause)
	$StopButton.pressed.connect(_on_stop)
	
func _on_song_loop():
	loops += 1
	print("%s number of loops: %s" % [player.stream.get_filename(), loops])
	
# This won't get called if looping is enabled
func _on_song_end():
	print("Finished playing")
	
func _on_tempo_changed(value):
	player.stream.tempo_factor = value

func _on_pitch_changed(value):
	player.stream.pitch_factor = value
	
func _on_channel_toggled(toggled_on: bool, channel: int):
	var volume = 0.0
	if toggled_on:
		volume = 1.0
	player.stream.set_channel_volume(channel, volume)
	
func _on_filter_selected(index: int):
	player.stream.interpolation_filter = INTERP_VALUES[index]
	
func _on_play():
	player.play()
	
func _on_pause():
	player.stream_paused = not player.stream_paused
	
func _on_stop():
	player.stop()

func _unhandled_key_input(event):
	if event is InputEventKey and event.pressed:
		if event.keycode == KEY_1:
			# Jump near the end of the song
			player.seek(APPROX_END)
