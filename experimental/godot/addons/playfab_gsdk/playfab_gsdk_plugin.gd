@tool
extends EditorPlugin
## PlayFab GSDK editor plugin.
##
## Automatically registers the PlayFabGSDK autoload singleton when the plugin
## is enabled, and removes it when disabled.


func _enter_tree() -> void:
	add_autoload_singleton("PlayFabGSDK", "res://addons/playfab_gsdk/gsdk.gd")


func _exit_tree() -> void:
	remove_autoload_singleton("PlayFabGSDK")
