project(nepomukwidgets-tests)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/..
)

add_definitions(-DBUILDING_NEPOMUK_TESTS)

# Ui tests
# --------------------------------------------
kde4_add_executable(tagwidgetapp TEST tagwidgetapp.cpp tagwidgettest.cpp)
target_link_libraries(tagwidgetapp
  ${KDE4_KIO_LIBS}
  ${NEPOMUK_CORE_LIBRARY}
  nepomukwidgets
  )

kde4_add_executable(metadatawidgetapp TEST metadatawidgetapp.cpp metadatawidgettest.cpp)
target_link_libraries(metadatawidgetapp
  ${KDE4_KIO_LIBS}
  ${NEPOMUK_CORE_LIBRARY}
  nepomukwidgets
  )

kde4_add_executable(metadataconfigwidgetapp TEST metadataconfigwidgetapp.cpp )
target_link_libraries(metadataconfigwidgetapp
  ${KDE4_KIO_LIBS}
  ${NEPOMUK_CORE_LIBRARY}
  nepomukwidgets
  )

