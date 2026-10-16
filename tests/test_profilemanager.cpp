/**
 * @file test_profilemanager.cpp
 * @brief ProfileManager::applyProfileSettings の単体テスト
 *
 * HOME と XDG_CONFIG_HOME を一時ディレクトリへ差し替え、偽の Firefox
 * profiles.ini と偽の実行ファイルでプロファイルを検出させる。
 * ユーザーの実設定(~/.config/kde-browser-pickerrc)には触れない。
 */

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QList>
#include <QString>
#include <QTemporaryDir>
#include <QTextStream>

#include "../src/configmanager.h"
#include "../src/profilemanager.h"

namespace {

/** 実行可能なダミーファイルを作成する */
QString createExecutableDummy(const QString& dir, const QString& name)
{
    const QString path = dir + "/" + name;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        QTextStream out(&f);
        out << "#!/bin/sh\nexit 0\n";
        f.close();
        QFile::setPermissions(path, QFile::permissions(path) | QFileDevice::ExeOwner);
    }
    return path;
}

/** ファイルへテキストを書き込む */
bool writeText(const QString& path, const QString& text)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }
    QTextStream out(&f);
    out << text;
    return true;
}

/** HOME / XDG_CONFIG_HOME を隔離し Firefox プロファイル3件を検出させるフィクスチャ */
class ProfileManagerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_home.isValid());
        ASSERT_TRUE(m_config.isValid());
        ASSERT_TRUE(m_bin.isValid());

        m_origHome = qgetenv("HOME");
        m_origXdg = qgetenv("XDG_CONFIG_HOME");
        m_origYaml = qgetenv(Constants::YAML_ENV_PATH);
        qputenv("HOME", m_home.path().toUtf8());
        qputenv("XDG_CONFIG_HOME", m_config.path().toUtf8());

        const QString ffDir = m_home.path() + "/.mozilla/firefox";
        ASSERT_TRUE(QDir().mkpath(ffDir));
        ASSERT_TRUE(writeText(ffDir + "/profiles.ini",
                              "[Profile0]\nName=alpha\nIsRelative=1\nPath=alpha.dir\nDefault=1\n\n"
                              "[Profile1]\nName=beta\nIsRelative=1\nPath=beta.dir\n\n"
                              "[Profile2]\nName=gamma\nIsRelative=1\nPath=gamma.dir\n"));

        const QString fakeFirefox = createExecutableDummy(m_bin.path(), "fake_firefox");
        const QString yamlPath = m_bin.path() + "/kde-browser-picker.yaml";
        ASSERT_TRUE(writeText(yamlPath,
                              "browsers:\n  firefox:\n    path: " + fakeFirefox + "\n    enabled: true\n"));
        qputenv(Constants::YAML_ENV_PATH, yamlPath.toUtf8());

        m_cfg = new ConfigManager();
        m_pm = new ProfileManager(m_cfg);
        m_pm->refreshProfiles();
    }

    void TearDown() override
    {
        delete m_pm;
        delete m_cfg;
        restoreEnv("HOME", m_origHome);
        restoreEnv("XDG_CONFIG_HOME", m_origXdg);
        restoreEnv(Constants::YAML_ENV_PATH, m_origYaml);
    }

    /** 環境変数を元の値へ戻す（元が空なら unset） */
    static void restoreEnv(const char* name, const QByteArray& value)
    {
        if (value.isEmpty()) {
            qunsetenv(name);
        } else {
            qputenv(name, value);
        }
    }

    /** firefox のプロファイルID を現在のソート順で返す */
    QStringList firefoxIds(bool enabledOnly = false) const
    {
        QStringList ids;
        for (const auto& p : m_pm->getAllProfiles(enabledOnly)) {
            if (p.browser == "firefox") {
                ids << p.profileId;
            }
        }
        return ids;
    }

    QByteArray m_origHome;
    QByteArray m_origXdg;
    QByteArray m_origYaml;
    QTemporaryDir m_home;
    QTemporaryDir m_config;
    QTemporaryDir m_bin;
    ConfigManager* m_cfg = nullptr;
    ProfileManager* m_pm = nullptr;
};

} // namespace

