/**
 * @file profilemanager.cpp
 * @brief ProfileManagerクラスの実装
 *
 * ブラウザプロファイルの管理機能を実装しています。
 * BrowserDetectorとConfigManagerを連携させ、プロファイル情報の
 * 取得、保存、表示、および起動を管理します。
 */

#include "profilemanager.h"
#include "configmanager.h"

#include <QDebug>
#include <algorithm>

ProfileManager::ProfileManager(ConfigManager* configManager, QObject* parent)
    : QObject(parent)
    , m_browserDetector(std::make_unique<BrowserDetector>(this))
    , m_configManager(configManager)
{
    // ブラウザ検出シグナルの接続
    connect(m_browserDetector.get(), &BrowserDetector::browserDetected,
            this, &ProfileManager::onBrowserDetected);
    connect(m_browserDetector.get(), &BrowserDetector::launchError,
            this, &ProfileManager::onLaunchError);
            
    // 設定管理シグナルの接続
    connect(m_configManager, &ConfigManager::profileSettingsChanged,
            this, &ProfileManager::profileSettingsChanged);
}

void ProfileManager::refreshProfiles()
{
    m_profiles.clear();
    
    // 全てのブラウザとそのプロファイルを検出
    auto browsers = m_browserDetector->detectBrowsers();
    
    for (auto it = browsers.begin(); it != browsers.end(); ++it) {
        const QString& browserId = it.key();
        const BrowserDetector::BrowserInfo& browserInfo = it.value();
        
        for (auto profIt = browserInfo.profiles.begin(); 
             profIt != browserInfo.profiles.end(); ++profIt) {
            const QString& profileId = profIt.key();
            const BrowserDetector::ProfileInfo& profileInfo = profIt.value();
            
            ProfileEntry entry;
            entry.browser = browserId;
            entry.browserDisplayName = browserInfo.name;
            entry.profileId = profileId;
            entry.profileDisplayName = profileInfo.displayName;
            entry.iconPath = browserInfo.iconPath;
            entry.lastUsed = profileInfo.lastUsed;
            entry.isDefault = profileInfo.isDefault;
            
            // 設定から設定情報を読み込み
            updateProfileFromConfig(entry);
            
            m_profiles.append(entry);
        }
    }
    
    // プロファイルをソート
    sortProfiles(m_profiles);
    
    m_lastRefresh = QDateTime::currentDateTime();
    emit profilesRefreshed();
}

QList<ProfileManager::ProfileEntry> ProfileManager::getAllProfiles(bool enabledOnly) const
{
    if (enabledOnly) {
        // 有効なプロファイルのみをフィルタリング
        QList<ProfileEntry> filtered;
        std::copy_if(m_profiles.begin(), m_profiles.end(), 
                    std::back_inserter(filtered),
                    [](const ProfileEntry& entry) { return entry.isEnabled; });
        return filtered;
    }
    
    return m_profiles;
}

QList<ProfileManager::ProfileEntry> ProfileManager::getProfilesForBrowser(const QString& browser, 
                                                                         bool enabledOnly) const
{
    QList<ProfileEntry> filtered;
    
    for (const ProfileEntry& entry : m_profiles) {
        if (entry.browser == browser && (!enabledOnly || entry.isEnabled)) {
            filtered.append(entry);
        }
    }
    
    return filtered;
}

ProfileManager::ProfileEntry ProfileManager::getProfile(const QString& browser, 
                                                       const QString& profileId) const
{
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                          [&](const ProfileEntry& entry) {
                              return entry.browser == browser && entry.profileId == profileId;
                          });
    
    if (it != m_profiles.end()) {
        return *it;
    }
    
    return ProfileEntry();
}

bool ProfileManager::hasProfile(const QString& browser, const QString& profileId) const
{
    return std::any_of(m_profiles.begin(), m_profiles.end(),
                      [&](const ProfileEntry& entry) {
                          return entry.browser == browser && entry.profileId == profileId;
                      });
}

