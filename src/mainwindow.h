/**
 * @file mainwindow.h
 * @brief ブラウザ選択のメインウィンドウクラス
 *
 * このファイルは、KDE Browser Pickerのメインウィンドウを定義します。
 * ユーザーがブラウザとプロファイルを選択するためのUIを提供し、
 * キーボードショートカット、タイムアウト機能、検索機能などをサポートします。
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QTimer>
#include <memory>

// Forward declarations
namespace Ui {
    class MainWindow;
}
class ProfileManager;
class ConfigManager;
class ProfileItem;

/**
 * @class MainWindow
 * @brief ブラウザ選択ダイアログのメインクラス
 *
 * MainWindowは、ユーザーがURLを開くためのブラウザとプロファイルを
 * 選択するためのダイアログを提供します。
 *
 * 主な機能:
 * - 利用可能なブラウザプロファイルの一覧表示
 * - 数字キー（1-9）によるクイック選択
 * - プロファイル名の検索機能
 * - 自動タイムアウト機能
 * - ウィンドウ位置とサイズの記憶
 */
class MainWindow : public QDialog {
    Q_OBJECT

public:
    explicit MainWindow(const QString& url = "", QWidget* parent = nullptr);
    ~MainWindow() override;

    // コピーコンストラクタと代入演算子を削除
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;

protected:
    /**
     * @brief キープレスイベントの処理
     * @param event キーイベント
     * @note 数字キー(1-9)でプロファイルを選択、Enterで決定、Escでキャンセル
     */
    void keyPressEvent(QKeyEvent* event) override;
    
    /**
     * @brief ウィンドウクローズイベントの処理
     * @param event クローズイベント
     * @note ウィンドウの位置を保存
     */
    void closeEvent(QCloseEvent* event) override;
    
    /**
     * @brief ウィンドウ表示イベントの処理
     * @param event 表示イベント
     * @note タイムアウトタイマーの開始
     */
    void showEvent(QShowEvent* event) override;

private slots:
    /**
     * @brief プロファイルアイテムがクリックされたときの処理
     */
    void onProfileClicked();
    
    /**
     * @brief プロファイルアイテムがダブルクリックされたときの処理
     * @note ダブルクリックで即座にブラウザを起動
     */
    void onProfileDoubleClicked();
    
    /**
     * @brief プロファイル設定ボタンがクリックされたときの処理
     */
    void onProfileSettingsClicked();
    
    /**
     * @brief 設定ボタンがクリックされたときの処理
     */
    void onSettingsClicked();
    
    /**
     * @brief 開くボタンがクリックされたときの処理
     */
    void onOpenClicked();
    
    /**
     * @brief キャンセルボタンがクリックされたときの処理
     */
    void onCancelClicked();
    
    /**
     * @brief タイムアウト時の処理
     * @note デフォルトプロファイルでブラウザを起動
     */
    void onTimeout();
    
    /**
     * @brief タイムアウトカウントダウンの更新処理
     */
    void onTimeoutTick();
    
    /**
     * @brief プロファイル一覧が更新されたときの処理
     */
    void onProfilesRefreshed();
    
    /**
     * @brief 設定が変更されたときの処理
     */
    void onConfigChanged();
    
    /**
     * @brief 検索テキストが変更されたときの処理
     * @param text 検索テキスト
     */
    void onSearchTextChanged(const QString& text);

private:
    /**
     * @brief UIの初期化と設定
     */
    void setupUI();
    
    /**
     * @brief キーボードショートカットの設定
     */
    void setupShortcuts();
    
    /**
     * @brief プロファイル一覧の読み込みと表示
     */
    void loadProfiles();
    
    /**
     * @brief 数字キーによるプロファイル選択
     * @param number 選択番号（1-9）
     */
    void selectProfileByNumber(int number);
    
    /**
     * @brief プロファイルアイテムの選択
     * @param item 選択するプロファイルアイテム
     */
    void selectProfile(ProfileItem* item);
    
    /**
     * @brief 選択されたプロファイルでブラウザを起動
     */
    void openSelectedProfile();
    
    /**
     * @brief タイムアウトラベルの更新
     */
    void updateTimeoutLabel();
    
    /**
     * @brief ウィンドウの位置とサイズを保存
     */
    void saveWindowGeometry();
    
    /**
     * @brief ウィンドウの位置とサイズを復元
     */
    void restoreWindowGeometry();
    
    /**
     * @brief URLを表示用に短縮
     * @param url 元のURL
     * @return 短縮されたURL
     */
    QString truncateUrl(const QString& url) const;
    
    std::unique_ptr<Ui::MainWindow> m_ui;                ///< UIフォーム
    std::unique_ptr<ProfileManager> m_profileManager;    ///< プロファイル管理オブジェクト
    std::unique_ptr<ConfigManager> m_configManager;      ///< 設定管理オブジェクト
    
    QString m_url;                                       ///< 開くURL
    QTimer* m_timeoutTimer;                              ///< タイムアウトタイマー
    QTimer* m_tickTimer;                                 ///< カウントダウン更新タイマー
    int m_remainingSeconds;                              ///< 残り秒数
    
    QList<ProfileItem*> m_profileItems;                  ///< プロファイルアイテムのリスト
    ProfileItem* m_selectedItem;                         ///< 現在選択されているアイテム
};

#endif // MAINWINDOW_H