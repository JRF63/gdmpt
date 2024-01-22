extends Node2D

@onready var player = get_node("AudioStreamPlayer")

func _ready():
	player.stream = AudioStreamGDMPT.load_from_file("res://bananasplit.mod")
	player.stream.loop = true
	player.play()

func _unhandled_key_input(event):
	if event is InputEventKey and event.pressed:
		if event.keycode == KEY_1:
			player.stream.set_tempo_factor(0.75)
		elif event.keycode == KEY_2:
			player.stream.set_tempo_factor(1.5)
		elif event.keycode == KEY_3:
			player.stream.set_tempo_factor(1.0)
		elif event.keycode == KEY_4:
			player.seek(20.0)
