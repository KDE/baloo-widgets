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

find_package(KFileMetaData CONFIG)
set_package_properties(KFileMetaData PROPERTIES
    DESCRIPTION "A library for extracting file metadata"
    URL "https://projects.kde.org/kfilemetadata"
    TYPE REQUIRED
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

# Some definitions
# ================
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${BALOO_INCLUDE_DIR}
  ${KFILEMETADATA_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
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

add_subdirectory(src)
add_subdirectory(test)

# install the file with the exported targets
install(EXPORT BalooWidgetsLibraryTargets
        DESTINATION ${LIB_INSTALL_DIR}/cmake/BalooWidgets
        FILE BalooWidgetsTargetsWithPrefix.cmake
)
