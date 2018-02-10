include(ECMMarkAsTest)

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

ecm_add_test(extractortest.cpp
    TEST_NAME "extractortest"
    LINK_LIBRARIES Qt5::Test
)
ecm_add_test(filemetadatawidgettest.cpp
    TEST_NAME "filemetadatawidgettest"
    LINK_LIBRARIES KF5::KIOCore
    KF5::KIOWidgets
    KF5::KIOFileWidgets
    KF5::BalooWidgets
    KF5::FileMetaData 
    Qt5::Test 
    Qt5::Widgets
)
