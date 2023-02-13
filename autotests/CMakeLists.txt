include(ECMMarkAsTest)

ecm_add_test(extractortest.cpp
    TEST_NAME "extractortest"
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test
    KF${KF_MAJOR_VERSION}::FileMetaData
)
ecm_add_tests(
    filemetadatawidgettest.cpp
    filemetadataitemcounttest.cpp
    filemetadatadatedisplaytest.cpp
    LINK_LIBRARIES KF${KF_MAJOR_VERSION}::KIOCore
    KF${KF_MAJOR_VERSION}::KIOWidgets
    KF${KF_MAJOR_VERSION}::KIOFileWidgets
    KF${KF_MAJOR_VERSION}::ConfigCore
    KF5::BalooWidgets
    KF${KF_MAJOR_VERSION}::FileMetaData
    Qt${QT_MAJOR_VERSION}::Test
    Qt${QT_MAJOR_VERSION}::Widgets
)

target_include_directories(extractortest
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
