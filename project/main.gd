extends Node2D

@onready var player: AudioStreamPlayer = get_node("AudioStreamPlayer")
var loops: int = 0
var num_channels: int = 0
var channel_0_enabled: bool = true

func _ready():
	var stream = AudioStreamGDMPT.load_from_file("res://bananasplit.mod")
	player.stream = stream
	player.stream.loop = true
	
	num_channels = stream.get_num_channels()
	print("%s number of channels: %s" % [player.stream.get_filename(), num_channels])
	
	stream.looped.connect(_on_song_loop)
	player.finished.connect(_on_song_end)
	
	player.play()
	
func _on_song_loop():
	loops += 1
	print("%s number of loops: %s" % [player.stream.get_filename(), loops])
	
# This won't get called if looping is enabled
func _on_song_end():
	print("Finished playing")

func _unhandled_key_input(event):
	if event is InputEventKey and event.pressed:
		if event.keycode == KEY_1:
			player.stream.set_tempo_factor(0.75)
		elif event.keycode == KEY_2:
			player.stream.set_tempo_factor(1.5)
		elif event.keycode == KEY_3:
			player.stream.set_tempo_factor(1.0)
		elif event.keycode == KEY_4:
			# Jump near the end of the song
			player.seek(94.0)
		elif event.keycode == KEY_5:
			channel_0_enabled = not channel_0_enabled
			if channel_0_enabled:
				player.stream.set_channel_volume(0, 1.0)
			else:
				player.stream.set_channel_volume(0, 0.0)
