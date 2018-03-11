include(ECMMarkAsTest)

ecm_add_test(extractortest.cpp
    TEST_NAME "extractortest"
    LINK_LIBRARIES Qt5::Test
)
ecm_add_tests(
    filemetadatawidgettest.cpp
    filemetadataitemcounttest.cpp
    LINK_LIBRARIES KF5::KIOCore
    KF5::KIOWidgets
    KF5::KIOFileWidgets
    KF5::ConfigCore
    KF5::BalooWidgets
    KF5::FileMetaData 
    Qt5::Test 
    Qt5::Widgets
)
