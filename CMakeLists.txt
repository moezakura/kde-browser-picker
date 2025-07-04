# KDEブラウザピッカー
# 複数のブラウザから選択してURLを開くKDEアプリケーション

cmake_minimum_required(VERSION 3.16)
project(kde-browser-picker VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ビルドオプション
# Options
option(ENABLE_CLANG_TIDY "Enable clang-tidy" ON)          # 静的コード解析ツール
option(ENABLE_CPPCHECK "Enable cppcheck" ON)              # C++静的チェックツール
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)         # メモリエラー検出ツール
option(BUILD_TESTS "Build unit tests" ON)                 # ユニットテストのビルド

# KDEのCMakeモジュールを検索
# Find ECM first
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

# Include KDE CMake settings
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

# 依存パッケージの検索
# Find Qt5 packages
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)

# Find KF5 packages
find_package(KF5 REQUIRED COMPONENTS Config ConfigWidgets Notifications I18n)

# Enable automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set AUTOUIC search paths
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/src/ui)

# インクルードディレクトリの設定
# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# ソースファイルの定義
# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/browserdetector.cpp
    src/profilemanager.cpp
    src/configmanager.cpp
    src/kdeintegration.cpp
    src/ui/profileitem.cpp
    src/ui/settingsdialog.cpp
)

# ヘッダーファイルの定義
# Header files
set(HEADERS
    src/mainwindow.h
    src/browserdetector.h
    src/profilemanager.h
    src/configmanager.h
    src/kdeintegration.h
    src/ui/profileitem.h
    src/ui/settingsdialog.h
    include/version.h
    include/constants.h
)

# UIファイルの定義（Qt Designerファイル）
# UI files
set(UI_FILES
    src/ui/mainwindow.ui
    src/ui/settingsdialog.ui
)

# リソースファイルの定義（アイコン等）
# Resource files
set(RESOURCE_FILES
    resources/kde-browser-picker.qrc
)

# 実行ファイルの作成
# Create executable
add_executable(kde-browser-picker
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCE_FILES}
)

# ライブラリのリンク
# Link libraries
target_link_libraries(kde-browser-picker
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::Notifications
    KF5::I18n
)

# コンパイラオプションの設定
# Compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(kde-browser-picker PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion
        -Wno-unused-parameter
        -Wno-deprecated-enum-enum-conversion
        -fno-rtti
        -fno-exceptions
    )
endif()

# AddressSanitizer（メモリエラー検出）
# AddressSanitizer
if(ENABLE_ASAN)
    target_compile_options(kde-browser-picker PRIVATE -fsanitize=address)
    target_link_options(kde-browser-picker PRIVATE -fsanitize=address)
endif()

# clang-tidy（静的コード解析）
# clang-tidy
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set_target_properties(kde-browser-picker PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=-*,readability-*,performance-*,bugprone-*,modernize-*;-header-filter=.*"
        )
    endif()
endif()

# cppcheck（C++静的解析）
# cppcheck
if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXE NAMES "cppcheck")
    if(CPPCHECK_EXE)
        set(CMAKE_CXX_CPPCHECK "${CPPCHECK_EXE}"
            "--enable=warning,performance,portability"
            "--inline-suppr"
            "--suppress=missingIncludeSystem"
        )
    endif()
endif()

# インストール設定
# Installation
install(TARGETS kde-browser-picker DESTINATION bin)
install(FILES resources/browser-picker.desktop DESTINATION share/applications)
install(DIRECTORY resources/icons/ DESTINATION share/icons/hicolor)

# テストの設定
# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()