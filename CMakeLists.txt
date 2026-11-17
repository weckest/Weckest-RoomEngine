
cmake_minimum_required(VERSION 4.1)

project(RoomEngine
		VERSION 0.1.0
		LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

if(WIN32)
	# Windows: dependencies installed through vcpkg
	find_package(Protobuf CONFIG REQUIRED)
	find_package(gRPC CONFIG REQUIRED)
else()
	# Linux: dependencies installed through APT
	find_package(Protobuf REQUIRED)
	find_package(gRPC CONFIG REQUIRED)
endif()

set(PROTO_GENERATED_DIR
		"${CMAKE_CURRENT_SOURCE_DIR}/src/generated"
)

set(PROTO_GENERATOR
		"${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_proto.py"
)

if(WIN32)
	set(PROTOCOLS_DIR
			"${CMAKE_CURRENT_SOURCE_DIR}/../Protocols/Proto"
	)
else()
	set(PROTOCOLS_DIR
			"${CMAKE_CURRENT_SOURCE_DIR}/../Weckest-Protocols/Proto"
	)
endif()

function(add_proto_library TARGET_NAME PROTO_NAME)

	set(PROTO_FILE "${PROTOCOLS_DIR}/${PROTO_NAME}.proto")

	set(PROTO_SRC
			"${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc"
	)

	set(PROTO_HDR
			"${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h"
	)

	add_custom_command(
			OUTPUT
			${PROTO_SRC}
			${PROTO_HDR}

			COMMAND protobuf::protoc
			"--proto_path=${PROTOCOLS_DIR}"
			"--cpp_out=${CMAKE_CURRENT_BINARY_DIR}"
			"${PROTO_FILE}"

			DEPENDS
			"${PROTO_FILE}"

			COMMENT "Generating C++ protobuf: ${PROTO_NAME}.proto"
			VERBATIM
	)

	add_library(${TARGET_NAME} STATIC
			${PROTO_SRC}
			${PROTO_HDR}
	)

	target_include_directories(${TARGET_NAME}
			PUBLIC
			"${CMAKE_CURRENT_BINARY_DIR}"
	)

	target_link_libraries(${TARGET_NAME}
			PUBLIC
			protobuf::libprotobuf
	)

endfunction()

# Generate C++ protobuf and gRPC code
function(add_grpc_proto_library TARGET_NAME PROTO_NAME)
	set(PROTO_FILE "${PROTOCOLS_DIR}/${PROTO_NAME}.proto")

	set(PROTO_SRC
			"${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc"
			"${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.cc"
	)

	set(PROTO_HDR
			"${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h"
			"${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.h"
	)

	set(PROTO_DEPENDENCIES
			"${PROTO_FILE}"
	)

	# Add imported proto files as build dependencies.
	if(PROTO_NAME STREQUAL "game")
		list(APPEND PROTO_DEPENDENCIES
				"${PROTOCOLS_DIR}/database_options.proto"
				"${PROTOCOLS_DIR}/hardware.proto"
		)
	elseif(PROTO_NAME STREQUAL "roomengine")
		list(APPEND PROTO_DEPENDENCIES
				"${PROTOCOLS_DIR}/game.proto"
		)
	endif()

	add_custom_command(
			OUTPUT ${PROTO_SRC} ${PROTO_HDR}

			COMMAND protobuf::protoc
			"--proto_path=${PROTOCOLS_DIR}"
			"--cpp_out=${CMAKE_CURRENT_BINARY_DIR}"
			"--grpc_out=${CMAKE_CURRENT_BINARY_DIR}"
			"--plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
			"${PROTO_FILE}"

			DEPENDS ${PROTO_DEPENDENCIES}

			COMMENT "Generating C++ protobuf/gRPC: ${PROTO_NAME}.proto"
			VERBATIM
	)

	add_library(${TARGET_NAME} STATIC ${PROTO_SRC} ${PROTO_HDR})

	target_include_directories(${TARGET_NAME}
			PUBLIC "${CMAKE_CURRENT_BINARY_DIR}"
	)

	target_link_libraries(${TARGET_NAME}
			PUBLIC protobuf::libprotobuf gRPC::grpc++
	)
endfunction()

# Generate protocol libraries
add_proto_library(database_options_proto database_options)
#add_proto_library(hardware_proto hardware)


add_grpc_proto_library(render_proto render)
add_grpc_proto_library(game_proto game)
add_grpc_proto_library(roomengine_proto roomengine)

# roomengine.proto imports game.proto
target_link_libraries(roomengine_proto
		PUBLIC
		game_proto
)

target_link_libraries(game_proto
		PUBLIC
		database_options_proto
#		hardware_proto
)

#add_library(render_proto
#		${RENDER_PROTO_SRC}
#		${RENDER_PROTO_HDR}
#)

#target_include_directories(render_proto
#		PUBLIC
#		"${CMAKE_CURRENT_BINARY_DIR}"
#)

#target_link_libraries(render_proto
#		PUBLIC
#		protobuf::libprotobuf
#		gRPC::grpc++
#)

add_custom_target(generate_proto ALL
		COMMAND "${Python3_EXECUTABLE}" "${PROTO_GENERATOR}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		COMMENT "Generating nanopb protobuf files"
		VERBATIM
)

add_executable(RoomEngine
		src/main.cpp
		src/network/TCPConnection.cpp
		src/network/TCPServer.cpp
		src/generated/hardware.pb.c
		src/services/ModuleService.cpp
		tools/nanopb-0.4.9.2-windows-x86/pb_common.c
		tools/nanopb-0.4.9.2-windows-x86/pb_encode.c
		tools/nanopb-0.4.9.2-windows-x86/pb_decode.c
		src/utils/Vector3D.cpp
		src/engine/RoomEngine.cpp
		src/ecs/World.cpp
		src/ecs/systems/LightSystem.cpp
		src/ecs/managers/RoomEventManager.cpp
		src/ecs/managers/ActionManager.cpp
		src/ecs/systems/RoomEventSystem.cpp
		src/services/visuals/RenderServiceClient.cpp
		src/ecs/managers/ModuleManager.cpp
		src/ecs/systems/MovementSystem.cpp
		src/ecs/managers/SystemManager.cpp
		src/ecs/systems/ColorSystem.cpp
		src/ecs/systems/RenderSystem.cpp
		src/ecs/systems/CombatSystem.cpp
		src/ecs/systems/HealthSystem.cpp
		src/ecs/systems/WaitSystem.cpp
		src/ecs/systems/RoomEventCooldownSystem.cpp
		src/ecs/systems/SceneSystem.cpp
		src/ecs/systems/SpawnSystem.cpp
		src/ecs/systems/ContextSystem.cpp
		src/ecs/systems/BlockSystem.cpp
		src/ecs/systems/EnemySystem.cpp
		src/services/RoomEngineService.cpp
		src/ecs/systems/TestSystem.cpp
)

add_dependencies(RoomEngine generate_proto)

target_include_directories(RoomEngine
		PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/src"
		"${PROTO_GENERATED_DIR}"
		"${CMAKE_CURRENT_SOURCE_DIR}/tools/nanopb-0.4.9.2-windows-x86"
)

target_link_libraries(RoomEngine
		PRIVATE
		render_proto
		roomengine_proto
)

if(WIN32)
	target_compile_definitions(RoomEngine
			PRIVATE
			WIN32_LEAN_AND_MEAN
			NOMINMAX
	)

	target_link_libraries(RoomEngine
			PRIVATE
			Ws2_32
	)
endif()
