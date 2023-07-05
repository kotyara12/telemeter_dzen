# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)
idf_component_register(SRCS ${app_sources})

target_add_binary_data(${COMPONENT_TARGET} C:/PlatformIO/libs/certs/isrg_root_x1.pem TEXT)
target_add_binary_data(${COMPONENT_TARGET} C:/PlatformIO/libs/certs/digi_cert.pem TEXT)
target_add_binary_data(${COMPONENT_TARGET} C:/PlatformIO/libs/certs/api_telegram_org.pem TEXT)

