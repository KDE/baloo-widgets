add_definitions(-DTRANSLATION_DOMAIN=\"baloowidgets5\")

kcoreaddons_add_plugin(baloofilepropertiesplugin INSTALL_NAMESPACE "kf5/propertiesdialog" SOURCES baloofilepropertiesplugin.cpp)

target_link_libraries(baloofilepropertiesplugin
    KF5::I18n
    KF5::KIOWidgets
    KF5::BalooWidgets
)
