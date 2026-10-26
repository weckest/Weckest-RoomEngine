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
        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated
)

set(PROTO_GENERATOR
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_proto.py
)

if(WIN32)
	set(PROTOCOLS_DIR
    		${CMAKE_CURRENT_SOURCE_DIR}/../Protocols/Proto
	)
	set(RENDER_PROTO
		${PROTOCOLS_DIR}/render.proto
	)
else()
	set(PROTOCOLS_DIR
    		${CMAKE_CURRENT_SOURCE_DIR}/../Weckest-Protocols/Proto
	)
	
	set(RENDER_PROTO
		${PROTOCOLS_DIR}/render.proto
	)
endif()

set(RENDER_PROTO_SRC
        ${CMAKE_CURRENT_BINARY_DIR}/render.pb.cc
        ${CMAKE_CURRENT_BINARY_DIR}/render.grpc.pb.cc
)

set(RENDER_PROTO_HDR
        ${CMAKE_CURRENT_BINARY_DIR}/render.pb.h
        ${CMAKE_CURRENT_BINARY_DIR}/render.grpc.pb.h
)

add_custom_command(
        OUTPUT
        ${RENDER_PROTO_SRC}
        ${RENDER_PROTO_HDR}

        COMMAND protobuf::protoc
        --proto_path=${PROTOCOLS_DIR}
        --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
        --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
        --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
        ${RENDER_PROTO}

        DEPENDS
        ${RENDER_PROTO}

        COMMENT "Generating C++ protobuf and gRPC sources"
)

add_library(render_proto
        ${RENDER_PROTO_SRC}
        ${RENDER_PROTO_HDR}
)

target_include_directories(render_proto
        PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(render_proto
        PUBLIC
        protobuf::libprotobuf
        gRPC::grpc++
)

add_custom_target(generate_proto ALL
        COMMAND ${Python3_EXECUTABLE} ${PROTO_GENERATOR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating nanopb protobuf files"
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
)

add_dependencies(RoomEngine generate_proto)

target_compile_definitions(RoomEngine PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
)

target_include_directories(RoomEngine
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/nanopb-0.4.9.2-windows-x86
)

target_link_libraries(RoomEngine PRIVATE render_proto)

if (WIN32)
    target_compile_definitions(RoomEngine PRIVATE NOMINMAX)
    target_link_libraries(RoomEngine PRIVATE Ws2_32)
endif()
