add_definitions(-DTRANSLATION_DOMAIN=\"baloowidgets5\")

add_subdirectory(filepropertiesplugin)
add_subdirectory(tagsfileitemactionplugin)

add_library(KF6BalooWidgets)
add_library(KF6::BalooWidgets ALIAS KF6BalooWidgets)
set_target_properties(KF6BalooWidgets PROPERTIES
    VERSION     ${BALOO_WIDGETS_VERSION}
    SOVERSION   ${BALOO_WIDGETS_SOVERSION}
    EXPORT_NAME BalooWidgets
)

target_sources(KF6BalooWidgets PRIVATE
    extractor.cpp
    extractorutil_p.h
    filefetchjob.cpp
    filefetchjob.h
    filemetadataconfigwidget.cpp
    filemetadataconfigwidget.h
    filemetadataprovider.cpp
    filemetadataprovider.h
    filemetadatautil.cpp
    filemetadatautil_p.h
    filemetadatawidget.cpp
    filemetadatawidget.h
    kblocklayout.cpp
    kblocklayout.h
    kcommentwidget.cpp
    kcommentwidget_p.h
    keditcommentdialog.cpp
    keditcommentdialog.h
    kedittagsdialog.cpp
    kedittagsdialog_p.h
    metadatafilter.cpp
    metadatafilter.h
    ondemandextractor.cpp
    ondemandextractor.h
    tagcheckbox.cpp
    tagcheckbox.h
    tagwidget.cpp
    tagwidget.h
    widgetfactory.cpp
    widgetfactory.h
)

ecm_qt_declare_logging_category(KF6BalooWidgets
    HEADER "widgetsdebug.h"
    IDENTIFIER "Baloo::WIDGETS"
    DEFAULT_SEVERITY Warning
    CATEGORY_NAME "org.kde.baloo.widgets"
    DESCRIPTION "Baloo Widgets"
    EXPORT BALOO_WIDGETS
)

target_link_libraries(KF6BalooWidgets
  PUBLIC
  Qt6::Widgets
  Qt6::Core
  KF6::KIOCore # KFileItem
  KF6::CoreAddons # KProcess, KJob

  PRIVATE
  KF6::I18n
  KF6::FileMetaData
  KF6::WidgetsAddons
  KF6::Baloo
  KF6::CoreAddons
  KF6::ConfigGui
  KF6::Service
)

target_include_directories(KF6BalooWidgets INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/BalooWidgets>")

ecm_generate_export_header(KF6BalooWidgets
    BASE_NAME BALOO_WIDGETS
    EXPORT_FILE_NAME widgets_export.h
    VERSION ${BALOO_WIDGETS_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 23.08
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

install(TARGETS KF6BalooWidgets EXPORT KF6BalooWidgetsTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(KFBalooWidgets_CamelCase_HEADERS
    HEADER_NAMES
    TagWidget
    FileMetaDataWidget
    FileMetaDataConfigWidget

    PREFIX baloo
    REQUIRED_HEADERS KFBalooWidgets_HEADERS
)

install(FILES
  ${KFBalooWidgets_CamelCase_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/BalooWidgets/Baloo
  COMPONENT Devel
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/widgets_export.h
  ${KFBalooWidgets_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/BalooWidgets/baloo
  COMPONENT Devel
)

#
# Extractor Process
#
add_executable(baloo_filemetadata_temp_extractor extractor.cpp)
target_link_libraries(baloo_filemetadata_temp_extractor
  Qt6::Core
  KF6::I18n
  KF6::FileMetaData
)

install(TARGETS baloo_filemetadata_temp_extractor DESTINATION ${KDE_INSTALL_BINDIR})

ecm_qt_install_logging_categories(
    EXPORT BALOO_WIDGETS
    FILE baloo-widgets.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
