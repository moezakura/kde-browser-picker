find_package(GTest REQUIRED)

# Qt/KF auto-detect (match top-level)
set(QT_PACKAGE "")
set(KF_PACKAGE "")

# Prefer Qt5/KF5; only try Qt6/KF6 if Qt5 is unavailable
find_package(Qt5 COMPONENTS Core Widgets QUIET)
if (Qt5_FOUND)
  find_package(KF5 REQUIRED COMPONENTS Config)
  set(QT_PACKAGE Qt5)
  set(KF_PACKAGE KF5)
else()
  find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
  find_package(KF6 REQUIRED COMPONENTS Config)
  set(QT_PACKAGE Qt6)
  set(KF_PACKAGE KF6)
endif()

# Test executables
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test_browserdetector.cpp)
  add_executable(test_browserdetector test_browserdetector.cpp ../src/browserdetector.cpp)
  target_link_libraries(test_browserdetector 
      ${QT_PACKAGE}::Core 
      ${QT_PACKAGE}::Widgets 
      GTest::GTest 
      GTest::Main
  )
  add_test(NAME BrowserDetectorTest COMMAND test_browserdetector)
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test_configmanager.cpp)
  add_executable(test_configmanager test_configmanager.cpp ../src/configmanager.cpp)
  target_link_libraries(test_configmanager 
      ${QT_PACKAGE}::Core 
      ${KF_PACKAGE}::ConfigCore
      GTest::GTest 
      GTest::Main
  )
  add_test(NAME ConfigManagerTest COMMAND test_configmanager)
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test_profilemanager.cpp)
  add_executable(test_profilemanager test_profilemanager.cpp 
      ../src/profilemanager.cpp
      ../src/browserdetector.cpp
      ../src/configmanager.cpp
  )
  target_link_libraries(test_profilemanager 
      ${QT_PACKAGE}::Core 
      ${QT_PACKAGE}::Widgets
      ${KF_PACKAGE}::ConfigCore
      GTest::GTest 
      GTest::Main
  )
  add_test(NAME ProfileManagerTest COMMAND test_profilemanager)
endif()

# Security test executable
add_executable(test_browserdetector_security test_browserdetector_security.cpp ../src/browserdetector.cpp)
target_link_libraries(test_browserdetector_security 
    ${QT_PACKAGE}::Core 
    ${QT_PACKAGE}::Widgets 
    GTest::GTest 
    GTest::Main
)

# Add tests
add_test(NAME BrowserDetectorSecurityTest COMMAND test_browserdetector_security)

# YAML override test
add_executable(test_yaml_overrides 
    test_yaml_overrides.cpp
    ../src/configmanager.cpp
    ../src/browserdetector.cpp
)
target_link_libraries(test_yaml_overrides 
    ${QT_PACKAGE}::Core 
    ${QT_PACKAGE}::Widgets
    ${KF_PACKAGE}::ConfigCore
    GTest::GTest 
    GTest::Main
)
add_test(NAME YamlOverridesTest COMMAND test_yaml_overrides)
