find_package(GTest REQUIRED)

# Test executables
add_executable(test_browserdetector test_browserdetector.cpp ../src/browserdetector.cpp)
target_link_libraries(test_browserdetector 
    Qt5::Core 
    Qt5::Widgets 
    GTest::GTest 
    GTest::Main
)

add_executable(test_configmanager test_configmanager.cpp ../src/configmanager.cpp)
target_link_libraries(test_configmanager 
    Qt5::Core 
    KF5::ConfigCore
    GTest::GTest 
    GTest::Main
)

add_executable(test_profilemanager test_profilemanager.cpp 
    ../src/profilemanager.cpp
    ../src/browserdetector.cpp
    ../src/configmanager.cpp
)
target_link_libraries(test_profilemanager 
    Qt5::Core 
    Qt5::Widgets
    KF5::ConfigCore
    GTest::GTest 
    GTest::Main
)

# Security test executable
add_executable(test_browserdetector_security test_browserdetector_security.cpp ../src/browserdetector.cpp)
target_link_libraries(test_browserdetector_security 
    Qt5::Core 
    Qt5::Widgets 
    GTest::GTest 
    GTest::Main
)

# Add tests
add_test(NAME BrowserDetectorTest COMMAND test_browserdetector)
add_test(NAME ConfigManagerTest COMMAND test_configmanager)
add_test(NAME ProfileManagerTest COMMAND test_profilemanager)
add_test(NAME BrowserDetectorSecurityTest COMMAND test_browserdetector_security)