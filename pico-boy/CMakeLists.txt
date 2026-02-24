cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)
project(Pico_ePaper_Code)
pico_sdk_init()

# add compiled files subdirectory
add_subdirectory(lib/Config)
add_subdirectory(lib/Infrared)
add_subdirectory(lib/Icm20948)
add_subdirectory(lib/LCD)
add_subdirectory(lib/Fonts)
add_subdirectory(lib/GUI)



# add header file directory
include_directories(./lib/Config)
include_directories(./lib/GUI)
include_directories(./lib/Icm20948)
include_directories(./lib/LCD)
include_directories(./lib/Infrared)


# generate executable file
add_executable(main 
main.cpp
)

# create map/bin/hex/uf2 file etc.
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

pico_add_extra_outputs(main)

target_link_libraries(main Config LCD Infrared Icm20948 GUI Fonts pico_stdlib hardware_spi hardware_dma)

# use picotool to boot newly generated uf2 file onto the picoboy
add_custom_command(
    TARGET main
    POST_BUILD
    COMMAND ./../picotool/build/picotool load -x main.uf2 -f
    COMMENT "uploading new uf2 to picoboy..."
)
