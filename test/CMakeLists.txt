add_executable(tagwidgetapp tagwidgetapp.cpp tagwidgettest.cpp tagwidgettest.h)
target_link_libraries(tagwidgetapp
  KF6::Baloo
  KF6::BalooWidgets
  Qt6::Widgets
  Qt6::Core
)

add_executable(metadatawidgetapp metadatawidgetapp.cpp)
target_link_libraries(metadatawidgetapp
  KF6::Baloo
  KF6::BalooWidgets
  Qt6::Widgets
  Qt6::Core
)

