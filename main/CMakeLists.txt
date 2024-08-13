# Glob all C source files under src/driver and src/application directories
file(GLOB_RECURSE DRIVER_SOURCES "src/driver/*.c")
file(GLOB_RECURSE APPLICATION_SOURCES "src/application/*.c")

# Combine all source files
list(APPEND SOURCES ${DRIVER_SOURCES} ${APPLICATION_SOURCES})

# Register component with the combined sources and include directories
idf_component_register(
    SRCS "main.c" ${SOURCES}
    INCLUDE_DIRS "include" "include/driver" "include/application"
)
