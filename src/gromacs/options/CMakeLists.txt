file(GLOB OPTIONS_SOURCES *.cpp)
set(LIBGROMACS_SOURCES ${LIBGROMACS_SOURCES} ${OPTIONS_SOURCES} PARENT_SCOPE)

set(OPTIONS_PUBLIC_HEADERS
    abstractoption.h
    basicoptions.h
    filenameoption.h
    optionfiletype.h
    optionflags.h
    options.h
    timeunitmanager.h)
install(FILES ${OPTIONS_PUBLIC_HEADERS}
        DESTINATION ${INCL_INSTALL_DIR}/gromacs/options
        COMPONENT development)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif (BUILD_TESTING)
