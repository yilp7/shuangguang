cmake_minimum_required(VERSION 3.5)

project(Demo_shuangguang LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Hik_SDK_DIR "${PROJECT_SOURCE_DIR}/sdk64/hik")
set(OpenCV_DIR "${PROJECT_SOURCE_DIR}/sdk64/opencv")

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets SerialPort Network REQUIRED)

include_directories(${Hik_SDK_DIR}/include)
include_directories(${OpenCV_DIR}/include)

link_directories(${Hik_SDK_DIR}/lib)
link_directories(${Hik_SDK_DIR}/lib/HCNetSDKCom)
link_directories(${OpenCV_DIR}/lib)
#link_directories(${OpenCV_DIR}/x64/mingw/lib)

add_executable(Demo WIN32
    main.cpp
    demo.cpp
	mylabel.cpp
    demo.h
	mylabel.h
	GeneralDef.h
    demo.ui
	img/resources.qrc
	icon.rc
)

qt5_use_modules(Demo Widgets SerialPort Network)

target_link_libraries(Demo
	Qt${QT_VERSION_MAJOR}::Widgets
	# hik-lib
	GdiPlus
	HCCore
	HCNetSDK
	PlayCtrl
	# HCNetSDKCom-lib
	HCAlarm
	HCGeneralCfgMgr
	HCPreview
	# opencv
    opencv_core454
    opencv_imgcodecs454
    opencv_imgproc454
    opencv_videoio454
)

add_library(plugin_ir_visible_light SHARED
    main.cpp
    demo.cpp
    mylabel.cpp
    plugin.cpp
    demo.h
    mylabel.h
    plugin.h
    plugininterface.h
    GeneralDef.h
    demo.ui
    img/resources.qrc
    icon.rc
    plugin_ir_visible_light.json
)

qt5_use_modules(plugin_ir_visible_light Widgets SerialPort Network)

add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_SHARED)
add_definitions(-DQT_NO_DEBUG)

target_link_libraries(plugin_ir_visible_light
    Qt${QT_VERSION_MAJOR}::Widgets
    # hik-lib
    GdiPlus
    HCCore
    HCNetSDK
    PlayCtrl
    # HCNetSDKCom-lib
    HCAlarm
    HCGeneralCfgMgr
    HCPreview
    # opencv
    opencv_core454
    opencv_imgcodecs454
    opencv_imgproc454
    opencv_videoio454
)
