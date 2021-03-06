file(GLOB UTILITY_SOURCES *.cpp)
set(LIBGROMACS_SOURCES ${LIBGROMACS_SOURCES} ${UTILITY_SOURCES} PARENT_SCOPE)

set(GENERATED_HEADER_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/gmx_header_config_gen.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gmx_header_config_gen.h.cmakein
               ${GENERATED_HEADER_CONFIG})

set(UTILITY_PUBLIC_HEADERS
    arrayref.h
    common.h
    errorcodes.h
    exceptions.h
    file.h
    flags.h
    gmx_header_config.h
    gmxassert.h
    programinfo.h
    stringutil.h
    uniqueptr.h)
install(FILES ${UTILITY_PUBLIC_HEADERS}
        DESTINATION ${INCL_INSTALL_DIR}/gromacs/utility
        COMPONENT development)
install(FILES ${GENERATED_HEADER_CONFIG}
        DESTINATION ${INCL_INSTALL_DIR}/gromacs/utility
        COMPONENT development)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif (BUILD_TESTING)
