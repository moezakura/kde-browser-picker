/**
 * @file browserdetector.cpp
 * @brief BrowserDetectorクラスの実装
 *
 * システムにインストールされたブラウザの検出、プロファイル情報の取得、
 * およびブラウザの起動機能を実装しています。
 * セキュリティを重視し、URLやプロファイル名の検証を行います。
 */

#include "browserdetector.h"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QStandardPaths>
#include <QSettings>
#include <QDebug>
#include <QStringList>
#include <QUrl>
#include <QRegularExpression>

BrowserDetector::BrowserDetector(QObject* parent)
    : QObject(parent)
{
}

QMap<QString, BrowserDetector::BrowserInfo> BrowserDetector::detectBrowsers()
{
    // 検出が最近実行された場合はキャッシュを使用（5秒以内）
    if (!m_cachedBrowsers.isEmpty() && 
        m_lastDetection.isValid() && 
        m_lastDetection.secsTo(QDateTime::currentDateTime()) < 5) {
        return m_cachedBrowsers;
    }

    QMap<QString, BrowserInfo> browsers;

    // Firefoxを検出（YAML上書き優先）
    if (m_enabledOverrides.contains("firefox") && !m_enabledOverrides.value("firefox")) {
        // 明示的に無効化
    } else {
    QString firefoxExec = m_execOverrides.value("firefox");
    if (firefoxExec.isEmpty()) {
        firefoxExec = findExecutable(Constants::FIREFOX_EXECUTABLE);
    }
    if (!firefoxExec.isEmpty()) {
        BrowserInfo firefox("Firefox", firefoxExec, Constants::BrowserType::Firefox);
        firefox.iconPath = "/usr/share/icons/hicolor/48x48/apps/firefox.png";
        firefox.profiles = getFirefoxProfiles();
        browsers["firefox"] = firefox;
        emit browserDetected("firefox");
    }
    }

    // Google Chromeを検出（YAML上書き優先）
    if (m_enabledOverrides.contains("chrome") && !m_enabledOverrides.value("chrome")) {
        // 無効化
    } else {
        QString chromeExec = m_execOverrides.value("chrome");
        if (chromeExec.isEmpty()) {
            chromeExec = findExecutableFromList(Constants::CHROME_EXECUTABLE_VARIANTS);
        }
        if (!chromeExec.isEmpty()) {
            BrowserInfo chrome("Google Chrome", chromeExec, Constants::BrowserType::Chrome);
            chrome.iconPath = "/usr/share/icons/hicolor/48x48/apps/google-chrome.png";
            chrome.profiles = getChromeProfiles("google-chrome");
            browsers["chrome"] = chrome;
            emit browserDetected("chrome");
        }
    }

    // Chromiumを検出（YAML上書き優先）
    if (m_enabledOverrides.contains("chromium") && !m_enabledOverrides.value("chromium")) {
        // 無効化
    } else {
        QString chromiumExec = m_execOverrides.value("chromium");
        if (chromiumExec.isEmpty()) {
            chromiumExec = findExecutable(Constants::CHROMIUM_EXECUTABLE);
        }
        if (!chromiumExec.isEmpty()) {
            BrowserInfo chromium("Chromium", chromiumExec, Constants::BrowserType::Chromium);
            chromium.iconPath = "/usr/share/icons/hicolor/48x48/apps/chromium.png";
            chromium.profiles = getChromeProfiles("chromium");
            browsers["chromium"] = chromium;
            emit browserDetected("chromium");
        }
    }

    // Vivaldiを検出（YAML上書き優先）
    if (m_enabledOverrides.contains("vivaldi") && !m_enabledOverrides.value("vivaldi")) {
        // 無効化
    } else {
        QString vivaldiExec = m_execOverrides.value("vivaldi");
        if (vivaldiExec.isEmpty()) {
            vivaldiExec = findExecutable(Constants::VIVALDI_EXECUTABLE);
        }
        if (!vivaldiExec.isEmpty()) {
            BrowserInfo vivaldi("Vivaldi", vivaldiExec, Constants::BrowserType::Vivaldi);
            vivaldi.iconPath = "/usr/share/icons/hicolor/48x48/apps/vivaldi.png";
            vivaldi.profiles = getChromeProfiles("vivaldi");
            browsers["vivaldi"] = vivaldi;
            emit browserDetected("vivaldi");
        }
    }

    // Zen Browser (flatpak)を検出（YAML上書き優先、profiles.ini存在時のみ）
    if (m_enabledOverrides.contains("zen") && !m_enabledOverrides.value("zen")) {
        // 無効化
    } else {
        const QString zenProfilesPath = QDir::homePath() + "/.var/app/" + Constants::ZEN_FLATPAK_ID + "/.zen";
        if (QFile::exists(zenProfilesPath + "/" + Constants::FIREFOX_CONFIG)) {
            QString zenExec = m_execOverrides.value("zen");
            if (zenExec.isEmpty()) {
                zenExec = findExecutable(Constants::ZEN_EXECUTABLE);
            }
            if (!zenExec.isEmpty()) {
                BrowserInfo zen("Zen Browser", zenExec, Constants::BrowserType::Zen);
                // flatpakのエクスポート先から実在するアイコンパスを使用（scalable SVGを優先）
                zen.iconPath = QString("/var/lib/flatpak/exports/share/icons/hicolor/scalable/apps/")
                               + Constants::ZEN_FLATPAK_ID + ".svg";
                if (!QFile::exists(zen.iconPath)) {
                    zen.iconPath = QDir::homePath() + "/.local/share/flatpak/exports/share/icons/hicolor/scalable/apps/"
                                   + Constants::ZEN_FLATPAK_ID + ".svg";
                }
                zen.profiles = getZenProfiles();
                browsers["zen"] = zen;
                emit browserDetected("zen");
            }
        }
    }

    m_cachedBrowsers = browsers;
    m_lastDetection = QDateTime::currentDateTime();
    
    return browsers;
}

