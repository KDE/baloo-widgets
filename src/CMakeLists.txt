add_definitions(-DTRANSLATION_DOMAIN=\"baloowidgets5\")

add_subdirectory(filepropertiesplugin)
add_subdirectory(tagsfileitemactionplugin)

set(widgets_SRCS
  kblocklayout.cpp
  tagwidget.cpp
  kedittagsdialog.cpp
  tagcheckbox.cpp
  filefetchjob.cpp
  filemetadatawidget.cpp
  filemetadataconfigwidget.cpp
  filemetadataprovider.cpp
  filemetadatautil.cpp
  kcommentwidget.cpp
  keditcommentdialog.cpp
  metadatafilter.cpp
  ondemandextractor.cpp
  widgetfactory.cpp
)

ecm_qt_declare_logging_category(
    widgets_SRCS
    HEADER "widgetsdebug.h"
    IDENTIFIER "Baloo::WIDGETS"
    DEFAULT_SEVERITY Warning
    CATEGORY_NAME "org.kde.baloo.widgets"
    DESCRIPTION "Baloo Widgets"
    EXPORT BALOO_WIDGETS
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
  KF5::FileMetaData
  KF5::WidgetsAddons
  KF5::Baloo
  KF5::CoreAddons
  KF5::ConfigGui
)

set_target_properties(KF5BalooWidgets PROPERTIES
   VERSION ${BALOO_WIDGETS_VERSION_STRING}
   SOVERSION ${BALOO_WIDGETS_SOVERSION}
   EXPORT_NAME BalooWidgets
)

target_include_directories(KF5BalooWidgets INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/BalooWidgets>")

generate_export_header(KF5BalooWidgets BASE_NAME BALOO_WIDGETS EXPORT_FILE_NAME widgets_export.h)

install(TARGETS KF5BalooWidgets EXPORT KF5BalooWidgetsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(KF5BalooWidgets_CamelCase_HEADERS
    HEADER_NAMES
    TagWidget
    FileMetaDataWidget
    FileMetaDataConfigWidget

    PREFIX baloo
    REQUIRED_HEADERS KF5BalooWidgets_HEADERS
)

install(FILES
  ${KF5BalooWidgets_CamelCase_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/BalooWidgets/Baloo
  COMPONENT Devel
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/widgets_export.h
  ${KF5BalooWidgets_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/BalooWidgets/baloo
  COMPONENT Devel
)

#
# Extractor Process
#
add_executable(baloo_filemetadata_temp_extractor extractor.cpp filemetadatautil.cpp)
target_link_libraries(baloo_filemetadata_temp_extractor
  Qt5::Core
  KF5::I18n
  KF5::FileMetaData
)

install(TARGETS baloo_filemetadata_temp_extractor DESTINATION ${KDE_INSTALL_BINDIR})

ecm_qt_install_logging_categories(
    EXPORT BALOO_WIDGETS
    FILE baloo-widgets.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
