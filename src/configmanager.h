/**
 * @file configmanager.h
 * @brief アプリケーションの設定を管理するクラス
 *
 * このファイルは、KDE Browser Pickerの設定管理機能を提供します。
 * ユーザー設定の読み書き、プロファイル設定の管理、最後に使用したブラウザの記録などを行います。
 */

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QString>
#include <QPair>
#include <memory>

// Forward declarations
class KConfig;
class KConfigGroup;

/**
 * @class ConfigManager
 * @brief アプリケーション設定の管理クラス
 *
 * ConfigManagerは、KDE Browser Pickerの全ての設定を管理します。
 * KConfigを使用してKDEの設定システムと統合し、設定の永続化を行います。
 *
 * 主な機能:
 * - 一般設定（タイムアウト、トレイアイコン表示など）
 * - ブラウザプロファイルの有効/無効状態の管理
 * - プロファイルの表示名と表示順序の管理
 * - 最後に使用したブラウザとプロファイルの記録
 *
 * @note このクラスはシングルトンパターンではありませんが、
 *       アプリケーション内で単一のインスタンスとして使用されることを想定しています。
 */
class ConfigManager : public QObject {
    Q_OBJECT

public:
    explicit ConfigManager(QObject* parent = nullptr);
    ~ConfigManager() override;

    // コピーコンストラクタと代入演算子を削除（シングルトン的な使用を保証）
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    // 一般設定
    /**
     * @brief デフォルトのタイムアウト時間を取得
     * @return タイムアウト時間（秒）
     */
    int defaultTimeout() const;
    
    /**
     * @brief デフォルトのタイムアウト時間を設定
     * @param seconds タイムアウト時間（秒）。最小値と最大値の範囲内に制限されます
     */
    void setDefaultTimeout(int seconds);
    
    /**
     * @brief 最後に使用したブラウザを記憶するかどうかを取得
     * @return true: 記憶する, false: 記憶しない
     */
    bool rememberLastUsed() const;
    
    /**
     * @brief 最後に使用したブラウザを記憶するかどうかを設定
     * @param remember true: 記憶する, false: 記憶しない
     */
    void setRememberLastUsed(bool remember);
    
    /**
     * @brief システムトレイアイコンを表示するかどうかを取得
     * @return true: 表示する, false: 表示しない
     */
    bool showTrayIcon() const;
    
    /**
     * @brief システムトレイアイコンを表示するかどうかを設定
     * @param show true: 表示する, false: 表示しない
     */
    void setShowTrayIcon(bool show);
    
    /**
     * @brief ウィンドウのジオメトリ（位置とサイズ）を取得
     * @return Qtのジオメトリデータ（QByteArray形式）
     */
    QByteArray windowGeometry() const;
    
    /**
     * @brief ウィンドウのジオメトリ（位置とサイズ）を保存
     * @param geometry Qtのジオメトリデータ（QByteArray形式）
     */
    void setWindowGeometry(const QByteArray& geometry);
    
    // プロファイル設定
    /**
     * @brief 指定されたブラウザプロファイルが有効かどうかを取得
     * @param browser ブラウザ名（例: "firefox", "chrome"）
     * @param profile プロファイルID
     * @return true: 有効, false: 無効
     */
    bool isProfileEnabled(const QString& browser, const QString& profile) const;
    
    /**
     * @brief 指定されたブラウザプロファイルの有効/無効を設定
     * @param browser ブラウザ名（例: "firefox", "chrome"）
     * @param profile プロファイルID
     * @param enabled true: 有効にする, false: 無効にする
     */
    void setProfileEnabled(const QString& browser, const QString& profile, bool enabled);
    
    /**
     * @brief プロファイルの表示名を取得
     * @param browser ブラウザ名
     * @param profile プロファイルID
     * @return カスタム表示名（設定されていない場合はプロファイルIDを返す）
     */
    QString getProfileDisplayName(const QString& browser, const QString& profile) const;
    
    /**
     * @brief プロファイルの表示名を設定
     * @param browser ブラウザ名
     * @param profile プロファイルID
     * @param name カスタム表示名（空文字列の場合は設定を削除）
     */
    void setProfileDisplayName(const QString& browser, const QString& profile, const QString& name);
    
    /**
     * @brief プロファイルの表示順序を取得
     * @param browser ブラウザ名
     * @param profile プロファイルID
     * @return 表示順序（小さい値ほど上に表示）
     */
    int getProfileOrder(const QString& browser, const QString& profile) const;
    
    /**
     * @brief プロファイルの表示順序を設定
     * @param browser ブラウザ名
     * @param profile プロファイルID
     * @param order 表示順序（小さい値ほど上に表示）
     */
    void setProfileOrder(const QString& browser, const QString& profile, int order);
    
    // 最後に使用したプロファイル
    /**
     * @brief 最後に使用したブラウザとプロファイルを取得
     * @return ブラウザ名とプロファイルIDのペア
     */
    QPair<QString, QString> getLastUsed() const;
    
    /**
     * @brief 最後に使用したブラウザとプロファイルを記録
     * @param browser ブラウザ名
     * @param profile プロファイルID
     */
    void setLastUsed(const QString& browser, const QString& profile);
    
    // ユーティリティ関数
    /**
     * @brief 設定をディスクに同期（保存）
     * @note 通常は自動的に保存されますが、即座に保存したい場合に使用
     */
    void sync();
    
    /**
     * @brief 指定されたグループとキーが設定ファイルに存在するかチェック
     * @param group グループ名
     * @param key キー名
     * @return true: 存在する, false: 存在しない
     */
    bool hasKey(const QString& group, const QString& key) const;
    
signals:
    /**
     * @brief 設定が変更されたときに発行されるシグナル
     */
    void configChanged();
    
    /**
     * @brief プロファイル設定が変更されたときに発行されるシグナル
     * @param browser 変更されたプロファイルのブラウザ名
     * @param profile 変更されたプロファイルのID
     */
    void profileSettingsChanged(const QString& browser, const QString& profile);

private:
    std::unique_ptr<KConfig> m_config;  ///< KDE設定ファイルへのアクセスオブジェクト
    
    // 設定グループへのアクセスヘルパーメソッド
    /**
     * @brief 一般設定グループを取得
     */
    KConfigGroup generalGroup() const;
    
    /**
     * @brief ブラウザ設定グループを取得
     */
    KConfigGroup browsersGroup() const;
    
    /**
     * @brief 最後に使用した設定グループを取得
     */
    KConfigGroup lastUsedGroup() const;
    
    /**
     * @brief 特定のプロファイル設定グループを取得
     * @param browser ブラウザ名
     * @param profile プロファイルID
     */
    KConfigGroup profileGroup(const QString& browser, const QString& profile) const;
    
    // 設定ファイルの検証
    /**
     * @brief 設定ファイルの整合性を確認し、必要に応じて初期化
     */
    void ensureConfigValid();
    
    /**
     * @brief 古い形式の設定ファイルを新しい形式に移行
     */
    void migrateOldConfig();
};

#endif // CONFIGMANAGER_H