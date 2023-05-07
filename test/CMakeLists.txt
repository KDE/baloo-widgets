add_executable(tagwidgetapp tagwidgetapp.cpp tagwidgettest.cpp tagwidgettest.h)
target_link_libraries(tagwidgetapp
  KF${QT_MAJOR_VERSION}::Baloo
  KF${QT_MAJOR_VERSION}::BalooWidgets
  Qt${QT_MAJOR_VERSION}::Widgets
  Qt${QT_MAJOR_VERSION}::Core
)

add_executable(metadatawidgetapp metadatawidgetapp.cpp)
target_link_libraries(metadatawidgetapp
  KF${QT_MAJOR_VERSION}::Baloo
  KF${QT_MAJOR_VERSION}::BalooWidgets
  Qt${QT_MAJOR_VERSION}::Widgets
  Qt${QT_MAJOR_VERSION}::Core
)

