add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

include_directories( ${KDE4_INCLUDES} )
add_definitions( ${KDE4_DEFINITIONS} )

set(videocapture_LIB_SRCS bayer.cpp  sonix_compress.cpp  videodevice.cpp  videodevicemodelpool.cpp  videodevicepool.cpp  videoinput.cpp)

kde4_add_library(videocapture SHARED ${videocapture_LIB_SRCS} )

target_link_libraries(videocapture  ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS})

install(TARGETS videocapture ${INSTALL_TARGETS_DEFAULT_ARGS})
