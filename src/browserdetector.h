/**
 * @file browserdetector.h
 * @brief システムにインストールされたブラウザとプロファイルを検出するクラス
 *
 * このファイルは、Firefox、Chrome、Chromiumなどのブラウザを検出し、
 * それぞれのプロファイル情報を取得する機能を提供します。
 * また、セキュリティのためのURLとプロファイル名の検証機能も含まれています。
 */

#ifndef BROWSERDETECTOR_H
#define BROWSERDETECTOR_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QDateTime>
#include <memory>

#include "constants.h"

/**
 * @class BrowserDetector
 * @brief ブラウザ検出と起動のメインクラス
 *
 * このクラスはシステムにインストールされたブラウザを検出し、
 * そのプロファイル情報を取得し、指定されたプロファイルでブラウザを起動する機能を提供します。
 *
 * サポートされるブラウザ:
 * - Mozilla Firefox
 * - Google Chrome
 * - Chromium
 *
 * @note セキュリティ上の理由から、URLとプロファイル名は検証・サニタイズされます
 */
class BrowserDetector : public QObject {
    Q_OBJECT

public:
    /**
     * @struct ProfileInfo
     * @brief ブラウザプロファイルの情報を格納する構造体
     */
    struct ProfileInfo {
        QString name;          ///< プロファイル名（内部ID）
        QString path;          ///< プロファイルのファイルシステムパス
        QString displayName;   ///< 表示用のプロファイル名
        QDateTime lastUsed;    ///< 最後に使用された日時
        bool isDefault;        ///< デフォルトプロファイルかどうか
        
        ProfileInfo() : isDefault(false) {}
        
        ProfileInfo(const QString& n, const QString& p) 
            : name(n), path(p), displayName(n), isDefault(false) {}
    };

    /**
     * @struct BrowserInfo
     * @brief ブラウザの情報を格納する構造体
     */
    struct BrowserInfo {
        QString name;                              ///< ブラウザ名（例: "Firefox"）
        QString executable;                        ///< 実行ファイルのパス
        QString iconPath;                          ///< アイコンファイルのパス
        QMap<QString, ProfileInfo> profiles;       ///< プロファイルIDからProfileInfoへのマップ
        Constants::BrowserType type;               ///< ブラウザの種類
        
        BrowserInfo() : type(Constants::BrowserType::Unknown) {}
        
        BrowserInfo(const QString& n, const QString& e, Constants::BrowserType t)
            : name(n), executable(e), type(t) {}
    };

    explicit BrowserDetector(QObject* parent = nullptr);
    ~BrowserDetector() override = default;

    // コピーコンストラクタと代入演算子を削除
    BrowserDetector(const BrowserDetector&) = delete;
    BrowserDetector& operator=(const BrowserDetector&) = delete;

    // ムーブ演算子はデフォルトで許可
    BrowserDetector(BrowserDetector&&) = default;
    BrowserDetector& operator=(BrowserDetector&&) = default;

    /**
     * @brief システムにインストールされた全てのブラウザを検出
     * @return ブラウザIDからBrowserInfoへのマップ
     * @note 結果は5秒間キャッシュされます
     */
    QMap<QString, BrowserInfo> detectBrowsers();
    
    /**
     * @brief 指定されたブラウザがインストールされているかチェック
     * @param browserName ブラウザ名（"firefox", "chrome", "chromium"）
     * @return true: インストールされている, false: インストールされていない
     */
    bool isBrowserInstalled(const QString& browserName) const;
    
    /**
     * @brief ブラウザのアイコンパスを取得
     * @param browserName ブラウザ名
     * @return アイコンファイルのパス（ブラウザが見つからない場合は空文字列）
     */
    QString getBrowserIcon(const QString& browserName) const;
    
    /**
     * @brief 指定されたプロファイルでブラウザを起動
     * @param browser ブラウザID
     * @param profile プロファイルID
     * @param url 開くURL（セキュリティのため検証されます）
     * @return true: 起動成功, false: 起動失敗
     */
    bool launchBrowser(const QString& browser, const QString& profile, const QString& url);

    // セキュリティ検証メソッド
    /**
     * @brief URLの妥当性を検証
     * @param url 検証するURL
     * @return true: 有効なURL, false: 無効または危険なURL
     * @note 許可されるスキーム: http, https, file, about, chrome, edge
     */
    static bool isValidUrl(const QString& url);
    
    /**
     * @brief プロファイル名の妥当性を検証
     * @param profileName 検証するプロファイル名
     * @return true: 有効なプロファイル名, false: 無効または危険なプロファイル名
     */
    static bool isValidProfileName(const QString& profileName);
    