TEST_F(ProfileManagerTest, DetectsFakeFirefoxProfiles)
{
    EXPECT_EQ(firefoxIds().size(), 3);
    EXPECT_TRUE(m_pm->hasProfile("firefox", "alpha"));
    EXPECT_TRUE(m_pm->hasProfile("firefox", "beta"));
    EXPECT_TRUE(m_pm->hasProfile("firefox", "gamma"));
}

TEST_F(ProfileManagerTest, AppliesOrder)
{
    m_pm->applyProfileSettings({
        {"firefox", "gamma", 0, true},
        {"firefox", "alpha", 1, true},
        {"firefox", "beta", 2, true},
    });
    EXPECT_EQ(firefoxIds(), (QStringList{"gamma", "alpha", "beta"}));

    // 設定値としても永続化されている
    EXPECT_EQ(m_cfg->getProfileOrder("firefox", "gamma"), 0);
    EXPECT_EQ(m_cfg->getProfileOrder("firefox", "beta"), 2);
}

TEST_F(ProfileManagerTest, AppliesEnabledFlag)
{
    m_pm->applyProfileSettings({
        {"firefox", "alpha", 0, true},
        {"firefox", "beta", 1, false},
        {"firefox", "gamma", 2, true},
    });
    EXPECT_EQ(firefoxIds(true), (QStringList{"alpha", "gamma"}));
    EXPECT_EQ(firefoxIds(false).size(), 3);
    EXPECT_FALSE(m_cfg->isProfileEnabled("firefox", "beta"));
}

TEST_F(ProfileManagerTest, WritesOnlyChangedItems)
{
    // 初期状態を確定させる
    m_pm->applyProfileSettings({
        {"firefox", "alpha", 0, true},
        {"firefox", "beta", 1, true},
        {"firefox", "gamma", 2, true},
    });

    int changed = 0;
    QObject::connect(m_pm, &ProfileManager::profileSettingsChanged,
                     [&changed](const QString&, const QString&) { ++changed; });
    int refreshed = 0;
    QObject::connect(m_pm, &ProfileManager::profilesRefreshed, [&refreshed]() { ++refreshed; });

    // 同一内容 -> 書込なし
    m_pm->applyProfileSettings({
        {"firefox", "alpha", 0, true},
        {"firefox", "beta", 1, true},
        {"firefox", "gamma", 2, true},
    });
    EXPECT_EQ(changed, 0);

    // beta の無効化のみ -> 1回
    m_pm->applyProfileSettings({
        {"firefox", "alpha", 0, true},
        {"firefox", "beta", 1, false},
        {"firefox", "gamma", 2, true},
    });
    EXPECT_EQ(changed, 1);

    // alpha と gamma の order 入替 -> 2回
    changed = 0;
    m_pm->applyProfileSettings({
        {"firefox", "alpha", 2, true},
        {"firefox", "beta", 1, false},
        {"firefox", "gamma", 0, true},
    });
    EXPECT_EQ(changed, 2);
    EXPECT_EQ(firefoxIds(), (QStringList{"gamma", "beta", "alpha"}));
    EXPECT_EQ(refreshed, 0);
}

TEST_F(ProfileManagerTest, IgnoresUnknownProfiles)
{
    const QStringList before = firefoxIds();
    int changed = 0;
    QObject::connect(m_pm, &ProfileManager::profileSettingsChanged,
                     [&changed](const QString&, const QString&) { ++changed; });

    m_pm->applyProfileSettings({
        {"firefox", "no-such-profile", 0, false},
        {"nosuchbrowser", "alpha", 1, false},
    });
    m_pm->applyProfileSettings({});

    EXPECT_EQ(changed, 0);
    EXPECT_EQ(firefoxIds(), before);
    EXPECT_EQ(firefoxIds(true).size(), 3);
}
