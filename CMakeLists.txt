cmake_minimum_required(VERSION 3.5)

project(diagnostic_mal_Med VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets PrintSupport REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets PrintSupport REQUIRED)


find_library(FFTW_LIB NAMES fftw3 fftw3-3 PATHS /usr/lib /usr/local/lib REQUIRED)
find_path(FFTW_INCLUDE_DIR NAMES fftw3.h PATHS /usr/include /usr/local/include REQUIRED)

include_directories(${FFTW_INCLUDE_DIR})


set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    signalwindows.cpp signalwindows.h signalwindows.ui
    paramswindows.cpp paramswindows.h paramswindows.ui
    qcustomplot.h qcustomplot.cpp devicesimulator.cpp devicesimulator.h
)


if(QT_VERSION_MAJOR VERSION_GREATER_EQUAL 6)
    qt_add_executable(diagnostic_mal_Med MANUAL_FINALIZATION ${PROJECT_SOURCES})
else()
    add_executable(diagnostic_mal_Med ${PROJECT_SOURCES})
endif()


target_link_libraries(diagnostic_mal_Med PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
    ${FFTW_LIB}
    m
)


set_target_properties(diagnostic_mal_Med PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(diagnostic_mal_Med)
endif()
