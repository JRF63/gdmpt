extends Node2D

func _ready():
	$AudioStreamPlayer.stream = AudioStreamGDMPT.load_from_file("res://bananasplit.mod")
	$AudioStreamPlayer.stream.loop = true
	$AudioStreamPlayer.play()

func _unhandled_key_input(event):
	if event is InputEventKey and event.pressed:
		if event.keycode == KEY_1:
			print("KEY_1")
			$AudioStreamPlayer.stream.set_tempo_factor(0.75)
		elif event.keycode == KEY_2:
			print("KEY_2")
			$AudioStreamPlayer.stream.set_tempo_factor(1.5)
		elif event.keycode == KEY_3:
			print("KEY_3")
			$AudioStreamPlayer.stream.set_tempo_factor(1.0)
