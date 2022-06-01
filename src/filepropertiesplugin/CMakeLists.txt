add_definitions(-DTRANSLATION_DOMAIN=\"baloowidgets5\")

kcoreaddons_add_plugin(baloofilepropertiesplugin INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/propertiesdialog" SOURCES baloofilepropertiesplugin.cpp baloofilepropertiesplugin.h)

target_link_libraries(baloofilepropertiesplugin
    KF5::I18n
    KF5::KIOWidgets
    KF5::BalooWidgets
)
