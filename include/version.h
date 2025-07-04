/**
 * @file version.h
 * @brief KDEブラウザピッカーのバージョン情報
 * 
 * アプリケーションのバージョン番号を定義します。
 * セマンティックバージョニングに従い、
 * MAJOR.重要な変更、MINOR.機能追加、PATCH.バグ修正
 * を表します。
 */

#ifndef KDE_BROWSER_PICKER_VERSION_H
#define KDE_BROWSER_PICKER_VERSION_H

/** @brief メジャーバージョン番号（互換性のない変更） */
#define KDE_BROWSER_PICKER_VERSION_MAJOR 1

/** @brief マイナーバージョン番号（新機能追加） */
#define KDE_BROWSER_PICKER_VERSION_MINOR 0

/** @brief パッチバージョン番号（バグ修正） */
#define KDE_BROWSER_PICKER_VERSION_PATCH 0

/** @brief 完全なバージョン文字列 */
#define KDE_BROWSER_PICKER_VERSION_STRING "1.0.0"

#endif // KDE_BROWSER_PICKER_VERSION_H