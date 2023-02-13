add_executable(tagwidgetapp tagwidgetapp.cpp tagwidgettest.cpp tagwidgettest.h)
target_link_libraries(tagwidgetapp
  KF${KF_MAJOR_VERSION}::Baloo
  KF5::BalooWidgets
  Qt${QT_MAJOR_VERSION}::Widgets
  Qt${QT_MAJOR_VERSION}::Core
)

add_executable(metadatawidgetapp metadatawidgetapp.cpp)
target_link_libraries(metadatawidgetapp
  KF${KF_MAJOR_VERSION}::Baloo
  KF5::BalooWidgets
  Qt${QT_MAJOR_VERSION}::Widgets
  Qt${QT_MAJOR_VERSION}::Core
)

add_executable(metadataconfigwidgetapp metadataconfigwidgetapp.cpp)
target_link_libraries(metadataconfigwidgetapp
  KF${KF_MAJOR_VERSION}::Baloo
  KF5::BalooWidgets
  Qt${QT_MAJOR_VERSION}::Widgets
  Qt${QT_MAJOR_VERSION}::Core
)
