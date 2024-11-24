
IF(NOT WITH_BMSITE_RRS)
	RETURN()
ENDIF(NOT WITH_BMSITE_RRS)

# 'client_sheets' and 'game_share'
INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/ryzom/client/src
	${CMAKE_SOURCE_DIR}/ryzom/common/src
	${LIBXML2_INCLUDE_DIR}
	)

# compile client sheets even when client is not compiled
IF(NOT WITH_RYZOM_CLIENT)
	ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/ryzom/client/src/client_sheets "${CMAKE_BINARY_DIR}/ryzom/client/src/client_sheets")
ENDIF(NOT WITH_RYZOM_CLIENT)

LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CMAKE_LIBRARY_DIR})

FILE(GLOB SRC src/*.cpp src/*.h)

ADD_EXECUTABLE(render_service ${SRC}
	${CMAKE_SOURCE_DIR}/ryzom/client/src/color_slot_manager.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/color_slot_manager.h
	${CMAKE_SOURCE_DIR}/ryzom/client/src/gabarit.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/gabarit.h
	${CMAKE_SOURCE_DIR}/ryzom/client/src/sheet_manager.cpp
	${CMAKE_SOURCE_DIR}/ryzom/client/src/sheet_manager.h
	)

TARGET_LINK_LIBRARIES(render_service
	nel3d
	nelnet
	ryzom_clientsheets
	ryzom_gameshare
	)

NL_DEFAULT_PROPS(render_service "Ryzom, Tools: Render Service")
NL_ADD_RUNTIME_FLAGS(render_service)

IF(WITH_PCH AND (NOT MINGW OR NOT WITH_SYMBOLS))
	target_precompile_headers(render_service
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.h ${CMAKE_CURRENT_SOURCE_DIR}/stdpch.cpp
	)
ENDIF()

INSTALL(TARGETS render_service RUNTIME DESTINATION bin COMPONENT tools)
