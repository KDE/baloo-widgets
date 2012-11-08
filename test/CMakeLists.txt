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

# Facet tests
# -----------------------------------
kde4_add_unit_test(simplefacettest TESTNAME nepomuk-simplefacettest simplefacettest.cpp)
target_link_libraries(simplefacettest
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
  ${NEPOMUK_CORE_LIBRARY}
  nepomukwidgets
)

kde4_add_unit_test(dynamicresourcefacettest TESTNAME nepomuk-dynamicresourcefacettest dynamicresourcefacettest.cpp)
target_link_libraries(dynamicresourcefacettest
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
  ${NEPOMUK_CORE_LIBRARY}
  nepomukwidgets
)

kde4_add_unit_test(proxyfacettest TESTNAME nepomuk-proxyfacettest proxyfacettest.cpp)
target_link_libraries(proxyfacettest
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
  ${NEPOMUK_CORE_LIBRARY}
  nepomukwidgets
)

kde4_add_unit_test(facetmodeltest TESTNAME nepomuk-facetmodeltest facetmodeltest.cpp)
target_link_libraries(facetmodeltest
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
  ${NEPOMUK_CORE_LIBRARY}
  nepomukwidgets
)

kde4_add_unit_test(typefacettest TESTNAME nepomuk-typefacettest typefacettest.cpp)
target_link_libraries(typefacettest
  ${QT_QTTEST_LIBRARY}
  ${SOPRANO_LIBRARIES}
  ${KDE4_KDECORE_LIBS}
  ${NEPOMUK_CORE_LIBRARY}
  nepomukwidgets
)

