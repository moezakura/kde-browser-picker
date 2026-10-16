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
#include <memory>

// Forward declarations
namespace Ui {
    class SettingsDialog;
}
class ConfigManager;
class ProfileManager;

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
 * - プロファイルの並び順と表示/非表示
 */
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param configManager 設定管理オブジェクト（非所有）
     * @param profileManager プロファイル管理オブジェクト（非所有、nullptr可。nullptrの場合プロファイルタブは空のまま）
     * @param parent 親ウィジェット
     */
    explicit SettingsDialog(ConfigManager* configManager, ProfileManager* profileManager, QWidget* parent = nullptr);
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

    /**
     * @brief 表示ONの件数に応じてOKボタンと警告ラベルを更新
     */
    void updateOkButtonState();

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

    /**
     * @brief プロファイル一覧へ全件（無効を含む）を現在の並びで投入
     */
    void populateProfileList();

    /**
     * @brief プロファイル一覧の並びと表示状態をProfileManagerへ保存
     */
    void saveProfilesTab();
    
    std::unique_ptr<Ui::SettingsDialog> m_ui;    ///< UIフォーム
    ConfigManager* m_configManager;               ///< 設定管理オブジェクト（非所有）
    ProfileManager* m_profileManager;             ///< プロファイル管理オブジェクト（非所有、nullptr可）
};

#endif // SETTINGSDIALOG_H