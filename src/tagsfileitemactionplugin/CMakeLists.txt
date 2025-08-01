add_definitions(-DTRANSLATION_DOMAIN=\"baloowidgets5\")

kcoreaddons_add_plugin(tagsfileitemaction
    SOURCES tagsfileitemaction.cpp tagsfileitemaction.h
    INSTALL_NAMESPACE "kf6/kfileitemaction")

target_link_libraries(tagsfileitemaction
    KF6::KIOWidgets
    KF6::I18n
    KF6::FileMetaData
    KF6::BalooWidgets
)
