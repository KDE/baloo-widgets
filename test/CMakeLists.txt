add_executable(tagwidgetapp tagwidgetapp.cpp tagwidgettest.cpp tagwidgettest.h)
target_link_libraries(tagwidgetapp
  KF5::Baloo
  KF5::BalooWidgets
  Qt${QT_MAJOR_VERSION}::Widgets
  Qt${QT_MAJOR_VERSION}::Core
)

add_executable(metadatawidgetapp metadatawidgetapp.cpp)
target_link_libraries(metadatawidgetapp
  KF5::Baloo
  KF5::BalooWidgets
  Qt${QT_MAJOR_VERSION}::Widgets
  Qt${QT_MAJOR_VERSION}::Core
)

add_executable(metadataconfigwidgetapp metadataconfigwidgetapp.cpp)
target_link_libraries(metadataconfigwidgetapp
  KF5::Baloo
  KF5::BalooWidgets
  Qt${QT_MAJOR_VERSION}::Widgets
  Qt${QT_MAJOR_VERSION}::Core
)
