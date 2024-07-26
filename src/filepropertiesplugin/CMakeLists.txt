add_definitions(-DTRANSLATION_DOMAIN=\"baloowidgets5\")

kcoreaddons_add_plugin(baloofilepropertiesplugin INSTALL_NAMESPACE "kf6/propertiesdialog" SOURCES baloofilepropertiesplugin.cpp baloofilepropertiesplugin.h)

target_link_libraries(baloofilepropertiesplugin
    KF6::I18n
    KF6::KIOWidgets
    KF6::BalooWidgets
)
