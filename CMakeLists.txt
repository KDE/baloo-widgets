project(baloo-widgets)

set(CMAKE_BALOO_WIDGETS_VERSION_MAJOR 0)
set(CMAKE_BALOO_WIDGETS_VERSION_MINOR 1)
set(CMAKE_BALOO_WIDGETS_VERSION_RELEASE 0)
set(CMAKE_BALOO_WIDGETS_VERSION_STRING "${CMAKE_BALOO_WIDGETS_VERSION_MAJOR}.${CMAKE_BALOO_WIDGETS_VERSION_MINOR}.${CMAKE_BALOO_WIDGETS_VERSION_RELEASE}")

# Search for the required packages
# ================================
find_package(KDE4 4.10 REQUIRED)
include(KDE4Defaults)

find_package(Baloo)
set_package_properties(Baloo PROPERTIES DESCRIPTION "The Baloo libraries" URL "http://kde.org"
                       TYPE REQUIRED PURPOSE "Required for compiling Baloo")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

# Some definitions
# ================
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${BALOO_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/ui
)


# generate BalooWidgetsConfig.cmake for easy utilisation of the package by other cmake build systems
# ===============================================================================================
configure_file(BalooWidgetsConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/BalooWidgetsConfig.cmake @ONLY)

# Generate BalooWidgetsConfigVersion.cmake
macro_write_basic_cmake_version_file(${CMAKE_CURRENT_BINARY_DIR}/BalooWidgetsConfigVersion.cmake
                                     ${CMAKE_BALOO_WIDGETS_VERSION_MAJOR}
                                     ${CMAKE_BALOO_WIDGETS_VERSION_MINOR}
                                     ${CMAKE_BALOO_WIDGETS_VERSION_RELEASE}
                                     )
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/BalooWidgetsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/BalooWidgetsConfigVersion.cmake
    DESTINATION ${LIB_INSTALL_DIR}/cmake/BalooWidgets)

set(ui_SRCS
  ui/kblocklayout.cpp
  ui/tagwidget.cpp
  ui/kedittagsdialog.cpp
  ui/tagcheckbox.cpp
  ui/filemetadatawidget.cpp
  ui/filemetadataconfigwidget.cpp
  ui/filemetadataprovider.cpp
  ui/indexeddataretriever.cpp
  ui/kcommentwidget.cpp
  ui/knfotranslator.cpp
  ui/metadatafilter.cpp
  ui/widgetfactory.cpp
)

set(widgets_SRCS ${ui_SRCS})

kde4_add_library(baloowidgets SHARED ${LIBRARY_TYPE} ${widgets_SRCS})

target_link_libraries(baloowidgets
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${BALOO_LIBRARIES}
)

target_link_libraries(baloowidgets
  LINK_INTERFACE_LIBRARIES
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${BALOO_LIBRARIES}
)

set_target_properties(baloowidgets PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS baloowidgets EXPORT BalooWidgetsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  widgets_export.h
  ui/tagwidget.h
  ui/filemetadatawidget.h
  ui/filemetadataconfigwidget.h

  DESTINATION ${INCLUDE_INSTALL_DIR}/baloo COMPONENT Devel
)

add_subdirectory(test)

# install the file with the exported targets
install(EXPORT BalooWidgetsLibraryTargets
        DESTINATION ${LIB_INSTALL_DIR}/cmake/BalooWidgets
        FILE BalooWidgetsTargetsWithPrefix.cmake
)
