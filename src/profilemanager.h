/**
 * @file profilemanager.h
 * @brief ブラウザプロファイルの管理クラス
 *
 * このファイルは、検出されたブラウザプロファイルの管理機能を提供します。
 * プロファイルの有効/無効、表示名のカスタマイズ、並び順の管理、
 * およびブラウザの起動機能を含みます。
 */

#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <memory>

#include "browserdetector.h"

// Forward declarations
class ConfigManager;

/**
 * @class ProfileManager
 * @brief ブラウザプロファイル管理クラス
 *
 * ProfileManagerは、BrowserDetectorとConfigManagerを連携させ、
 * ブラウザプロファイルの統合的な管理を行います。
 *
 * 主な機能:
 * - ブラウザプロファイルの検出と更新
 * - プロファイルの有効/無効状態の管理
 * - カスタム表示名と表示順序の設定
 * - 最後に使用したプロファイルの追跡
 * - 指定されたプロファイルでのブラウザ起動
 */
class ProfileManager : public QObject {
    Q_OBJECT

public:
    /**
     * @struct ProfileEntry
     * @brief プロファイルエントリ情報
     *
     * 各ブラウザプロファイルの完全な情報を格納します。
     * 表示用の情報と設定情報の両方を含みます。
     */
    struct ProfileEntry {
        QString browser;              ///< ブラウザID（"firefox", "chrome"など）
        QString browserDisplayName;   ///< ブラウザの表示名
        QString profileId;            ///< プロファイルID
        QString profileDisplayName;   ///< プロファイルの表示名（カスタマイズ可能）
        QString iconPath;             ///< アイコンファイルのパス
        QDateTime lastUsed;           ///< 最終使用日時
        bool isEnabled;               ///< 有効/無効状態
        bool isDefault;               ///< デフォルトプロファイルかどうか
        int order;                    ///< 表示順序
        
        ProfileEntry() 
            : isEnabled(true)
            , isDefault(false)
            , order(999) {}
            
        /**
         * @brief ソート用の比較演算子
         * @note 表示順序、ブラウザ名、プロファイル名の順でソート
         */
        bool operator<(const ProfileEntry& other) const {
            // まず表示順序、次にブラウザ、最後にプロファイル名でソート
            if (order != other.order) {
                return order < other.order;
            }
            if (browser != other.browser) {
                return browser < other.browser;
            }
            return profileDisplayName < other.profileDisplayName;
        }
    };

    explicit ProfileManager(ConfigManager* configManager, QObject* parent = nullptr);
    ~ProfileManager() override = default;

    // コピーコンストラクタと代入演算子を削除
    ProfileManager(const ProfileManager&) = delete;
    ProfileManager& operator=(const ProfileManager&) = delete;

    // ブラウザからプロファイルを更新
    /**
     * @brief システムからプロファイル情報を再検出
     */
    void refreshProfiles();
    
    // プロファイルの取得
    /**
     * @brief 全てのプロファイルを取得
     * @param enabledOnly true: 有効なプロファイルのみ, false: 全て
     * @return プロファイルエントリのリスト
     */
    QList<ProfileEntry> getAllProfiles(bool enabledOnly = false) const;
    
    /**
     * @brief 特定のブラウザのプロファイルを取得
     * @param browser ブラウザID
     * @param enabledOnly true: 有効なプロファイルのみ, false: 全て
     * @return プロファイルエントリのリスト
     */
    QList<ProfileEntry> getProfilesForBrowser(const QString& browser, bool enabledOnly = false) const;
    
    /**
     * @brief 特定のプロファイルを取得
     * @param browser ブラウザID
     * @param profileId プロファイルID
     * @return プロファイルエントリ
     */
    ProfileEntry getProfile(const QString& browser, const QString& profileId) const;
    
    /**
     * @brief プロファイルが存在するか確認
     * @param browser ブラウザID
     * @param profileId プロファイルID
     * @return true: 存在する, false: 存在しない
     */
    bool hasProfile(const QString& browser, const QString& profileId) const;
    
