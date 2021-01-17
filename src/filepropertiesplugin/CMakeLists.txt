add_definitions(-DTRANSLATION_DOMAIN=\"baloowidgets5\")

set(baloofilepropertiesplugin_PART_SRCS baloofilepropertiesplugin.cpp)

add_library(baloofilepropertiesplugin MODULE ${baloofilepropertiesplugin_PART_SRCS})

target_link_libraries(baloofilepropertiesplugin
    KF5::I18n
    KF5::KIOWidgets
    KF5::BalooWidgets
)

install(TARGETS baloofilepropertiesplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES baloofilepropertiesplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
