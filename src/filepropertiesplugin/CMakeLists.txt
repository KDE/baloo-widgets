add_definitions(-DTRANSLATION_DOMAIN=\"baloowidgets5\")

kcoreaddons_add_plugin(baloofilepropertiesplugin INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/propertiesdialog" SOURCES baloofilepropertiesplugin.cpp baloofilepropertiesplugin.h)

target_link_libraries(baloofilepropertiesplugin
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::KIOWidgets
    KF${QT_MAJOR_VERSION}::BalooWidgets
)