    /**
     * @brief デフォルトプロファイルを取得
     * @return 最後に使用されたまたは最初の有効なプロファイル
     */
    ProfileEntry getDefaultProfile() const;
    
    // プロファイルの起動
    /**
     * @brief プロファイルエントリを使用してブラウザを起動
     * @param profile プロファイルエントリ
     * @param url 開くURL
     * @return true: 起動成功, false: 起動失敗
     */
    bool launchProfile(const ProfileEntry& profile, const QString& url);
    
    /**
     * @brief IDを使用してブラウザを起動
     * @param browser ブラウザID
     * @param profileId プロファイルID
     * @param url 開くURL
     * @return true: 起動成功, false: 起動失敗
     */
    bool launchProfile(const QString& browser, const QString& profileId, const QString& url);
    
    // プロファイル設定の更新
    /**
     * @brief プロファイルの有効/無効を設定
     * @param browser ブラウザID
     * @param profileId プロファイルID
     * @param enabled true: 有効, false: 無効
     */
    void setProfileEnabled(const QString& browser, const QString& profileId, bool enabled);
    
    /**
     * @brief プロファイルの表示名を設定
     * @param browser ブラウザID
     * @param profileId プロファイルID
     * @param name 新しい表示名
     */
    void setProfileDisplayName(const QString& browser, const QString& profileId, const QString& name);
    
    /**
     * @brief プロファイルの表示順序を設定
     * @param browser ブラウザID
     * @param profileId プロファイルID
     * @param order 新しい表示順序
     */
    void setProfileOrder(const QString& browser, const QString& profileId, int order);
    
    // プロファイルの並び替え
    /**
     * @brief プロファイルを一つ上に移動
     * @param browser ブラウザID
     * @param profileId プロファイルID
     */
    void moveProfileUp(const QString& browser, const QString& profileId);
    
    /**
     * @brief プロファイルを一つ下に移動
     * @param browser ブラウザID
     * @param profileId プロファイルID
     */
    void moveProfileDown(const QString& browser, const QString& profileId);
    
    /**
     * @brief 高度な操作のためのBrowserDetectorへのアクセス
     * @return BrowserDetectorオブジェクトへのポインタ
     */
    BrowserDetector* browserDetector() const { return m_browserDetector.get(); }

signals:
    /**
     * @brief プロファイルが更新されたときに発行されるシグナル
     */
    void profilesRefreshed();
    
    /**
     * @brief プロファイルが起動されたときに発行されるシグナル
     * @param browser ブラウザID
     * @param profile プロファイルID
     */
    void profileLaunched(const QString& browser, const QString& profile);
    
    /**
     * @brief プロファイルの起動に失敗したときに発行されるシグナル
     * @param error エラーメッセージ
     */
    void profileLaunchFailed(const QString& error);
    
    /**
     * @brief プロファイル設定が変更されたときに発行されるシグナル
     * @param browser ブラウザID
     * @param profile プロファイルID
     */
    void profileSettingsChanged(const QString& browser, const QString& profile);

private slots:
    /**
     * @brief ブラウザが検出されたときの処理
     * @param browserName 検出されたブラウザ名
     */
    void onBrowserDetected(const QString& browserName);
    
    /**
     * @brief 起動エラーが発生したときの処理
     * @param error エラーメッセージ
     */
    void onLaunchError(const QString& error);

private:
    /**
     * @brief 設定からプロファイル情報を更新
     * @param entry 更新するプロファイルエントリ
     */
    void updateProfileFromConfig(ProfileEntry& entry);
    
    /**
     * @brief プロファイルリストをソート
     * @param profiles ソートするプロファイルリスト
     */
    void sortProfiles(QList<ProfileEntry>& profiles) const;
    
    std::unique_ptr<BrowserDetector> m_browserDetector;  ///< ブラウザ検出オブジェクト
    ConfigManager* m_configManager;                      ///< 設定管理オブジェクト（非所有）
    
    QList<ProfileEntry> m_profiles;                      ///< プロファイルエントリのリスト
    mutable QDateTime m_lastRefresh;                     ///< 最後に更新した日時
};

#endif // PROFILEMANAGER_H