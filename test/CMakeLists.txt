add_executable(tagwidgetapp tagwidgetapp.cpp tagwidgettest.cpp)
target_link_libraries(tagwidgetapp
  KF5::KDE4Support
  ${BALOO_LIBRARIES}
  baloowidgets
)

add_executable(metadatawidgetapp metadatawidgetapp.cpp metadatawidgettest.cpp)
target_link_libraries(metadatawidgetapp
  KF5::KDE4Support
  ${BALOO_LIBRARIES}
  baloowidgets
)

add_executable(metadataconfigwidgetapp metadataconfigwidgetapp.cpp)
target_link_libraries(metadataconfigwidgetapp
  KF5::KDE4Support
  ${BALOO_LIBRARIES}
  baloowidgets
)