bool BrowserDetector::isBrowserInstalled(const QString& browserName) const
{
    auto existsAndExec = [](const QString& p) {
        if (p.isEmpty()) return false;
        QFileInfo fi(p);
        return fi.exists() && fi.isFile() && fi.isExecutable();
    };

    if (browserName == "firefox") {
        if (m_enabledOverrides.contains("firefox") && !m_enabledOverrides.value("firefox")) return false;
        if (existsAndExec(m_execOverrides.value("firefox"))) return true;
        return !findExecutable(Constants::FIREFOX_EXECUTABLE).isEmpty();
    } else if (browserName == "chrome") {
        if (m_enabledOverrides.contains("chrome") && !m_enabledOverrides.value("chrome")) return false;
        if (existsAndExec(m_execOverrides.value("chrome"))) return true;
        return !findExecutable(Constants::CHROME_EXECUTABLE).isEmpty();
    } else if (browserName == "chromium") {
        if (m_enabledOverrides.contains("chromium") && !m_enabledOverrides.value("chromium")) return false;
        if (existsAndExec(m_execOverrides.value("chromium"))) return true;
        return !findExecutable(Constants::CHROMIUM_EXECUTABLE).isEmpty();
    } else if (browserName == "vivaldi") {
        if (m_enabledOverrides.contains("vivaldi") && !m_enabledOverrides.value("vivaldi")) return false;
        if (existsAndExec(m_execOverrides.value("vivaldi"))) return true;
        return !findExecutable(Constants::VIVALDI_EXECUTABLE).isEmpty();
    } else if (browserName == "zen") {
        if (m_enabledOverrides.contains("zen") && !m_enabledOverrides.value("zen")) return false;
        const QString zenIniPath = QDir::homePath() + "/.var/app/" + Constants::ZEN_FLATPAK_ID
                                   + "/.zen/" + Constants::FIREFOX_CONFIG;
        if (!QFile::exists(zenIniPath)) return false;
        if (existsAndExec(m_execOverrides.value("zen"))) return true;
        return !findExecutable(Constants::ZEN_EXECUTABLE).isEmpty();
    }
    return false;
}

QString BrowserDetector::getBrowserIcon(const QString& browserName) const
{
    if (m_cachedBrowsers.contains(browserName)) {
        return m_cachedBrowsers[browserName].iconPath;
    }
    return QString();
}

