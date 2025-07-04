/**
 * @file kdeintegration.h
 * @brief KDEデスクトップ環境との統合機能
 *
 * このファイルは、KDEデスクトップ環境固有の機能を提供します。
 * システムトレイ、通知、デフォルトブラウザ登録、
 * グローバルショートカットなどの機能を含みます。
 */

#ifndef KDEINTEGRATION_H
#define KDEINTEGRATION_H

#include <QObject>
#include <QString>
#include <QSystemTrayIcon>

// Forward declarations
class QMenu;
class QAction;
class KNotification;

/**
 * @class KDEIntegration
 * @brief KDEデスクトップ統合クラス
 *
 * KDE Plasmaデスクトップ環境との統合機能を提供します。
 * システムトレイアイコン、KDE通知システム、
 * デフォルトアプリケーションの登録などをサポートします。
 *
 * @note KDE以外の環境でも一部の機能は動作します
 */
class KDEIntegration : public QObject {
    Q_OBJECT

public:
    explicit KDEIntegration(QObject* parent = nullptr);
    ~KDEIntegration() override;

    // システムトレイ
    /**
     * @brief システムトレイアイコンの表示/非表示を設定
     * @param visible true: 表示, false: 非表示
     */
    void setTrayIconVisible(bool visible);
    
    /**
     * @brief システムトレイアイコンが表示されているか確認
     * @return true: 表示中, false: 非表示
     */
    bool isTrayIconVisible() const;
    
    /**
     * @brief トレイメニューを更新
     */
    void updateTrayMenu();
    
    // 通知
    /**
     * @brief KDE通知を表示
     * @param title 通知のタイトル
     * @param text 通知の本文
     * @param iconName アイコン名（オプション）
     */
    void showNotification(const QString& title, 
                         const QString& text,
                         const QString& iconName = QString());
                         
    // デスクトップファイル登録
    /**
     * @brief デフォルトブラウザとして登録
     * @return true: 登録成功, false: 登録失敗
     */
    static bool registerAsDefaultBrowser();
    
    /**
     * @brief デフォルトブラウザとして登録されているか確認
     * @return true: 登録済み, false: 未登録
     */
    static bool isRegisteredAsDefaultBrowser();
    
    // KDEグローバルショートカット
    /**
     * @brief グローバルショートカットを登録
     */
    void registerGlobalShortcuts();
    
    /**
     * @brief グローバルショートカットを解除
     */
    void unregisterGlobalShortcuts();
    
signals:
    /**
     * @brief トレイアイコンがアクティベートされたときに発行されるシグナル
     */
    void trayActivated();
    
    /**
     * @brief 設定ダイアログの開く要求時に発行されるシグナル
     */
    void openSettingsRequested();
    
    /**
     * @brief アプリケーション終了要求時に発行されるシグナル
     */
    void quitRequested();

private slots:
    /**
     * @brief トレイアイコンがアクティベートされたときの処理
     * @param reason アクティベーションの理由
     */
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    
    /**
     * @brief トレイメニュー項目が選択されたときの処理
     * @param action 選択されたアクション
     */
    void onTrayMenuTriggered(QAction* action);
    
    /**
     * @brief 通知がアクティベートされたときの処理
     */
    void onNotificationActivated();

private:
    /**
     * @brief トレイアイコンを作成
     */
    void createTrayIcon();
    
    /**
     * @brief トレイメニューを作成
     */
    void createTrayMenu();
    
    QSystemTrayIcon* m_trayIcon;                       ///< システムトレイアイコン
    QMenu* m_trayMenu;                                 ///< トレイメニュー
    
    // アクティブな通知の追跡
    QList<KNotification*> m_activeNotifications;       ///< 現在表示中の通知リスト
};

#endif // KDEINTEGRATION_H