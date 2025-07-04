/**
 * @file configmanager.cpp
 * @brief ConfigManagerクラスの実装
 *
 * KDEの設定システムを使用して、アプリケーション設定の読み書きを管理します。
 * 設定ファイルは "kde-browser-pickerrc" として保存されます。
 */

#include "configmanager.h"
#include "constants.h"

#include <KConfig>
#include <KConfigGroup>
#include <QDebug>

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent)
    , m_config(std::make_unique<KConfig>("kde-browser-pickerrc"))
{
    // 設定ファイルの整合性を確認し、必要に応じて初期化
    ensureConfigValid();
}

ConfigManager::~ConfigManager() = default;

int ConfigManager::defaultTimeout() const
{
    return generalGroup().readEntry(Constants::CONFIG_KEY_DEFAULT_TIMEOUT, 
                                   Constants::DEFAULT_TIMEOUT);
}

void ConfigManager::setDefaultTimeout(int seconds)
{
    // タイムアウト値を有効範囲内に制限
    seconds = qBound(Constants::MIN_TIMEOUT, seconds, Constants::MAX_TIMEOUT);
    
    generalGroup().writeEntry(Constants::CONFIG_KEY_DEFAULT_TIMEOUT, seconds);
    sync();
    emit configChanged();
}

bool ConfigManager::rememberLastUsed() const
{
    return generalGroup().readEntry(Constants::CONFIG_KEY_REMEMBER_LAST_USED, true);
}

void ConfigManager::setRememberLastUsed(bool remember)
{
    generalGroup().writeEntry(Constants::CONFIG_KEY_REMEMBER_LAST_USED, remember);
    sync();
    emit configChanged();
}

bool ConfigManager::showTrayIcon() const
{
    return generalGroup().readEntry(Constants::CONFIG_KEY_SHOW_TRAY_ICON, false);
}

void ConfigManager::setShowTrayIcon(bool show)
{
    generalGroup().writeEntry(Constants::CONFIG_KEY_SHOW_TRAY_ICON, show);
    sync();
    emit configChanged();
}

QByteArray ConfigManager::windowGeometry() const
{
    return generalGroup().readEntry(Constants::CONFIG_KEY_WINDOW_GEOMETRY, QByteArray());
}

void ConfigManager::setWindowGeometry(const QByteArray& geometry)
{
    generalGroup().writeEntry(Constants::CONFIG_KEY_WINDOW_GEOMETRY, geometry);
    sync();
}

bool ConfigManager::isProfileEnabled(const QString& browser, const QString& profile) const
{
    return profileGroup(browser, profile).readEntry("Enabled", true);
}

void ConfigManager::setProfileEnabled(const QString& browser, const QString& profile, bool enabled)
{
    profileGroup(browser, profile).writeEntry("Enabled", enabled);
    sync();
    emit profileSettingsChanged(browser, profile);
}

QString ConfigManager::getProfileDisplayName(const QString& browser, const QString& profile) const
{
    return profileGroup(browser, profile).readEntry("DisplayName", profile);
}

void ConfigManager::setProfileDisplayName(const QString& browser, const QString& profile, const QString& name)
{
    if (name.isEmpty() || name == profile) {
        // カスタム表示名が空またはプロファイル名と同じ場合は削除
        profileGroup(browser, profile).deleteEntry("DisplayName");
    } else {
        profileGroup(browser, profile).writeEntry("DisplayName", name);
    }
    sync();
    emit profileSettingsChanged(browser, profile);
}

int ConfigManager::getProfileOrder(const QString& browser, const QString& profile) const
{
    return profileGroup(browser, profile).readEntry("Order", 999);
}

void ConfigManager::setProfileOrder(const QString& browser, const QString& profile, int order)
{
    profileGroup(browser, profile).writeEntry("Order", order);
    sync();
    emit profileSettingsChanged(browser, profile);
}

QPair<QString, QString> ConfigManager::getLastUsed() const
{
    QString browser = lastUsedGroup().readEntry("Browser", QString());
    QString profile = lastUsedGroup().readEntry("Profile", QString());
    
    return qMakePair(browser, profile);
}

void ConfigManager::setLastUsed(const QString& browser, const QString& profile)
{
    // "最後に使用したブラウザを記憶"設定がオフの場合は何もしない
    if (!rememberLastUsed()) {
        return;
    }
    
    lastUsedGroup().writeEntry("Browser", browser);
    lastUsedGroup().writeEntry("Profile", profile);
    sync();
}

void ConfigManager::sync()
{
    m_config->sync();
}

bool ConfigManager::hasKey(const QString& group, const QString& key) const
{
    return m_config->group(group).hasKey(key);
}

KConfigGroup ConfigManager::generalGroup() const
{
    return m_config->group(Constants::CONFIG_GROUP_GENERAL);
}

KConfigGroup ConfigManager::browsersGroup() const
{
    return m_config->group(Constants::CONFIG_GROUP_BROWSERS);
}

KConfigGroup ConfigManager::lastUsedGroup() const
{
    return m_config->group(Constants::CONFIG_GROUP_LAST_USED);
}

KConfigGroup ConfigManager::profileGroup(const QString& browser, const QString& profile) const
{
    // 階層的なグループ構造を作成: Browsers/firefox/ProfileName
    QString groupPath = QString("%1/%2/%3")
        .arg(Constants::CONFIG_GROUP_BROWSERS)
        .arg(browser)
        .arg(profile);
    
    return m_config->group(groupPath);
}

void ConfigManager::ensureConfigValid()
{
    // 新規設定ファイルかどうかをチェック
    if (!generalGroup().exists()) {
        // 新規設定ファイルの場合はデフォルト値を設定
        setDefaultTimeout(Constants::DEFAULT_TIMEOUT);
        setRememberLastUsed(true);
        setShowTrayIcon(false);
        
        qDebug() << "Created new config file with defaults";
    }
    
    // 古い設定形式をチェックし、必要に応じて移行
    migrateOldConfig();
}

void ConfigManager::migrateOldConfig()
{
    // 設定ファイル形式の移行を処理する場所
    // 現在はバージョンをチェックするのみ
    int configVersion = generalGroup().readEntry("ConfigVersion", 1);
    
    if (configVersion < 2) {
        // 必要に応じて移行を実行
        // 例: プロファイル設定を新しい構造に移動
        
        // 設定バージョンを更新
        generalGroup().writeEntry("ConfigVersion", 2);
        sync();
        
        qDebug() << "Migrated config from version" << configVersion << "to 2";
    }
}