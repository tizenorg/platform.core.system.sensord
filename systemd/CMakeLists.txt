INSTALL(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/sensord.service
	${CMAKE_CURRENT_SOURCE_DIR}/sensord_command.socket
	${CMAKE_CURRENT_SOURCE_DIR}/sensord_event.socket
	DESTINATION lib/systemd/system)