bool BrowserDetector::isValidUrl(const QString& url)
{
    if (url.isEmpty()) {
        return false;
    }
    
    // 許可されるスキームのリスト
    QStringList allowedSchemes = {"http", "https", "file", "about", "chrome", "edge"};
    QUrl qurl(url);
    
    if (qurl.isValid()) {
        QString scheme = qurl.scheme().toLower();
        if (scheme.isEmpty() || allowedSchemes.contains(scheme)) {
            // インジェクション攻撃に使用される可能性のある危険な文字をチェック
            static QRegularExpression dangerousChars(R"([;|`$\(\)\{\}\[\]<>])");
            return !dangerousChars.match(url).hasMatch();
        }
    }
    
    // スキームのないURLも許可（https://がプレフィックスされる）
    if (!url.contains("://")) {
        static QRegularExpression dangerousChars(R"([;|`$\(\)\{\}\[\]<>])");
        return !dangerousChars.match(url).hasMatch();
    }
    
    return false;
}

bool BrowserDetector::isValidProfileName(const QString& profileName)
{
    if (profileName.isEmpty()) {
        return false;
    }
    
    // プロファイル名は英数字、スペース、ドット、ハイフン、アンダースコアのみ許可
    static QRegularExpression validProfile(R"(^[a-zA-Z0-9 ._\-]+$)");
    return validProfile.match(profileName).hasMatch();
}

QString BrowserDetector::sanitizeUrl(const QString& url)
{
    QString sanitized = url;
    
    // nullバイトを削除
    sanitized.remove(QChar('\0'));
    
    // 余計な空白をトリミング
    sanitized = sanitized.trimmed();
    
    return sanitized;
}

QString BrowserDetector::sanitizeProfileName(const QString& profileName)
{
    QString sanitized = profileName;
    
    // nullバイトを削除
    sanitized.remove(QChar('\0'));
    
    // 余計な空白をトリミング
    sanitized = sanitized.trimmed();
    
    // 英数字、スペース、ドット、ハイフン、アンダースコア以外の文字を削除
    static QRegularExpression invalidChars(R"([^a-zA-Z0-9 ._\-])");
    sanitized.remove(invalidChars);
    
    return sanitized;
}

bool BrowserDetector::launchBrowser(const QString& browser, const QString& profile, const QString& url)
{
    // 入力をサニタイズして検証
    QString sanitizedUrl = sanitizeUrl(url);
    QString sanitizedProfile = sanitizeProfileName(profile);
    
    // URLの検証
    if (!isValidUrl(sanitizedUrl)) {
        emit launchError(tr("Invalid URL: %1").arg(url));
        return false;
    }
    
    // プロファイル名の検証
    if (!isValidProfileName(sanitizedProfile)) {
        emit launchError(tr("Invalid profile name: %1").arg(profile));
        return false;
    }
    
    if (!m_cachedBrowsers.contains(browser)) {
        emit launchError(tr("Browser %1 not found").arg(browser));
        return false;
    }

    const BrowserInfo& browserInfo = m_cachedBrowsers[browser];
    if (!browserInfo.profiles.contains(sanitizedProfile)) {
        emit launchError(tr("Profile %1 not found for browser %2").arg(sanitizedProfile, browser));
        return false;
    }

    QStringList args;
    
    switch (browserInfo.type) {
    case Constants::BrowserType::Firefox:
        args << "-P" << sanitizedProfile << sanitizedUrl;
        break;
        
    case Constants::BrowserType::Zen: {
        // flatpak経由の場合はApp IDを指定して起動、それ以外は直接起動
        const bool isFlatpak = browserInfo.executable == Constants::ZEN_EXECUTABLE
                               || browserInfo.executable.endsWith("/" + QString(Constants::ZEN_EXECUTABLE));
        if (isFlatpak) {
            // launch-script.sh経由で起動
            args << "run" << "--command=launch-script.sh" << Constants::ZEN_FLATPAK_ID;
        }
        args << "-P" << sanitizedProfile;
        // flatpak run に --file-forwarding を指定していないため @@u トークンは
        // 解釈されず文字列のまま渡る。通常引数として渡す（http/https URLは
        // サンドボックス内から直接アクセス可能、finish-args --share=network 済み）。
        args << sanitizedUrl;
        break;
    }
        
    case Constants::BrowserType::Chrome:
    case Constants::BrowserType::Chromium:
    case Constants::BrowserType::Vivaldi:
        args << QString("--profile-directory=%1").arg(sanitizedProfile) << sanitizedUrl;
        break;
        
    default:
        emit launchError(tr("Unknown browser type"));
        return false;
    }

    QProcess* process = new QProcess(this);
    process->setProgram(browserInfo.executable);
    process->setArguments(args);
    
    // アプリケーション終了後もプロセスが継続するようにデタッチ
    connect(process, &QProcess::started, [process]() {
        process->disconnect();
        process->deleteLater();
    });
    
    connect(process, &QProcess::errorOccurred, [this, browser](QProcess::ProcessError error) {
        emit launchError(tr("Failed to launch %1: %2").arg(browser).arg(static_cast<int>(error)));
    });
    
    return process->startDetached();
}

