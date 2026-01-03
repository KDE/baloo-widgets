cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "25")
set (RELEASE_SERVICE_VERSION_MINOR "12")
set (RELEASE_SERVICE_VERSION_MICRO "1")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(baloo-widgets VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "6.7.0")
set(KF_MIN_VERSION "6.4.0")

# ECM setup
find_package(ECM ${KF_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include(QtVersionOption)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)

include(ECMInstallIcons)

include(FeatureSummary)
include(ECMGenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMAddTests)
include(ECMQtDeclareLoggingCategory)
include(CMakePackageConfigHelpers)
include(ECMDeprecationSettings)


find_package(Qt6 REQUIRED NO_MODULE COMPONENTS Core Widgets Test)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Config KIO I18n FileMetaData Baloo Service)

ecm_setup_version(PROJECT
                  VARIABLE_PREFIX BALOO_WIDGETS
                  SOVERSION 6
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/baloowidgets_version.h"

                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6BalooWidgetsConfigVersion.cmake")
ecm_set_disabled_deprecation_versions(QT 5.15.2
     KF 6.4
    )

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

add_subdirectory(src)
if (BUILD_TESTING)
  add_subdirectory(test)
  add_subdirectory(autotests)
endif()

ki18n_install(po)

# Config files
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6BalooWidgets")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KFBalooWidgetsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6BalooWidgetsConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF6BalooWidgetsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6BalooWidgetsConfigVersion.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT devel
)

install(EXPORT KF6BalooWidgetsTargets
        NAMESPACE KF6::
        DESTINATION ${CMAKECONFIG_INSTALL_DIR}
        FILE KF6BalooWidgetsTargets.cmake
)

feature_summary(WHAT ALL
    INCLUDE_QUIET_PACKAGES
    FATAL_ON_MISSING_REQUIRED_PACKAGES
)

file(GLOB_RECURSE CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
