## Logger for the PlayFab GSDK.
##
## Writes log messages to both the Godot console (stdout) and a log file
## in the configured log directory.

var _log_file: FileAccess = null


## Initializes the logger by opening a log file in the specified directory.
func initialize(directory: String) -> void:
	if not DirAccess.dir_exists(directory):
		DirAccess.make_dir_recursive_absolute(directory)

	var timestamp := int(Time.get_unix_time_from_system())
	var pid := OS.get_process_id()
	var filename := "%s/GSDK_output_%d_%d.txt" % [directory, timestamp, pid]
	_log_file = FileAccess.open(filename, FileAccess.WRITE)
	if _log_file == null:
		push_error("GSDK: Failed to open log file: %s" % filename)


## Logs an informational message.
func log_info(message: String) -> void:
	var formatted := "[INFO] %s" % message
	print(formatted)
	_write_to_file(formatted)


## Logs a warning message.
func log_warn(message: String) -> void:
	var formatted := "[WARN] %s" % message
	push_warning(formatted)
	_write_to_file(formatted)


## Logs an error message.
func log_error(message: String) -> void:
	var formatted := "[ERROR] %s" % message
	push_error(formatted)
	_write_to_file(formatted)


## Logs a debug message.
func log_debug(message: String) -> void:
	var formatted := "[DEBUG] %s" % message
	print(formatted)
	_write_to_file(formatted)


func _write_to_file(message: String) -> void:
	if _log_file != null:
		_log_file.store_line(message)
		_log_file.flush()
