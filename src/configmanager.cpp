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
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QSaveFile>

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent)
    , m_config(std::make_unique<KConfig>("kde-browser-pickerrc"))
{
    // 設定ファイルの整合性を確認し、必要に応じて初期化
    ensureConfigValid();

    // YAML上書きの読み込み（存在すれば）
    loadYamlOverrides();
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

QMap<QString, QString> ConfigManager::browserExecutableOverrides() const
{
    return m_browserExecOverrides;
}

QString ConfigManager::browserExecutableOverride(const QString& browser) const
{
    return m_browserExecOverrides.value(browser);
}

QMap<QString, bool> ConfigManager::browserEnabledOverrides() const
{
    return m_browserEnabledOverrides;
}

bool ConfigManager::isBrowserEnabledOverride(const QString& browser) const
{
    if (m_browserEnabledOverrides.contains(browser)) {
        return m_browserEnabledOverrides.value(browser);
    }
    return true; // 未指定は有効扱い
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

// 簡易YAMLローダー: 以下の最小構文を想定
// browsers:
//   firefox: /opt/firefox/firefox
//   chrome: /usr/bin/google-chrome-stable
//   chromium: /usr/local/bin/chromium
// もしくはネスト形式:
//   firefox:
//     path: /opt/firefox/firefox
void ConfigManager::loadYamlOverrides()
{
    m_browserExecOverrides.clear();
    m_browserEnabledOverrides.clear();

    // 参照YAMLパスを決定
    QByteArray envPath = qgetenv(Constants::YAML_ENV_PATH);
    QString yamlPath;
    if (!envPath.isEmpty()) {
        yamlPath = QString::fromUtf8(envPath);
    } else {
        const QString cfgDir = QDir::homePath() + "/.config";
        const QString yaml1 = cfgDir + "/" + Constants::YAML_CONFIG_FILENAME_YAML;
        const QString yaml2 = cfgDir + "/" + Constants::YAML_CONFIG_FILENAME_YML;
        if (QFile::exists(yaml1)) yamlPath = yaml1;
        else if (QFile::exists(yaml2)) yamlPath = yaml2;
    }

    if (yamlPath.isEmpty()) {
        return; // YAMLなし
    }

    QFile f(yamlPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open YAML config:" << yamlPath;
        return;
    }

    QTextStream in(&f);
    in.setCodec("UTF-8");

    auto isExecutableFile = [](const QString& p) -> bool {
        QFileInfo info(p);
        return !p.isEmpty() && info.exists() && info.isFile() && info.isExecutable();
    };
    auto cleanValue = [](QString v) -> QString {
        v = v.trimmed();
        if ((v.startsWith('"') && v.endsWith('"')) || (v.startsWith('\'') && v.endsWith('\''))) {
            v = v.mid(1, v.size() - 2);
        }
        return v.trimmed();
    };
    auto parseBool = [](const QString& v, bool& ok) -> bool {
        QString s = v.trimmed().toLower();
        ok = true;
        if (s == "true" || s == "yes" || s == "on" || s == "1") return true;
        if (s == "false" || s == "no" || s == "off" || s == "0") return false;
        ok = false;
        return true;
    };

    bool inBrowsers = false;
    int baseIndent = -1;
    QString currentKey;
    int currentKeyIndent = -1;

    while (!in.atEnd()) {
        QString line = in.readLine();
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith('#')) {
            continue;
        }

        int indent = 0;
        while (indent < line.size() && line[indent] == ' ') ++indent;

        if (!inBrowsers) {
            if (trimmed == "browsers:") {
                inBrowsers = true;
                baseIndent = indent;
                currentKey.clear();
                currentKeyIndent = -1;
            }
            continue;
        }

        if (indent <= baseIndent) {
            // browsers ブロック終了
            inBrowsers = false;
            currentKey.clear();
            currentKeyIndent = -1;
            continue;
        }

        if (indent == baseIndent + 2) {
            // 新しいブラウザキー
            int cpos = line.indexOf(':', indent);
            if (cpos <= indent) continue;
            QString key = line.mid(indent, cpos - indent).trimmed();
            QString rest = line.mid(cpos + 1).trimmed();

            if (key == "firefox" || key == "chrome" || key == "chromium") {
                if (!rest.isEmpty()) {
                    QString path = cleanValue(rest);
                    if (isExecutableFile(path)) {
                        m_browserExecOverrides.insert(key, path);
                    }
                    currentKey.clear();
                    currentKeyIndent = -1;
                } else {
                    currentKey = key;
                    currentKeyIndent = indent;
                }
            } else {
                currentKey.clear();
                currentKeyIndent = -1;
            }
            continue;
        }

        if (!currentKey.isEmpty() && indent >= baseIndent + 4) {
            int cpos = line.indexOf(':', indent);
            if (cpos > indent) {
                QString subKey = line.mid(indent, cpos - indent).trimmed();
                QString subRest = line.mid(cpos + 1).trimmed();
                if (subKey == "path" && !subRest.isEmpty()) {
                    QString path = cleanValue(subRest);
                    if (isExecutableFile(path)) {
                        m_browserExecOverrides.insert(currentKey, path);
                    }
                } else if (subKey == "enabled" && !subRest.isEmpty()) {
                    bool okb = false;
                    bool val = parseBool(cleanValue(subRest), okb);
                    if (okb) {
                        m_browserEnabledOverrides.insert(currentKey, val);
                    }
                }
            }
            continue;
        }
    }
}

bool ConfigManager::deployDefaults(bool overwriteYaml)
{
    bool changed = false;

    // rc: ensure defaults exist
    if (!generalGroup().exists()) {
        // ensureConfigValid() 内で作成されるが、明示的に初期値を再設定
        setDefaultTimeout(Constants::DEFAULT_TIMEOUT);
        setRememberLastUsed(true);
        setShowTrayIcon(false);
        changed = true;
    }

    // YAML template
    const QString cfgDir = QDir::homePath() + "/.config";
    QDir().mkpath(cfgDir);
    const QString yaml1 = cfgDir + "/" + Constants::YAML_CONFIG_FILENAME_YAML;
    const QString yaml2 = cfgDir + "/" + Constants::YAML_CONFIG_FILENAME_YML;
    const bool yamlExists = QFile::exists(yaml1) || QFile::exists(yaml2);
    const QString yamlPath = yamlExists ? (overwriteYaml ? (QFile::exists(yaml1) ? yaml1 : yaml2) : QString()) : yaml1;
    if (!yamlPath.isEmpty()) {
        QSaveFile f(yamlPath);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&f);
            out.setCodec("UTF-8");
            out.setGenerateByteOrderMark(false);
            // ASCII-only template to avoid mojibake
            out << "# KDE Browser Picker YAML configuration\n";
            out << "# Override executable paths only if needed.\n";
            out << "# Supported keys under 'browsers': firefox, chrome, chromium\n";
            out << "#\n";
            out << "# Example (inline):\n";
            out << "# browsers:\n";
            out << "#   firefox: /opt/firefox/firefox\n";
            out << "#   chrome: /usr/bin/google-chrome-stable\n";
            out << "#   chromium: /usr/local/bin/chromium\n";
            out << "#\n";
            out << "# Example (nested):\n";
            out << "# browsers:\n";
            out << "#   firefox:\n";
            out << "#     path: /opt/firefox/firefox\n";
            out << "#     enabled: true\n";
            if (f.commit()) {
                changed = true;
            }
        }
    }

    return changed;
}
