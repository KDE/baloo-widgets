add_definitions(-DTRANSLATION_DOMAIN=\"baloowidgets\")
add_subdirectory(naturalqueryparser)

set(widgets_SRCS
  kblocklayout.cpp
  tagwidget.cpp
  kedittagsdialog.cpp
  tagcheckbox.cpp
  filemetadatawidget.cpp
  filemetadataconfigwidget.cpp
  filemetadataprovider.cpp
  indexeddataretriever.cpp
  kcommentwidget.cpp
  keditcommentdialog.cpp
  metadatafilter.cpp
  widgetfactory.cpp
  groupedlineedit.cpp
  querybuilder.cpp
  querybuildercompleter.cpp
  filefetchjob.cpp
)

add_library(KF5BalooWidgets ${widgets_SRCS})

add_library(KF5::BalooWidgets ALIAS KF5BalooWidgets)

target_link_libraries(KF5BalooWidgets
  PUBLIC
  Qt5::Widgets
  Qt5::Core
  KF5::KIOCore # KFileItem
  KF5::CoreAddons # KProcess, KJob

  PRIVATE
  KF5::I18n
  KF5::Completion
  KF5::FileMetaData
  KF5::WidgetsAddons
  KF5::Baloo
  KF5::BalooNaturalQueryParser
  KF5::CoreAddons
  KF5::ConfigGui
)

set_target_properties(KF5BalooWidgets PROPERTIES
   VERSION ${BALOO_WIDGETS_VERSION_STRING}
   SOVERSION ${BALOO_WIDGETS_SOVERSION}
   EXPORT_NAME BalooWidgets
)

target_include_directories(KF5BalooWidgets INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/BalooWidgets>")

generate_export_header(KF5BalooWidgets BASE_NAME BALOO_WIDGETS EXPORT_FILE_NAME widgets_export.h)

install(TARGETS KF5BalooWidgets EXPORT KF5BalooWidgetsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(KF5BalooWidgets_CamelCase_HEADERS
    HEADER_NAMES
    TagWidget
    FileMetaDataWidget
    FileMetaDataConfigWidget
    QueryBuilder
    GroupedLineEdit

    PREFIX baloo
    REQUIRED_HEADERS KF5BalooWidgets_HEADERS
)

install(FILES
  ${KF5BalooWidgets_CamelCase_HEADERS}
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/BalooWidgets/Baloo
  COMPONENT Devel
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/widgets_export.h
  ${KF5BalooWidgets_HEADERS}
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/BalooWidgets/baloo
  COMPONENT Devel
)

#
# Extractor Process
#
add_executable(baloo_filemetadata_temp_extractor extractor.cpp)
target_link_libraries(baloo_filemetadata_temp_extractor
  Qt5::Core
  KF5::I18n
  KF5::FileMetaData
)

install(TARGETS baloo_filemetadata_temp_extractor DESTINATION ${BIN_INSTALL_DIR})
