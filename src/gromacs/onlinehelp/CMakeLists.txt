file(GLOB ONLINEHELP_SOURCES *.cpp)
set(LIBGROMACS_SOURCES ${LIBGROMACS_SOURCES} ${ONLINEHELP_SOURCES} PARENT_SCOPE)

set(ONLINEHELP_PUBLIC_HEADERS
    helptopicinterface.h)
install(FILES ${ONLINEHELP_PUBLIC_HEADERS}
        DESTINATION ${INCL_INSTALL_DIR}/gromacs/onlinehelp
        COMPONENT development)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif (BUILD_TESTING)
