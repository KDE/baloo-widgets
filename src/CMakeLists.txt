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
  metadatafilter.cpp
  widgetfactory.cpp
)

add_library(KF5BalooWidgets ${widgets_SRCS})

add_library(KF5::BalooWidgets ALIAS KF5BalooWidgets)

target_link_libraries(KF5BalooWidgets
  PUBLIC
  Qt5::Widgets
  KF5::KIOCore # KFileItem
  KF5::CoreAddons # KProcess, KJob

  PRIVATE
  Qt5::DBus
  KF5::I18n
  KF5::Completion
  KF5::FileMetaData
  KF5::WidgetsAddons
  KF5::KDELibs4Support
  KF5::BalooFiles
)

set_target_properties(KF5BalooWidgets PROPERTIES
   VERSION ${BALOO_WIDGETS_VERSION_STRING}
   SOVERSION ${BALOO_WIDGETS_SOVERSION}
   EXPORT_NAME BalooWidgets
)

generate_export_header(KF5BalooWidgets BASE_NAME BALOO_WIDGETS EXPORT_FILE_NAME widgets_export.h)

install(TARGETS KF5BalooWidgets EXPORT KF5BalooWidgetsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  tagwidget.h
  filemetadatawidget.h
  filemetadataconfigwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/widgets_export.h

  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/baloo
  COMPONENT Devel
)
