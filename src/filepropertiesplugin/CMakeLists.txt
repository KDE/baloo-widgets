set(baloofilepropertiesplugin_PART_SRCS baloofilepropertiesplugin.cpp)

add_library(baloofilepropertiesplugin MODULE ${baloofilepropertiesplugin_PART_SRCS})

target_link_libraries(baloofilepropertiesplugin
    KF5::I18n
    KF5::KIOWidgets
    KF5::BalooWidgets
)

install(TARGETS baloofilepropertiesplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES baloofilepropertiesplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
