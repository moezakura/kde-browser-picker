/**
 * @file test_yaml_overrides.cpp
 * @brief YAMLによるブラウザ実行パス上書きのテスト
 */

#include <gtest/gtest.h>
#include <QFile>
#include <QDir>
#include <QTemporaryDir>
#include <QTextStream>
#include <QFileInfo>
#include <QProcessEnvironment>

#include "../src/configmanager.h"
#include "../src/browserdetector.h"

// ヘルパー: 実行可能なダミーファイルを作成
static QString createExecutableDummy(const QString& dir, const QString& name)
{
    QString path = dir + "/" + name;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly)) {
        QTextStream out(&f);
        out << "#!/bin/sh\nexit 0\n";
        f.close();
        QFile::setPermissions(path, QFile::permissions(path) | QFileDevice::ExeOwner);
    }
    return path;
}

TEST(YamlOverrides, LoadAndApply)
{
    QTemporaryDir tmpdir;
    ASSERT_TRUE(tmpdir.isValid());

    // ダミー実行ファイル
    QString fakeFirefox = createExecutableDummy(tmpdir.path(), "fake_firefox");

    // YAML作成（ネスト形式で enabled: true）
    QString yamlPath = tmpdir.path() + "/kde-browser-picker.yaml";
    QFile yaml(yamlPath);
    ASSERT_TRUE(yaml.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    {
        QTextStream out(&yaml);
        out.setCodec("UTF-8");
        out << "browsers:\n";
        out << "  firefox:\n";
        out << "    path: " << fakeFirefox << "\n";
        out << "    enabled: true\n";
    }
    yaml.close();

    // 環境変数でパス指定
    qputenv(Constants::YAML_ENV_PATH, yamlPath.toUtf8());

    // ConfigManagerで読み込み
    ConfigManager cfg;
    auto overrides = cfg.browserExecutableOverrides();
    ASSERT_TRUE(overrides.contains("firefox"));
    EXPECT_EQ(overrides.value("firefox"), fakeFirefox);

    // BrowserDetectorに適用し、検出できること（プロファイルは空で良い）
    BrowserDetector detector;
    detector.setExecutableOverrides(overrides);
    detector.setEnabledOverrides(cfg.browserEnabledOverrides());
    EXPECT_TRUE(detector.isBrowserInstalled("firefox"));

    auto browsers = detector.detectBrowsers();
    ASSERT_TRUE(browsers.contains("firefox"));
    EXPECT_EQ(browsers["firefox"].executable, fakeFirefox);

    // enabled: false にして無効化を確認
    ASSERT_TRUE(yaml.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
    {
        QTextStream out(&yaml);
        out.setCodec("UTF-8");
        out << "browsers:\n";
        out << "  firefox:\n";
        out << "    path: " << fakeFirefox << "\n";
        out << "    enabled: false\n";
    }
    yaml.close();

    // 再読み込み
    ConfigManager cfg2;
    BrowserDetector detector2;
    detector2.setExecutableOverrides(cfg2.browserExecutableOverrides());
    detector2.setEnabledOverrides(cfg2.browserEnabledOverrides());
    EXPECT_FALSE(detector2.isBrowserInstalled("firefox"));
    auto browsers2 = detector2.detectBrowsers();
    EXPECT_FALSE(browsers2.contains("firefox"));

    // 片付け
    qunsetenv(Constants::YAML_ENV_PATH);
}
