cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "23")
set (RELEASE_SERVICE_VERSION_MINOR "07")
set (RELEASE_SERVICE_VERSION_MICRO "90")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(baloo-widgets VERSION ${RELEASE_SERVICE_VERSION})

if (BUILD_WITH_QT6)
    set(QT_MIN_VERSION "6.4.0")
    set(KF_MIN_VERSION "5.240.0")
else()
    set(QT_MIN_VERSION "5.15.2")
    set(KF_MIN_VERSION "5.92.0")
endif()

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


find_package(Qt${QT_MAJOR_VERSION} REQUIRED NO_MODULE COMPONENTS Core Widgets Test)
find_package(KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS Config KIO I18n FileMetaData Baloo Service)

ecm_setup_version(PROJECT
                  VARIABLE_PREFIX BALOO_WIDGETS
                  SOVERSION ${QT_MAJOR_VERSION}
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/baloowidgets_version.h"

                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF${QT_MAJOR_VERSION}BalooWidgetsConfigVersion.cmake")
ecm_set_disabled_deprecation_versions(QT 5.15.2
     KF 5.102.0
    )

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

add_subdirectory(src)
if (BUILD_TESTING)
  add_subdirectory(test)
  add_subdirectory(autotests)
endif()

ki18n_install(po)

# Config files
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF${QT_MAJOR_VERSION}BalooWidgets")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KFBalooWidgetsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF${QT_MAJOR_VERSION}BalooWidgetsConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF${QT_MAJOR_VERSION}BalooWidgetsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KF${QT_MAJOR_VERSION}BalooWidgetsConfigVersion.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT devel
)

install(EXPORT KF${QT_MAJOR_VERSION}BalooWidgetsTargets
        NAMESPACE KF${QT_MAJOR_VERSION}::
        DESTINATION ${CMAKECONFIG_INSTALL_DIR}
        FILE KF${QT_MAJOR_VERSION}BalooWidgetsTargets.cmake
)

feature_summary(WHAT ALL
    INCLUDE_QUIET_PACKAGES
    FATAL_ON_MISSING_REQUIRED_PACKAGES
)

file(GLOB_RECURSE CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
