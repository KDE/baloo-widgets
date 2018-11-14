include_directories(${CMAKE_SOURCE_DIR})

set(tagsfileitemaction_debug_SRCS tagsfileitemaction.cpp)

kcoreaddons_add_plugin(tagsfileitemaction
    SOURCES ${tagsfileitemaction_debug_SRCS}
    JSON tagsfileitemaction.json
    INSTALL_NAMESPACE "kf5/kfileitemaction")

target_link_libraries(tagsfileitemaction
    KF5::KIOWidgets
    KF5::I18n
    KF5::FileMetaData
)
