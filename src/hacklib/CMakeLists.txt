SET(PROJ_NAME hacklib)
PROJECT(${PROJ_NAME})

SET(FILES_CPP
    src/Main.cpp
    src/Injector.cpp
    src/Hooker.cpp
    )
SET(FILES_H
    include/hacklib/MessageBox.h
    include/hacklib/Main.h
    include/hacklib/Injector.h
    include/hacklib/ForeignClass.h
    )

# windows specific
IF(WIN32)
    SET(FILES_CPP ${FILES_CPP}
        src/MessageBox_WIN32.cpp
        src/D3DDeviceFetcher_WIN32.cpp
        src/Drawer_WIN32.cpp
        src/GfxOverlay_WIN32.cpp
        src/WindowOverlay_WIN32.cpp
        src/PatternScanner.cpp  # TODO this is not really windows specific
        src/ConsoleEx.cpp
        )
    SET(FILES_H ${FILES_H}
        include/hacklib/D3DDeviceFetcher.h
        include/hacklib/Drawer.h
        include/hacklib/GfxOverlay.h
        include/hacklib/WindowOverlay.h
        include/hacklib/ConsoleEx.h
        )
ENDIF()

ADD_LIBRARY(${PROJ_NAME} STATIC
    ${FILES_CPP}
    ${FILES_H})

SET_TARGET_PROPERTIES(${PROJ_NAME} PROPERTIES FOLDER ${PROJ_NAME})

TARGET_INCLUDE_DIRECTORIES(${PROJ_NAME} PUBLIC include)

IF(WIN32)
    TARGET_LINK_LIBRARIES(${PROJ_NAME} d3d9.lib d3dx9.lib)
ENDIF()