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
find_package(libssh2 REQUIRED)

include_directories(${Hik_SDK_DIR}/include)
include_directories(${OpenCV_DIR}/include)

link_directories(${Hik_SDK_DIR}/lib)
link_directories(${Hik_SDK_DIR}/lib/HCNetSDKCom)
link_directories(${OpenCV_DIR}/lib)
#link_directories(${OpenCV_DIR}/x64/mingw/lib)

add_executable(Shuangguang WIN32
    main.cpp
    ftpsettings.cpp
    ftpsettings.h
    ftpsettings.ui
    mylabel.cpp
    mylabel.h
    demo.cpp
    demo.h
    demo.ui
    GeneralDef.h
    img/resources.qrc
    icon.rc
)

qt5_use_modules(Shuangguang Widgets SerialPort Network)

target_compile_definitions(Shuangguang PRIVATE DISPLAY=1 CONTROL=1)

target_link_libraries(Shuangguang
    Qt${QT_VERSION_MAJOR}::Widgets
    # libssh2
    Libssh2::libssh2
    ws2_32
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
    ftpsettings.cpp
    ftpsettings.h
    ftpsettings.ui
    demo.cpp
    demo.h
    demo.ui
    mylabel.cpp
    mylabel.h
    ir_visible_light/plugin.cpp
    ir_visible_light/plugin.h
    ir_visible_light/plugininterface.h
    GeneralDef.h
    img/resources.qrc
    icon.rc
    ir_visible_light/plugin_ir_visible_light.json
)

qt5_use_modules(plugin_ir_visible_light Widgets SerialPort Network)

target_compile_definitions(plugin_ir_visible_light PRIVATE -DQT_PLUGIN)
target_compile_definitions(plugin_ir_visible_light PRIVATE -DQT_SHARED)
target_compile_definitions(plugin_ir_visible_light PRIVATE -DQT_NO_DEBUG)
target_compile_definitions(plugin_ir_visible_light PRIVATE DISPLAY=1 CONTROL=1)

target_link_libraries(plugin_ir_visible_light
    Qt${QT_VERSION_MAJOR}::Widgets
    # libssh2
    Libssh2::libssh2
    ws2_32
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

add_library(plugin_hik_control SHARED
    main.cpp
    hikcontrol.cpp
    hikcontrol.h
    hikcontrol.ui
    hik_control/plugin.cpp
    hik_control/plugin.h
    hik_control/plugininterface.h
    GeneralDef.h
    img/resources.qrc
    hik_control/plugin_hik_control.json
)

qt5_use_modules(plugin_hik_control Widgets SerialPort Network)

target_compile_definitions(plugin_hik_control PRIVATE -DQT_PLUGIN)
target_compile_definitions(plugin_hik_control PRIVATE -DQT_SHARED)
target_compile_definitions(plugin_hik_control PRIVATE -DQT_NO_DEBUG)
target_compile_definitions(plugin_hik_control PRIVATE DISPLAY=0 CONTROL=1)

target_link_libraries(plugin_hik_control
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
)
