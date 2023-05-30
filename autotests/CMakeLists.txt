include(ECMMarkAsTest)

ecm_add_test(extractortest.cpp ../src/ondemandextractor.cpp ../src/widgetsdebug.cpp
    TEST_NAME "extractortest"
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test
    KF${QT_MAJOR_VERSION}::FileMetaData
)
target_include_directories(extractortest
    PRIVATE ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
)

ecm_add_tests(
    filemetadatawidgettest.cpp
    filemetadataitemcounttest.cpp
    filemetadatadatedisplaytest.cpp
    LINK_LIBRARIES
    KF${QT_MAJOR_VERSION}::BalooWidgets
    Qt${QT_MAJOR_VERSION}::Test
)
target_link_libraries(filemetadatawidgettest
    KF${QT_MAJOR_VERSION}::WidgetsAddons
    KF${QT_MAJOR_VERSION}::ConfigCore
)
target_link_libraries(filemetadataitemcounttest
    KF${QT_MAJOR_VERSION}::ConfigCore
)

ecm_add_test(filemetadatamergedatatest.cpp ../src/filemetadatautil.cpp
    TEST_NAME "mergedatatest"
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test
    KF${QT_MAJOR_VERSION}::FileMetaData
)
target_include_directories(mergedatatest
    PRIVATE ${CMAKE_SOURCE_DIR}/src
)

# Set PATH so baloo_filemetadata_temp_extractor from the build directory is used
if(WIN32)
   set_property(TEST "extractortest"               PROPERTY ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/bin\\;$ENV{PATH}")
   set_property(TEST "filemetadatawidgettest"      PROPERTY ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/bin\\;$ENV{PATH}")
   set_property(TEST "filemetadataitemcounttest"   PROPERTY ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/bin\\;$ENV{PATH}")
   set_property(TEST "filemetadatadatedisplaytest" PROPERTY ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/bin\\;$ENV{PATH}")
else()
   set_property(TEST "extractortest"               PROPERTY ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/bin:$ENV{PATH}")
   set_property(TEST "filemetadatawidgettest"      PROPERTY ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/bin:$ENV{PATH}")
   set_property(TEST "filemetadataitemcounttest"   PROPERTY ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/bin:$ENV{PATH}")
   set_property(TEST "filemetadatadatedisplaytest" PROPERTY ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/bin:$ENV{PATH}")
endif()
