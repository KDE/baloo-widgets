add_executable(tagwidgetapp tagwidgetapp.cpp tagwidgettest.cpp)
target_link_libraries(tagwidgetapp
  KF5::Baloo
  KF5::BalooWidgets
  Qt5::Widgets
  Qt5::Core
)

add_executable(metadatawidgetapp metadatawidgetapp.cpp)
target_link_libraries(metadatawidgetapp
  KF5::Baloo
  KF5::BalooWidgets
  Qt5::Widgets
  Qt5::Core
)

add_executable(metadataconfigwidgetapp metadataconfigwidgetapp.cpp)
target_link_libraries(metadataconfigwidgetapp
  KF5::Baloo
  KF5::BalooWidgets
  Qt5::Widgets
  Qt5::Core
)

#add_executable(querybuilderapp querybuilderapp.cpp )
#target_link_libraries(querybuilderapp
#  KF5::BalooNaturalQueryParser
#  KF5::BalooWidgets
#  Qt5::Widgets
#  Qt5::Core
#)
