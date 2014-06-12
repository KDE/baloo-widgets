set(ui_SRCS
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

set(widgets_SRCS ${ui_SRCS})

add_library(baloowidgets SHARED ${widgets_SRCS})

target_link_libraries(baloowidgets PUBLIC
  Qt5::Widgets
  KF5::KIOCore # KFileItem
  KF5::CoreAddons # KProcess, KJob
)
target_link_libraries(baloowidgets PRIVATE
  Qt5::DBus
  KF5::I18n
  KF5::Completion
  KF5::FileMetaData
  KF5::WidgetsAddons
  KF5::KDELibs4Support
  KF5::BalooFiles
)

set_target_properties(baloowidgets PROPERTIES
   VERSION ${BALOO_WIDGETS_VERSION_STRING}
   SOVERSION ${BALOO_WIDGETS_SOVERSION}
)

generate_export_header(baloowidgets BASE_NAME BALOO_WIDGETS EXPORT_FILE_NAME widgets_export.h)

install(TARGETS baloowidgets EXPORT BalooWidgetsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  tagwidget.h
  filemetadatawidget.h
  filemetadataconfigwidget.h

  ${CMAKE_CURRENT_BINARY_DIR}/widgets_export.h

  DESTINATION ${INCLUDE_INSTALL_DIR}/baloo COMPONENT Devel
)

