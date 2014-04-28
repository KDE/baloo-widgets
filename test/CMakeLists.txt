add_executable(tagwidgetapp tagwidgetapp.cpp tagwidgettest.cpp)
target_link_libraries(tagwidgetapp
  KF5::KDELibs4Support
  ${BALOO_FILE_LIBRARY}
  baloowidgets
)

add_executable(metadatawidgetapp metadatawidgetapp.cpp metadatawidgettest.cpp)
target_link_libraries(metadatawidgetapp
  KF5::KDELibs4Support
  ${BALOO_FILE_LIBRARY}
  baloowidgets
)

add_executable(metadataconfigwidgetapp metadataconfigwidgetapp.cpp)
target_link_libraries(metadataconfigwidgetapp
  KF5::KDELibs4Support
  ${BALOO_FILE_LIBRARY}
  baloowidgets
)
