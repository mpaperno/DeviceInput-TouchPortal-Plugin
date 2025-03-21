cmake_minimum_required(VERSION 3.24)

set (VERSION_STR 1.0.0.1)
set (VERSION_SFX beta2)
set (PROJECT_FULL_NAME  "Device Input Plugin for Touch Portal")
set (PROJECT_SHORT_NAME "Device Input Plugin")
set (PROJECT_ORG_NAME   "${PROJECT_FULL_NAME}")
set (PROJECT_DOMAIN "di.tpp.max.paperno.us")
set (TP_STATE_NAME_PREFIX "Device Input")
set (TP_PLUGIN_ID   "us.paperno.max.tpp.di")

project(
  DeviceInputPlugin
  VERSION ${VERSION_STR}
	HOMEPAGE_URL "https://github.com/mpaperno/DeviceInput-TouchPortal-Plugin"
  DESCRIPTION "This plugin allows using external hardware devices, such as game controllers, keyboards, etc, as inputs for Touch Portal."
  LANGUAGES CXX
)

math(EXPR VERSION_NUM
	"((${PROJECT_VERSION_MAJOR} & 0xFF) << 24) | ((${PROJECT_VERSION_MINOR} & 0xFF) << 16) | ((${PROJECT_VERSION_PATCH} & 0xFF) << 8) | (${PROJECT_VERSION_TWEAK} & 0xFF)"
	OUTPUT_FORMAT HEXADECIMAL
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if (WIN32)
  option(USE_WINDOWS_HOOK "Include Windows low-level keyboard/mouse hooking code." TRUE)
endif()

cmake_path(SET SRCPATH "${PROJECT_SOURCE_DIR}")
#cmake_path(SET DOXPATH "${CMAKE_SOURCE_DIR}/../doc/doxygen")
cmake_path(SET BLDPATH "${CMAKE_SOURCE_DIR}/../build")
cmake_path(SET LOCAL_LIB_DIR "${SRCPATH}/lib")

set(isWin FALSE)
set(isMac FALSE)
set(isLinux FALSE)

if(WIN32)
  set(PLATFORM_OS Windows)
  set(isWin TRUE)
elseif(APPLE)
  set(PLATFORM_OS MacOS)
  set(isMac TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(PLATFORM_OS Linux)
  set(isLinux TRUE)
else()
  message(FATAL_ERROR "Platform not supported.")
endif()

find_package(QT NAMES Qt5 Qt6 REQUIRED)

cmake_path(SET INSTALL_DEST_REL "${CMAKE_SOURCE_DIR}/../dist/${PLATFORM_OS}/${PROJECT_NAME}")
cmake_path(SET INSTALL_DEST_DBG "${CMAKE_SOURCE_DIR}/../dist/${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${INSTALL_DEST_DBG}/bin")
	# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${INSTALL_DEST_DBG}/bin")
	# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${INSTALL_DEST_DBG}/bin")
# else()
	# use static msvc runtime (but Qt uses dynamic anyway)
	# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

add_executable(${PROJECT_NAME}

  main.cpp
	logging.h
	strings.h
  utils.h
  version.h
  version.h.in

  Logger.h
  Logger.cpp
  Plugin.h
  Plugin.cpp
  RunGuard.h

	device/devices.h
	device/events.h
	device/DeviceDescriptor.h
  device/DeviceManager.h
  device/DeviceManager.cpp
	device/InputDevice.h
	device/InputDevice.cpp
	device/IApiManager.h
	device/SDLManager.h
  device/SDLManager.cpp

#  resources/resources.qrc
)

if (WIN32)
  target_sources(${PROJECT_NAME} PRIVATE
		device/WindowsDeviceManager.h
		device/WindowsDeviceManager.cpp

		resources/win.rc
	)
	target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN)
	if (USE_WINDOWS_HOOK)
		target_compile_definitions(${PROJECT_NAME} PRIVATE USE_WINDOWS_HOOK)
	endif()
endif()

configure_file(version.h.in version.h @ONLY)
configure_file(${BLDPATH}/version.json.in ${BLDPATH}/version.json @ONLY)
#configure_file(${DOXPATH}/Doxyfile.version.in ${DOXPATH}/Doxyfile.version @ONLY)

target_include_directories(${PROJECT_NAME} PRIVATE
	device
  "${CMAKE_CURRENT_BINARY_DIR}"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  MACOSX_BUNDLE TRUE
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  QT_USE_QSTRINGBUILDER
  QT_MESSAGELOGCONTEXT
  #QT_NO_KEYWORDS
)

if (MSVC)
  target_compile_definitions(${PROJECT_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
		# _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR
  )
endif()

if (UNIX OR MINGW AND NOT APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs )
endif()

## Qt
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
  Core
)
target_link_libraries(${PROJECT_NAME} PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
)

## TPClientQt
add_subdirectory("${LOCAL_LIB_DIR}/TPClientQt" EXCLUDE_FROM_ALL)
target_link_libraries(${PROJECT_NAME} PRIVATE TPClientQt)

## SDL
add_subdirectory("${LOCAL_LIB_DIR}/SDL" EXCLUDE_FROM_ALL)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

##  Install


if (CMAKE_BUILD_TYPE STREQUAL "Release")
  set(INSTALL_DEST ${INSTALL_DEST_REL})
else()
  set(INSTALL_DEST ${INSTALL_DEST_DBG})
endif()
set(CMAKE_INSTALL_PREFIX "${INSTALL_DEST}")
#set(CMAKE_INSTALL_PREFIX "$<IF:$<CONFIG:Release>,\${INSTALL_DEST_REL},\${INSTALL_DEST_DBG}>")

install(TARGETS ${PROJECT_NAME}
  BUNDLE  DESTINATION .
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
  RUNTIME DESTINATION bin
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
)

if (isWin)
  set(executable_path "${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:${PROJECT_NAME}>")
  set(deploy_command "${WINDEPLOYQT_EXECUTABLE}" --no-compiler-runtime --no-translations "${executable_path}")
  install(CODE "execute_process(COMMAND ${deploy_command})" COMPONENT Runtime)

elseif(isLinux)
  set_target_properties(${PROJECT_NAME} PROPERTIES
    CMAKE_BUILD_RPATH_USE_ORIGIN TRUE
    INSTALL_RPATH "$ORIGIN/lib"
  )
  set(install_dest "${CMAKE_INSTALL_PREFIX}/bin")
  #install(TARGETS ${PROJECT_NAME} RUNTIME_DEPENDENCIES DESTINATION bin)  # all C++ libs
  set(qt_ver ${QT_VERSION_MAJOR})
  set(qt_libs "")
  foreach(qt_lib
    Qt${qt_ver}Core.so.${qt_ver} Qt${qt_ver}Network.so.${qt_ver} Qt${qt_ver}WlShellIntegration.so.${qt_ver}
    #icui18n.so.56 icuuc.so.56 icudata.so.56
  )
    list(APPEND qt_libs "${CMAKE_PREFIX_PATH}/lib/lib${qt_lib}")
  endforeach()
  install(CODE "
    file(INSTALL ${qt_libs} DESTINATION \"${install_dest}/lib\" FOLLOW_SYMLINK_CHAIN)
    file(WRITE \"${install_dest}/qt.conf\" \"[Paths]\nPlugins=plugins\nLibraries=lib\n\")
    " COMPONENT Runtime)
  foreach(qt_plugin
    networkinformation
    platforms
    platforminputcontexts
  )
    install(DIRECTORY "${CMAKE_PREFIX_PATH}/plugins/${qt_plugin}" DESTINATION "${install_dest}/plugins")
  endforeach()

elseif(isMac)
  set_target_properties(${PROJECT_NAME} PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH "@executable_path/../Frameworks/"
  )
  # macdeployqt is only needed when completely rebuilding the bundle; It also adds a bunch of GUI QApplication junk we don't need.
  set(bundle_path "${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:${PROJECT_NAME}>.app")
  add_custom_target(deployqt
    COMMAND ${MACDEPLOYQT_EXECUTABLE} "${bundle_path}"
    COMMAND rm -f -r "${bundle_path}/Contents/Frameworks/QtWidgets.framework"
    COMMAND rm -f -r "${bundle_path}/Contents/Plugins/styles"
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
  )
endif()
