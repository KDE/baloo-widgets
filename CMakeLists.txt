cmake_minimum_required(VERSION 3.0)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "20")
set (RELEASE_SERVICE_VERSION_MINOR "11")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(baloo-widgets VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "5.11.0")
set(KF5_MIN_VERSION "5.70.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${ECM_MODULE_PATH})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Widgets Test)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Config KIO I18n FileMetaData Baloo)

include(ECMSetupVersion)
include(ECMInstallIcons)

include(GenerateExportHeader)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMGenerateHeaders)
include(ECMAddTests)
include(ECMQtDeclareLoggingCategory)
include(CMakePackageConfigHelpers)

add_definitions(-DQT_NO_FOREACH)

ecm_setup_version(PROJECT
                  VARIABLE_PREFIX BALOO_WIDGETS
                  SOVERSION 5
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/baloowidgets_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5BalooWidgetsConfigVersion.cmake")

include_directories(
   ${CMAKE_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/src
)
add_subdirectory(src)
if (BUILD_TESTING)
  add_subdirectory(test)
  add_subdirectory(autotests)
endif()

# Config files
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5BalooWidgets")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF5BalooWidgetsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5BalooWidgetsConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF5BalooWidgetsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5BalooWidgetsConfigVersion.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT devel
)

install(EXPORT KF5BalooWidgetsTargets
        NAMESPACE KF5::
        DESTINATION ${CMAKECONFIG_INSTALL_DIR}
        FILE KF5BalooWidgetsTargets.cmake
)

install(FILES baloo-widgets.categories  DESTINATION  ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

feature_summary(WHAT ALL
    INCLUDE_QUIET_PACKAGES
    FATAL_ON_MISSING_REQUIRED_PACKAGES
)
