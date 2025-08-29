# CMakeLists.txt for Audiobook Player

cmake_minimum_required(VERSION 3.20)
project(AudiobookPlayer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ==================== BẮT ĐẦU CẢI TIẾN ====================
# Thêm module 'Sql' để sử dụng SQLite
find_package(Qt6 COMPONENTS Core Gui Widgets Multimedia Sql REQUIRED)
# ===================== KẾT THÚC CẢI TIẾN =====================

# --- START: FFmpeg Configuration ---
set(FFMPEG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg)
set(FFMPEG_INCLUDE_DIR ${FFMPEG_DIR}/include)
set(FFMPEG_LIBRARY_DIR ${FFMPEG_DIR}/lib)

if(EXISTS ${FFMPEG_INCLUDE_DIR} AND EXISTS ${FFMPEG_LIBRARY_DIR})
    message(STATUS "Found local FFmpeg at: ${FFMPEG_DIR}")
else()
    message(FATAL_ERROR "FFmpeg not found in project root directory. Please extract it to a folder named 'ffmpeg'.")
endif()
# --- END: FFmpeg Configuration ---

set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/PlayerWidget.cpp
    src/LibraryWidget.cpp
    src/SettingsDialog.cpp
    src/AudioEngine.cpp
    src/BookModel.cpp
    src/ChapterModel.cpp
    src/SpeedControlPopup.cpp
    src/TimeInputDialog.cpp
    src/ButtonDelegate.cpp
    src/SleepTimerDialog.cpp
    # ==================== BẮT ĐẦU CẢI TIẾN ====================
    # Thêm các file quản lý database
    src/DatabaseManager.cpp
    # ===================== KẾT THÚC CẢI TIẾN =====================
)

set(PROJECT_RESOURCES src/resources.qrc)

add_executable(${PROJECT_NAME} WIN32 "") 

qt_add_resources(${PROJECT_NAME} ${PROJECT_RESOURCES})

if(WIN32)
    message(STATUS "Windows platform detected. Adding icon resource file.")
    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app_icon.rc)
endif()

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIR})

target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARY_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets 
    Qt6::Multimedia
    # ==================== BẮT ĐẦU CẢI TIẾN ====================
    Qt6::Sql
    # ===================== KẾT THÚC CẢI TIẾN =====================
    avformat
    avcodec
    avutil
    swresample
    avfilter
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE ON
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
    LIBRARY DESTINATION lib
)