QString BrowserDetector::findExecutable(const QString& name) const
{
    // まずPATH内をチェック
    QString path = QStandardPaths::findExecutable(name);
    if (!path.isEmpty()) {
        return path;
    }
    
    // 一般的なインストール先をチェック
    QStringList commonPaths = {
        "/usr/bin/" + name,
        "/usr/local/bin/" + name,
        "/opt/" + name + "/" + name,
        QDir::homePath() + "/.local/bin/" + name
    };
    
    for (const QString& p : commonPaths) {
        if (QFile::exists(p) && QFileInfo(p).isExecutable()) {
            return p;
        }
    }
    
    return QString();
}

QString BrowserDetector::findExecutableFromList(const QStringList& names) const
{
    // リスト内の各名前を順番に試し、最初に見つかったものを返す
    for (const QString& name : names) {
        QString exec = findExecutable(name);
        if (!exec.isEmpty()) {
            return exec;
        }
    }
    
    return QString();
}

QString BrowserDetector::getFirefoxProfilePath() const
{
    return QDir::homePath() + "/.mozilla/firefox";
}

QString BrowserDetector::getChromeProfilePath(const QString& browserName) const
{
    return QDir::homePath() + "/.config/" + browserName;
}

QMap<QString, BrowserDetector::ProfileInfo> BrowserDetector::getFirefoxProfiles()
{
    QMap<QString, ProfileInfo> profiles;
    
    QString profilesPath = getFirefoxProfilePath();
    QString iniPath = profilesPath + "/" + Constants::FIREFOX_CONFIG;
    
    // snap版Firefoxのフォールバック（profiles.iniの存在で判定。~/.mozillaのみ存在して
    // 空の環境があるためディレクトリ存在判定は使わない）
    if (!QFile::exists(iniPath)) {
        const QString snapProfilesPath = QDir::homePath() + "/snap/firefox/common/.mozilla/firefox";
        const QString snapIniPath = snapProfilesPath + "/" + Constants::FIREFOX_CONFIG;
        if (QFile::exists(snapIniPath)) {
            profilesPath = snapProfilesPath;
            iniPath = snapIniPath;
        }
    }
    
    if (!QFile::exists(iniPath)) {
        qDebug() << "Firefox profiles.ini not found at" << iniPath;
        return profiles;
    }
    
    parseFirefoxIni(iniPath, profiles);
    
    // 各プロファイルの最終使用日時を取得
    for (auto& profile : profiles) {
        QString profileFullPath = profilesPath + "/" + profile.path;
        profile.lastUsed = getProfileLastUsed("firefox", profileFullPath);
        emit profileDetected("firefox", profile.name);
    }
    
    return profiles;
}

QMap<QString, BrowserDetector::ProfileInfo> BrowserDetector::getZenProfiles()
{
    QMap<QString, ProfileInfo> profiles;
    
    const QString profilesPath = QDir::homePath() + "/.var/app/" + Constants::ZEN_FLATPAK_ID + "/.zen";
    const QString iniPath = profilesPath + "/" + Constants::FIREFOX_CONFIG;
    
    if (!QFile::exists(iniPath)) {
        qDebug() << "Zen profiles.ini not found at" << iniPath;
        return profiles;
    }
    
    parseFirefoxIni(iniPath, profiles);
    
    // 各プロファイルの最終使用日時を取得（Firefoxと同じtimes.json扱い）
    for (auto& profile : profiles) {
        QString profileFullPath = profilesPath + "/" + profile.path;
        profile.lastUsed = getProfileLastUsed("zen", profileFullPath);
        emit profileDetected("zen", profile.name);
    }
    
    return profiles;
}

