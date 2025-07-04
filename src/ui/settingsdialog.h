/**
 * @file settingsdialog.h
 * @brief アプリケーション設定ダイアログ
 *
 * このファイルは、KDE Browser Pickerの設定ダイアログを定義します。
 * タイムアウト設定、デフォルトブラウザ登録、その他の
 * アプリケーション設定を管理します。
 */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

// Forward declarations
namespace Ui {
    class SettingsDialog;
}
class ConfigManager;

/**
 * @class SettingsDialog
 * @brief 設定ダイアログクラス
 *
 * ユーザーがアプリケーションの各種設定を変更できる
 * ダイアログを提供します。
 *
 * 設定項目:
 * - デフォルトタイムアウト
 * - 最後に使用したブラウザを記憶
 * - システムトレイアイコンの表示
 * - デフォルトブラウザとしての登録
 */
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(ConfigManager* configManager, QWidget* parent = nullptr);
    ~SettingsDialog() override;

private slots:
    /**
     * @brief OKボタンがクリックされたときの処理
     */
    void onAccepted();
    
    /**
     * @brief キャンセルボタンがクリックされたときの処理
     */
    void onRejected();
    
    /**
     * @brief デフォルトタイムアウト値が変更されたときの処理
     * @param value 新しいタイムアウト値（秒）
     */
    void onDefaultTimeoutChanged(int value);
    
    /**
     * @brief デフォルトブラウザとして登録ボタンがクリックされたときの処理
     */
    void onRegisterAsDefaultClicked();

private:
    /**
     * @brief 設定を読み込んでUIに反映
     */
    void loadSettings();
    
    /**
     * @brief UIの値を設定に保存
     */
    void saveSettings();
    
    /**
     * @brief デフォルトブラウザの登録状態を更新
     */
    void updateDefaultBrowserStatus();
    
    std::unique_ptr<Ui::SettingsDialog> m_ui;    ///< UIフォーム
    ConfigManager* m_configManager;               ///< 設定管理オブジェクト（非所有）
};

#endif // SETTINGSDIALOG_H