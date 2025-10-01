cmake_minimum_required(VERSION 3.21)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/.cmake/)

# Initialize fallback values first
set(VERSION_SEMANTIC "1.0.0")
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(IDENTIFIER "com.sast.gallery")
set(COPYRIGHT "Copyright (c) 2025 SAST")

# Try to include Version, but continue with fallback if it fails
# Only include Version.cmake if we're in a git repository
find_package(Git QUIET)
if(Git_FOUND AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/.cmake/Version.cmake)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} -C ${CMAKE_CURRENT_LIST_DIR} rev-parse --git-dir
        RESULT_VARIABLE GIT_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    if(GIT_RESULT EQUAL 0)
        # We're in a git repository, safe to include Version.cmake
        include(Version)
        # Check if Version.cmake actually set the variables
        if(NOT DEFINED VERSION_SEMANTIC OR VERSION_SEMANTIC STREQUAL "")
            set(VERSION_SEMANTIC "1.0.0")
            message(WARNING "Version.cmake failed to set VERSION_SEMANTIC, using fallback: ${VERSION_SEMANTIC}")
        endif()
    else()
        message(STATUS "Not in a git repository, using fallback version: ${VERSION_SEMANTIC}")
    endif()
else()
    if(NOT Git_FOUND)
        message(STATUS "Git not found, using fallback version: ${VERSION_SEMANTIC}")
    else()
        message(STATUS "Version.cmake not found, using fallback version: ${VERSION_SEMANTIC}")
    endif()
endif()

project(sast-gallery VERSION ${VERSION_SEMANTIC} LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
  set(VERSION_RC_PATH ${CMAKE_SOURCE_DIR}/sast-gallery-version.rc)
  set(WIN32_MANIFEST_PATH ${CMAKE_SOURCE_DIR}/app.manifest)
  if(EXISTS ${CMAKE_SOURCE_DIR}/.cmake/version_exe.rc.in)
    configure_file(
        ${CMAKE_SOURCE_DIR}/.cmake/version_exe.rc.in
        ${VERSION_RC_PATH}
    )
  endif()
  if(EXISTS ${CMAKE_SOURCE_DIR}/.cmake/InstallerScript.iss.in)
    configure_file(
        ${CMAKE_SOURCE_DIR}/.cmake/InstallerScript.iss.in
        ${CMAKE_SOURCE_DIR}/action-cli/InstallerScript.iss
    )
  endif()
endif()

if(APPLE)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER ${IDENTIFIER})
    set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
    set(MACOSX_BUNDLE_COPYRIGHT ${COPYRIGHT})

    # Use PROJECT_VERSION instead of spectral_VERSION
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})

    set(ICON_NAME "res/icon/app.icns")
    set(${PROJECT_NAME}_MAC_ICON "${CMAKE_SOURCE_DIR}/${ICON_NAME}")
    set(MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
    if(EXISTS "${${PROJECT_NAME}_MAC_ICON}")
        set_property(SOURCE "${${PROJECT_NAME}_MAC_ICON}" PROPERTY
            MACOSX_PACKAGE_LOCATION Resources)
    endif()
endif()

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets Concurrent)

file(GLOB_RECURSE PROJECT_SOURCES
    src/*.cpp
    src/*.h
)

# Create list of source files, filtering out non-existent ones
set(EXECUTABLE_SOURCES ${PROJECT_SOURCES})
if(EXISTS "${CMAKE_SOURCE_DIR}/res/res.qrc")
    list(APPEND EXECUTABLE_SOURCES res/res.qrc)
endif()
if(WIN32 AND EXISTS "${CMAKE_SOURCE_DIR}/res/icon/app.rc")
    list(APPEND EXECUTABLE_SOURCES res/icon/app.rc)
endif()
if(WIN32 AND EXISTS "${VERSION_RC_PATH}")
    list(APPEND EXECUTABLE_SOURCES ${VERSION_RC_PATH})
endif()
if(WIN32 AND EXISTS "${WIN32_MANIFEST_PATH}")
    list(APPEND EXECUTABLE_SOURCES ${WIN32_MANIFEST_PATH})
endif()
if(APPLE AND EXISTS "${${PROJECT_NAME}_MAC_ICON}")
    list(APPEND EXECUTABLE_SOURCES ${${PROJECT_NAME}_MAC_ICON})
endif()

qt_add_executable(sast-gallery MACOSX_BUNDLE
    ${EXECUTABLE_SOURCES}
)

# Add ElaWidgetTools, use static library
set(ELAWIDGETTOOLS_BUILD_STATIC_LIB "STATIC" CACHE STRING "Build ElaWidgetTools as a static library")
if(EXISTS "${CMAKE_SOURCE_DIR}/3rdpart/ElaWidgetTools")
    add_subdirectory(3rdpart/ElaWidgetTools)
    
    target_include_directories(sast-gallery PRIVATE
        src
        3rdpart/ElaWidgetTools/src/include
    )
    
    set(ELA_WIDGET_AVAILABLE TRUE)
else()
    message(WARNING "ElaWidgetTools not found, building without it")
    target_include_directories(sast-gallery PRIVATE src)
    set(ELA_WIDGET_AVAILABLE FALSE)
endif()

# Link libraries
set(LINK_LIBRARIES
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::Concurrent
)

if(ELA_WIDGET_AVAILABLE)
    list(APPEND LINK_LIBRARIES ElaWidgetTools)
endif()

if(TARGET version::version)
    list(APPEND LINK_LIBRARIES version::version)
endif()

target_link_libraries(sast-gallery PRIVATE ${LINK_LIBRARIES})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(sast-gallery PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
    set_target_properties(sast-gallery PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER com.example.sast-gallery
    )
endif()

include(GNUInstallDirs)
install(TARGETS sast-gallery
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
