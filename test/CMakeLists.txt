add_executable(tagwidgetapp tagwidgetapp.cpp tagwidgettest.cpp)
target_link_libraries(tagwidgetapp
  KF5::BalooFiles
  KF5BalooWidgets
)

add_executable(metadatawidgetapp metadatawidgetapp.cpp metadatawidgettest.cpp)
target_link_libraries(metadatawidgetapp
  KF5::BalooFiles
  KF5BalooWidgets
)

add_executable(metadataconfigwidgetapp metadataconfigwidgetapp.cpp)
target_link_libraries(metadataconfigwidgetapp
  KF5::BalooFiles
  KF5BalooWidgets
)
