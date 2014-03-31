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

if (${BalooQueryParser_FOUND})
  set(querybuilder_SRCS
    groupedlineedit.cpp
    querybuilder.cpp
    querybuildercompleter.cpp
  )
endif(${BalooQueryParser_FOUND})

set(widgets_SRCS ${ui_SRCS} ${querybuilder_SRCS})

kde4_add_library(baloowidgets SHARED ${LIBRARY_TYPE} ${widgets_SRCS})

target_link_libraries(baloowidgets
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KIO_LIBS}
  ${BALOO_LIBRARIES}
  ${KFILEMETADATA_LIBRARY}
  ${BALOO_QUERYPARSER_LIBRARY}
)

target_link_libraries(baloowidgets
  LINK_INTERFACE_LIBRARIES
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${BALOO_LIBRARIES}
)

set_target_properties(baloowidgets PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS baloowidgets EXPORT BalooWidgetsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  widgets_export.h
  tagwidget.h
  filemetadatawidget.h
  filemetadataconfigwidget.h

  DESTINATION ${INCLUDE_INSTALL_DIR}/baloo COMPONENT Devel
)

