project(nepomuk-widgets)

set(CMAKE_NEPOMUK_WIDGETS_VERSION_MAJOR 0)
set(CMAKE_NEPOMUK_WIDGETS_VERSION_MINOR 1)
set(CMAKE_NEPOMUK_WIDGETS_VERSION_RELEASE 0)
set(CMAKE_NEPOMUK_WIDGETS_VERSION_STRING "${CMAKE_NEPOMUK_WIDGETS_VERSION_MAJOR}.${CMAKE_NEPOMUK_WIDGETS_VERSION_MINOR}.${CMAKE_NEPOMUK_WIDGETS_VERSION_RELEASE}")

# Search for the required packages
# ================================
set(KDE_MIN_VERSION "4.7.00")
find_package(KDE4 4.7.00 REQUIRED)
include(KDE4Defaults)

find_package(NepomukCore REQUIRED)

macro_log_feature(NepomukCore_FOUND "NepomukCore" "NepomukCore contains the core Nepomuk libraries" "http://nepomuk.kde.org" TRUE "Required for running Nepomuk")
macro_display_feature_log()

# Some definitions
# ================
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${SOPRANO_INCLUDE_DIR}
)


# generate NepomukWidgetsConfig.cmake for easy utilisation of the package by other cmake build systems
# ===============================================================================================
configure_file(NepomukWidgetsConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/NepomukWidgetsConfig.cmake @ONLY)

# Generate NepomukWidgetsConfigVersion.cmake
macro_write_basic_cmake_version_file(${CMAKE_CURRENT_BINARY_DIR}/NepomukWidgetsConfigVersion.cmake
                                     ${CMAKE_NEPOMUK_WIDGETS_VERSION_MAJOR}
                                     ${CMAKE_NEPOMUK_WIDGETS_VERSION_MINOR}
                                     ${CMAKE_NEPOMUK_WIDGETS_VERSION_RELEASE}
                                     )
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/NepomukWidgetsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/NepomukWidgetsConfigVersion.cmake
    DESTINATION ${LIB_INSTALL_DIR}/cmake/NepomukWidgets)

set(nepomuk_ui_SRCS
  ui/kblocklayout.cpp
  ui/nepomukmassupdatejob.cpp
  ui/tagwidget.cpp
  ui/kedittagsdialog.cpp
  ui/tagcheckbox.cpp
)

kde4_add_library(nepomukwidgets ${LIBRARY_TYPE} ${nepomuk_ui_SRCS})

target_link_libraries(nepomukwidgets
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${SOPRANO_LIBRARIES}
  ${NEPOMUK_CORE_LIBRARY}
)

target_link_libraries(nepomukwidgets
  LINK_INTERFACE_LIBRARIES
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${SOPRANO_LIBRARIES}
)

set_target_properties(nepomukwidgets PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS nepomukwidgets EXPORT NepomukWidgetsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ui/tagwidget.h

  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk2 COMPONENT Devel
)

#add_subdirectory(utils)
