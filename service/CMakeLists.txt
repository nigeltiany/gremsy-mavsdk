get_filename_component(service "../proto/gimbal.proto" ABSOLUTE)
get_filename_component(service_path "${service}" PATH)

set(service_sources "${CMAKE_CURRENT_BINARY_DIR}/gimbal.pb.cc")
set(service_headers "${CMAKE_CURRENT_BINARY_DIR}/gimbal.pb.h")
set(gimbal_grpc_sources "${CMAKE_CURRENT_BINARY_DIR}/gimbal.grpc.pb.cc")
set(gimbal_grpc_headers "${CMAKE_CURRENT_BINARY_DIR}/gimbal.grpc.pb.h")

set(CURL_DIR /home/nigel/MAVSDK/build/default/third_party/install/lib/cmake/CURL)
set(jsoncpp_DIR /home/nigel/MAVSDK/build/default/third_party/install/lib/cmake/jsoncpp)

add_custom_command(
        OUTPUT "${service_sources}" "${service_headers}" "${gimbal_grpc_sources}" "${gimbal_grpc_headers}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}" --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${service_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${service}"
        DEPENDS "${service}")

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_library(gimbal_grpc_proto
        ${service_sources}
        ${service_headers}
        ${gimbal_grpc_sources}
        ${gimbal_grpc_headers})

target_link_libraries(gimbal_grpc_proto
        ${_PROTOBUF_LIBPROTOBUF}
        ${_REFLECTION}
        ${_GRPC_GRPCPP})

find_package(MAVSDK REQUIRED)

target_link_libraries(gimbal_grpc_proto
    MAVSDK::mavsdk
)

foreach(_target service)
    add_executable(${_target} "${_target}.cc")
    target_link_libraries(${_target}
            gimbal_grpc_proto
            ${_PROTOBUF_LIBPROTOBUF}
            ${_REFLECTION}
            ${_GRPC_GRPCPP})
endforeach()