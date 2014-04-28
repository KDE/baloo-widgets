project(baloo-widgets)
cmake_minimum_required(VERSION 2.8.12)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${ECM_MODULE_PATH})

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Widgets Xml Test Sql)
find_package(KF5 REQUIRED COMPONENTS KDELibs4Support I18n Service)
find_package(KFileMetaData 4.97 REQUIRED)
find_package(Baloo 4.97 REQUIRED)

include(ECMSetupVersion)
include(ECMAddTests)
include(ECMInstallIcons)

include(GenerateExportHeader)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(${KDE4_ENABLE_EXCEPTIONS})

set(KF5_VERSION "5.0.0")
ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX BALOO_WIDGETS
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/baloowidgets_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/BalooWidgetsConfigVersion.cmake")

include_directories(
  ${CMAKE_SOURCE_DIR}/src
)

add_subdirectory(src)
add_subdirectory(test)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/BalooWidgetsConfig.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/BalooWidgetsConfig.cmake @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/BalooWidgetsConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/BalooWidgetsConfigVersion.cmake
        DESTINATION ${LIB_INSTALL_DIR}/cmake/BalooWidgets)
install(EXPORT BalooWidgetsLibraryTargets
        DESTINATION ${LIB_INSTALL_DIR}/cmake/BalooWidgets
        FILE BalooWidgetsTargetsWithPrefix.cmake)


feature_summary(WHAT ALL
    INCLUDE_QUIET_PACKAGES
    FATAL_ON_MISSING_REQUIRED_PACKAGES
)