    /**
     * @brief URLをサニタイズ（危険な文字を除去）
     * @param url サニタイズするURL
     * @return サニタイズされたURL
     */
    static QString sanitizeUrl(const QString& url);
    
    /**
     * @brief プロファイル名をサニタイズ（危険な文字を除去）
     * @param profileName サニタイズするプロファイル名
     * @return サニタイズされたプロファイル名
     */
    static QString sanitizeProfileName(const QString& profileName);

    /**
     * @brief 実行ファイルパスの上書きマップを設定（YAMLなどから）
     * @param overrides browserId -> executable パス
     */
    void setExecutableOverrides(const QMap<QString, QString>& overrides) { m_execOverrides = overrides; }

    /**
     * @brief 有効/無効の上書きマップを設定（YAMLなどから）
     * @param overrides browserId -> enabled
     */
    void setEnabledOverrides(const QMap<QString, bool>& overrides) { m_enabledOverrides = overrides; }

signals:
    /**
     * @brief ブラウザが検出されたときに発行されるシグナル
     * @param browserName 検出されたブラウザの名前
     */
    void browserDetected(const QString& browserName);
    
    /**
     * @brief プロファイルが検出されたときに発行されるシグナル
     * @param browserName ブラウザ名
     * @param profileName 検出されたプロファイル名
     */
    void profileDetected(const QString& browserName, const QString& profileName);
    
    /**
     * @brief ブラウザの起動に失敗したときに発行されるシグナル
     * @param error エラーメッセージ
     */
    void launchError(const QString& error);

private:
    /**
     * @brief Firefoxのプロファイル一覧を取得
     * @return プロファイルIDからProfileInfoへのマップ
     */
    QMap<QString, ProfileInfo> getFirefoxProfiles();
    
    /**
     * @brief Chrome/Chromiumのプロファイル一覧を取得
     * @param browserName ブラウザ名（"google-chrome" または "chromium"）
     * @return プロファイルIDからProfileInfoへのマップ
     */
    QMap<QString, ProfileInfo> getChromeProfiles(const QString& browserName);
    
    /**
     * @brief 指定された名前の実行ファイルを検索
     * @param name 実行ファイル名
     * @return 実行ファイルのフルパス（見つからない場合は空文字列）
     */
    QString findExecutable(const QString& name) const;
    
    /**
     * @brief 複数の実行ファイル名のリストから最初に見つかったものを検索
     * @param names 実行ファイル名のリスト
     * @return 実行ファイルのフルパス（見つからない場合は空文字列）
     */
    QString findExecutableFromList(const QStringList& names) const;
    
    /**
     * @brief プロファイルの最終使用日時を取得
     * @param browserPath ブラウザのパス
     * @param profilePath プロファイルのパス
     * @return 最終使用日時
     */
    QDateTime getProfileLastUsed(const QString& browserPath, const QString& profilePath);
    
    /**
     * @brief Firefoxのプロファイルディレクトリを取得
     * @return プロファイルディレクトリのパス
     */
    QString getFirefoxProfilePath() const;
    
    /**
     * @brief Chrome/Chromiumのプロファイルディレクトリを取得
     * @param browserName ブラウザ名
     * @return プロファイルディレクトリのパス
     */
    QString getChromeProfilePath(const QString& browserName) const;
    
    // プロファイル解析ヘルパー
    /**
     * @brief Firefoxのprofiles.iniファイルを解析
     * @param iniPath profiles.iniのパス
     * @param profiles 結果を格納するマップ
     */
    void parseFirefoxIni(const QString& iniPath, QMap<QString, ProfileInfo>& profiles);
    
    /**
     * @brief Chrome/Chromiumの"Local State"ファイルを解析
     * @param localStatePath "Local State"ファイルのパス
     * @param configDir 設定ディレクトリ
     * @param profiles 結果を格納するマップ
     */
    void parseChromiumLocalState(const QString& localStatePath, 
                                const QString& configDir,
                                QMap<QString, ProfileInfo>& profiles);
    
    // 検出結果のキャッシュ
    mutable QMap<QString, BrowserInfo> m_cachedBrowsers;  ///< 検出されたブラウザ情報のキャッシュ
    mutable QDateTime m_lastDetection;                    ///< 最後に検出を実行した日時
    QMap<QString, QString> m_execOverrides;               ///< 実行ファイルパスの上書き
    QMap<QString, bool> m_enabledOverrides;               ///< 有効/無効の上書き
};

#endif // BROWSERDETECTOR_H
