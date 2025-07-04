/**
 * @file constants.h
 * @brief KDEブラウザピッカーアプリケーションの定数定義
 * 
 * このファイルにはアプリケーション全体で使用される定数、
 * 設定値、列挙型が定義されています。
 */

#ifndef KDE_BROWSER_PICKER_CONSTANTS_H
#define KDE_BROWSER_PICKER_CONSTANTS_H

#include <QString>

namespace Constants {
    /**
     * @brief タイムアウト設定
     * ユーザーがブラウザを選択するまでの待機時間（秒）
     */
    // Timeouts
    constexpr int DEFAULT_TIMEOUT = 10;
    constexpr int MIN_TIMEOUT = 5;
    constexpr int MAX_TIMEOUT = 60;
    
    /**
     * @brief 制限値
     * アプリケーションの各種制限を定義
     */
    // Limits
    constexpr int MAX_PROFILES = 20;
    constexpr int MAX_URL_DISPLAY_LENGTH = 80;
    
    /**
     * @brief ウィンドウ設定
     * メインウィンドウのサイズと制限
     */
    // Window settings
    constexpr int DEFAULT_WINDOW_WIDTH = 600;
    constexpr int DEFAULT_WINDOW_HEIGHT = 400;
    constexpr int MIN_WINDOW_WIDTH = 400;
    constexpr int MIN_WINDOW_HEIGHT = 300;
    
    /**
     * @brief ファイルパス
     * 各ブラウザの設定ファイル名
     */
    // File paths
    constexpr auto FIREFOX_CONFIG = "profiles.ini";
    constexpr auto CHROME_CONFIG = "Local State";
    constexpr auto CHROMIUM_CONFIG = "Local State";
    
    /**
     * @brief ブラウザ実行ファイル
     * 各ブラウザの実行ファイル名
     */
    // Browser executables
    constexpr auto FIREFOX_EXECUTABLE = "firefox";
    constexpr auto CHROME_EXECUTABLE = "google-chrome";
    constexpr auto CHROMIUM_EXECUTABLE = "chromium";
    
    /**
     * @brief Chrome実行ファイルのバリエーション
     * ディストリビューションによって異なるChrome実行ファイル名のリスト
     */
    // Chrome executable variants
    inline const QStringList CHROME_EXECUTABLE_VARIANTS = {
        "google-chrome",
        "google-chrome-stable",
        "google-chrome-beta",
        "google-chrome-unstable"
    };
    
    /**
     * @brief 設定キー
     * KConfigで使用される設定グループとキー名
     */
    // Config keys
    constexpr auto CONFIG_GROUP_GENERAL = "General";
    constexpr auto CONFIG_GROUP_BROWSERS = "Browsers";
    constexpr auto CONFIG_GROUP_LAST_USED = "LastUsed";
    
    constexpr auto CONFIG_KEY_DEFAULT_TIMEOUT = "DefaultTimeout";
    constexpr auto CONFIG_KEY_REMEMBER_LAST_USED = "RememberLastUsed";
    constexpr auto CONFIG_KEY_SHOW_TRAY_ICON = "ShowTrayIcon";
    constexpr auto CONFIG_KEY_WINDOW_GEOMETRY = "WindowGeometry";
    
    /**
     * @brief ブラウザタイプ
     * サポートされているブラウザの種類
     */
    // Browser types
    enum class BrowserType {
        Firefox,
        Chrome,
        Chromium,
        Unknown
    };
    
    /**
     * @brief プロファイルステータス
     * ブラウザプロファイルの状態を表す
     */
    // Profile status
    enum class ProfileStatus {
        Enabled,
        Disabled,
        NotFound
    };
}

#endif // KDE_BROWSER_PICKER_CONSTANTS_H