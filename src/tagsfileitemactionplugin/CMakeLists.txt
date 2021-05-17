add_definitions(-DTRANSLATION_DOMAIN=\"baloowidgets5\")

kcoreaddons_add_plugin(tagsfileitemaction
    SOURCES tagsfileitemaction.cpp
    INSTALL_NAMESPACE "kf5/kfileitemaction")

target_link_libraries(tagsfileitemaction
    KF5::KIOWidgets
    KF5::I18n
    KF5::FileMetaData
)