void BrowserDetector::parseFirefoxIni(const QString& iniPath, QMap<QString, ProfileInfo>& profiles)
{
    QSettings settings(iniPath, QSettings::IniFormat);
    
    // Firefoxは[Profile0], [Profile1]などのセクションを使用
    QStringList groups = settings.childGroups();
    
    for (const QString& group : groups) {
        if (!group.startsWith("Profile")) {
            continue;
        }
        
        settings.beginGroup(group);
        
        QString name = settings.value("Name").toString();
        QString path = settings.value("Path").toString();
        bool isDefault = settings.value("Default", false).toBool();
        
        if (!name.isEmpty() && !path.isEmpty()) {
            ProfileInfo info(name, path);
            info.isDefault = isDefault;
            profiles[name] = info;
        }
        
        settings.endGroup();
    }
}

QMap<QString, BrowserDetector::ProfileInfo> BrowserDetector::getChromeProfiles(const QString& browserName)
{
    QMap<QString, ProfileInfo> profiles;
    
    QString configPath = getChromeProfilePath(browserName);
    QString localStatePath = configPath + "/" + Constants::CHROME_CONFIG;
    
    // snap版Chromiumのフォールバック（Local Stateの存在で判定）
    if (browserName == "chromium" && !QFile::exists(localStatePath)) {
        const QString snapConfigPath = QDir::homePath() + "/snap/chromium/common/chromium";
        const QString snapLocalStatePath = snapConfigPath + "/" + Constants::CHROME_CONFIG;
        if (QFile::exists(snapLocalStatePath)) {
            configPath = snapConfigPath;
            localStatePath = snapLocalStatePath;
        }
    }
    
    if (!QFile::exists(localStatePath)) {
        qDebug() << browserName << "Local State not found at" << localStatePath;
        return profiles;
    }
    
    parseChromiumLocalState(localStatePath, configPath, profiles);
    
    // Get last used times
    for (auto& profile : profiles) {
        QString profileFullPath = configPath + "/" + profile.path;
        profile.lastUsed = getProfileLastUsed(browserName, profileFullPath);
        emit profileDetected(browserName, profile.name);
    }
    
    return profiles;
}

void BrowserDetector::parseChromiumLocalState(const QString& localStatePath, 
                                            const QString& configDir,
                                            QMap<QString, ProfileInfo>& profiles)
{
    QFile file(localStatePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open Local State file:" << localStatePath;
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qDebug() << "Invalid JSON in Local State file";
        return;
    }
    
    QJsonObject root = doc.object();
    QJsonObject profileObj = root["profile"].toObject();
    QJsonObject infoCache = profileObj["info_cache"].toObject();
    
    // Always add Default profile
    ProfileInfo defaultProfile("Default", "Default");
    defaultProfile.displayName = "Default";
    profiles["Default"] = defaultProfile;
    
    // Parse other profiles
    for (auto it = infoCache.begin(); it != infoCache.end(); ++it) {
        QString profileDir = it.key();
        QJsonObject info = it.value().toObject();
        
        QString name = info["name"].toString();
        if (name.isEmpty()) {
            name = profileDir;
        }
        
        ProfileInfo profile(name, profileDir);
        profile.displayName = name;
        
        // Check if this profile exists
        QString profilePath = configDir + "/" + profileDir;
        if (QDir(profilePath).exists()) {
            profiles[profileDir] = profile;
        }
    }
}

QDateTime BrowserDetector::getProfileLastUsed(const QString& browserPath, const QString& profilePath)
{
    // For Firefox/Zen, check times.json
    if (browserPath == "firefox" || browserPath == "zen") {
        QString timesPath = profilePath + "/times.json";
        if (QFile::exists(timesPath)) {
            QFile file(timesPath);
            if (file.open(QIODevice::ReadOnly)) {
                QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
                file.close();
                
                if (doc.isObject()) {
                    QJsonObject times = doc.object();
                    qint64 firstUse = static_cast<qint64>(times["firstUse"].toDouble() / 1000); // Convert from ms to s
                    if (firstUse > 0) {
                        return QDateTime::fromSecsSinceEpoch(firstUse);
                    }
                }
            }
        }
    }
    
    // For Chrome/Chromium/Vivaldi, check Preferences file
    else if (browserPath == "chrome" || browserPath == "chromium" || browserPath == "vivaldi") {
        QString prefsPath = profilePath + "/Preferences";
        if (QFile::exists(prefsPath)) {
            // Just use file modification time for now
            QFileInfo info(prefsPath);
            return info.lastModified();
        }
    }
    
    // Fallback: use directory modification time
    QFileInfo dirInfo(profilePath);
    return dirInfo.lastModified();
}