ProfileManager::ProfileEntry ProfileManager::getDefaultProfile() const
{
    // First, check if we have a last used profile
    QPair<QString, QString> lastUsed = m_configManager->getLastUsed();
    if (!lastUsed.first.isEmpty() && !lastUsed.second.isEmpty()) {
        ProfileEntry entry = getProfile(lastUsed.first, lastUsed.second);
        if (!entry.browser.isEmpty() && entry.isEnabled) {
            return entry;
        }
    }
    
    // Otherwise, return the first enabled profile
    QList<ProfileEntry> enabled = getAllProfiles(true);
    if (!enabled.isEmpty()) {
        return enabled.first();
    }
    
    // No enabled profiles
    return ProfileEntry();
}

bool ProfileManager::launchProfile(const ProfileEntry& profile, const QString& url)
{
    return launchProfile(profile.browser, profile.profileId, url);
}

bool ProfileManager::launchProfile(const QString& browser, const QString& profileId, const QString& url)
{
    bool success = m_browserDetector->launchBrowser(browser, profileId, url);
    
    if (success) {
        // Update last used
        m_configManager->setLastUsed(browser, profileId);
        emit profileLaunched(browser, profileId);
    }
    
    return success;
}

void ProfileManager::setProfileEnabled(const QString& browser, const QString& profileId, bool enabled)
{
    m_configManager->setProfileEnabled(browser, profileId, enabled);
    
    // Update our cached entry
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                          [&](ProfileEntry& entry) {
                              return entry.browser == browser && entry.profileId == profileId;
                          });
    
    if (it != m_profiles.end()) {
        it->isEnabled = enabled;
    }
}

void ProfileManager::setProfileDisplayName(const QString& browser, const QString& profileId, 
                                          const QString& name)
{
    m_configManager->setProfileDisplayName(browser, profileId, name);
    
    // Update our cached entry
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                          [&](ProfileEntry& entry) {
                              return entry.browser == browser && entry.profileId == profileId;
                          });
    
    if (it != m_profiles.end()) {
        it->profileDisplayName = name.isEmpty() ? profileId : name;
    }
}

void ProfileManager::setProfileOrder(const QString& browser, const QString& profileId, int order)
{
    m_configManager->setProfileOrder(browser, profileId, order);
    
    // Update our cached entry
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                          [&](ProfileEntry& entry) {
                              return entry.browser == browser && entry.profileId == profileId;
                          });
    
    if (it != m_profiles.end()) {
        it->order = order;
        sortProfiles(m_profiles);
    }
}

void ProfileManager::moveProfileUp(const QString& browser, const QString& profileId)
{
    // Find the profile and the one before it
    auto profiles = getAllProfiles();
    
    for (int i = 1; i < profiles.size(); ++i) {
        if (profiles[i].browser == browser && profiles[i].profileId == profileId) {
            // Swap order values
            int prevOrder = profiles[i-1].order;
            int currOrder = profiles[i].order;
            
            setProfileOrder(profiles[i-1].browser, profiles[i-1].profileId, currOrder);
            setProfileOrder(browser, profileId, prevOrder);
            break;
        }
    }
}

void ProfileManager::moveProfileDown(const QString& browser, const QString& profileId)
{
    // Find the profile and the one after it
    auto profiles = getAllProfiles();
    
    for (int i = 0; i < profiles.size() - 1; ++i) {
        if (profiles[i].browser == browser && profiles[i].profileId == profileId) {
            // Swap order values
            int currOrder = profiles[i].order;
            int nextOrder = profiles[i+1].order;
            
            setProfileOrder(profiles[i+1].browser, profiles[i+1].profileId, currOrder);
            setProfileOrder(browser, profileId, nextOrder);
            break;
        }
    }
}

void ProfileManager::onBrowserDetected(const QString& browserName)
{
    qDebug() << "Browser detected:" << browserName;
}

void ProfileManager::onLaunchError(const QString& error)
{
    emit profileLaunchFailed(error);
}

void ProfileManager::updateProfileFromConfig(ProfileEntry& entry)
{
    // Load settings from config
    entry.isEnabled = m_configManager->isProfileEnabled(entry.browser, entry.profileId);
    
    QString customName = m_configManager->getProfileDisplayName(entry.browser, entry.profileId);
    if (!customName.isEmpty() && customName != entry.profileId) {
        entry.profileDisplayName = customName;
    }
    
    entry.order = m_configManager->getProfileOrder(entry.browser, entry.profileId);
}

void ProfileManager::sortProfiles(QList<ProfileEntry>& profiles) const
{
    std::sort(profiles.begin(), profiles.end());
}