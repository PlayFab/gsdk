extends GutTest
## Unit tests for the GSDK logger.

const GsdkLogger := preload("res://addons/playfab_gsdk/gsdk_logger.gd")

var _logger = null
var _test_dir: String = ""


func before_each() -> void:
	_logger = GsdkLogger.new()
	_test_dir = "user://test_logs_%d" % randi()


func after_each() -> void:
	_logger = null
	if _test_dir != "" and DirAccess.dir_exists_absolute(_test_dir):
		var dir := DirAccess.open(_test_dir)
		if dir != null:
			dir.list_dir_begin()
			var file_name := dir.get_next()
			while file_name != "":
				dir.remove(file_name)
				file_name = dir.get_next()
			dir.list_dir_end()
		DirAccess.remove_absolute(_test_dir)


func test_initialize_creates_directory() -> void:
	assert_false(DirAccess.dir_exists_absolute(_test_dir))
	_logger.initialize(_test_dir)
	assert_true(DirAccess.dir_exists_absolute(_test_dir))


func test_initialize_creates_log_file() -> void:
	_logger.initialize(_test_dir)
	var dir := DirAccess.open(_test_dir)
	assert_not_null(dir, "Should be able to open test log directory")
	dir.list_dir_begin()
	var file_name := dir.get_next()
	var found_log := false
	while file_name != "":
		if file_name.begins_with("GSDK_output_") and file_name.ends_with(".txt"):
			found_log = true
			break
		file_name = dir.get_next()
	dir.list_dir_end()
	assert_true(found_log, "Log file should be created")


func test_log_info_writes_to_file() -> void:
	_logger.initialize(_test_dir)
	_logger.log_info("test info message")
	var content := _read_log_content()
	assert_string_contains(content, "[INFO] test info message")


func test_log_warn_writes_to_file() -> void:
	_logger.initialize(_test_dir)
	_logger.log_warn("test warning")
	var content := _read_log_content()
	assert_string_contains(content, "[WARN] test warning")


func test_log_error_writes_to_file() -> void:
	_logger.initialize(_test_dir)
	_logger.log_error("test error")
	var content := _read_log_content()
	assert_string_contains(content, "[ERROR] test error")


func test_log_debug_writes_to_file() -> void:
	_logger.initialize(_test_dir)
	_logger.log_debug("test debug")
	var content := _read_log_content()
	assert_string_contains(content, "[DEBUG] test debug")


func test_log_without_initialize_does_not_crash() -> void:
	# Logger should handle gracefully when not initialized (no file)
	_logger.log_info("no crash expected")
	_logger.log_warn("no crash expected")
	_logger.log_error("no crash expected")
	_logger.log_debug("no crash expected")
	pass_test("Logger did not crash without initialization")


func _read_log_content() -> String:
	var dir := DirAccess.open(_test_dir)
	if dir == null:
		return ""
	dir.list_dir_begin()
	var file_name := dir.get_next()
	while file_name != "":
		if file_name.begins_with("GSDK_output_") and file_name.ends_with(".txt"):
			var path := _test_dir + "/" + file_name
			var file := FileAccess.open(path, FileAccess.READ)
			if file != null:
				var content := file.get_as_text()
				file.close()
				dir.list_dir_end()
				return content
		file_name = dir.get_next()
	dir.list_dir_end()
	return ""
