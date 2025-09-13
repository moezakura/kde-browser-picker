/**
 * @file main.cpp
 * @brief アプリケーションのエントリポイント
 *
 * KDE Browser Pickerのメイン関数を定義しています。
 * コマンドライン引数の処理、KDE統合の初期化、
 * アプリケーションの起動を行います。
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QDebug>
#include <KLocalizedString>
#include <KAboutData>

#include "mainwindow.h"
#include "kdeintegration.h"
#include "configmanager.h"
#include "version.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // KDEローカライゼーションの設定
    KLocalizedString::setApplicationDomain("kde-browser-picker");
    
    // アプリケーションメタデータの設定
    KAboutData aboutData("kde-browser-picker",
                        i18n("KDE Browser Picker"),
                        KDE_BROWSER_PICKER_VERSION_STRING,
                        i18n("Select browser and profile for opening links"),
                        KAboutLicense::GPL_V3,
                        i18n("(c) 2025"),
                        QString(),
                        "https://github.com/yourusername/kde-browser-picker");
                        
    aboutData.addAuthor(i18n("Your Name"), i18n("Developer"), "your.email@example.com");
    
    KAboutData::setApplicationData(aboutData);
    
    // アプリケーションプロパティの設定
    app.setApplicationName("kde-browser-picker");
    app.setApplicationDisplayName(i18n("KDE Browser Picker"));
    app.setOrganizationName("KDE");
    app.setOrganizationDomain("kde.org");
    app.setWindowIcon(QIcon::fromTheme("web-browser"));
    
    // コマンドラインパーサーの設定
    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("KDE Browser Profile Picker"));
    parser.addHelpOption();
    parser.addVersionOption();
    
    // URLの位置引数を追加
    parser.addPositionalArgument("url", i18n("URL to open"), "[url]");
    
    // オプションの追加
    QCommandLineOption registerOption("register-default",
                                    i18n("Register as default browser"));
    parser.addOption(registerOption);

    QCommandLineOption settingsOption("settings",
                                    i18n("Show settings dialog"));
    parser.addOption(settingsOption);

    QCommandLineOption deployDefaultsOption("init-defaults",
                                           i18n("Deploy default config files to ~/.config"));
    parser.addOption(deployDefaultsOption);
    QCommandLineOption forceOption("force",
                                   i18n("Force overwrite when used with --init-defaults"));
    parser.addOption(forceOption);
    
    // コマンドラインを処理
    parser.process(app);
    
    // 特殊オプションの処理
    if (parser.isSet(registerOption)) {
        if (KDEIntegration::registerAsDefaultBrowser()) {
            qInfo() << "Successfully registered as default browser";
            return 0;
        } else {
            qCritical() << "Failed to register as default browser";
            return 1;
        }
    }

    if (parser.isSet(deployDefaultsOption)) {
        ConfigManager cfg;
        bool changed = cfg.deployDefaults(parser.isSet(forceOption));
        if (changed) {
            qInfo() << "Default config deployed";
        } else {
            qInfo() << "Nothing to do (defaults already present)";
        }
        return 0;
    }
    
    // コマンドラインからURLを取得
    QString url;
    QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        url = args.first();
        
        // URLにスキームがあることを確認
        if (!url.contains("://")) {
            if (url.startsWith("www.")) {
                url = "https://" + url;
            } else {
                url = "https://www." + url;
            }
        }
    }
    
    // 設定ダイアログが要求された場合の処理
    if (parser.isSet(settingsOption)) {
        // TODO: 設定ダイアログを表示
        qDebug() << "Settings dialog not yet implemented";
        return 0;
    }
    
    // URLが指定されていない場合は使用方法を表示
    if (url.isEmpty()) {
        parser.showHelp(1);
    }
    
    // メインウィンドウの作成と表示
    MainWindow window(url);
    window.show();
    
    return app.exec();
}
