/**
 * @file kdeintegration.cpp
 * @brief KDEIntegrationクラスの実装
 *
 * KDEデスクトップ環境との統合機能を実装しています。
 * システムトレイ、KDE通知、デスクトップファイルの登録などを
 * サポートします。
 */

#include "kdeintegration.h"

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>

#include <KNotification>
#include <KLocalizedString>

KDEIntegration::KDEIntegration(QObject* parent)
    : QObject(parent)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
{
    createTrayIcon();
}

KDEIntegration::~KDEIntegration()
{
    // アクティブな通知をクリーンアップ
    for (KNotification* notification : m_activeNotifications) {
        notification->close();
    }
}

void KDEIntegration::setTrayIconVisible(bool visible)
{
    if (!m_trayIcon) {
        return;
    }
    
    m_trayIcon->setVisible(visible);
}

bool KDEIntegration::isTrayIconVisible() const
{
    return m_trayIcon && m_trayIcon->isVisible();
}

void KDEIntegration::updateTrayMenu()
{
    if (!m_trayMenu) {
        createTrayMenu();
    }
    
    // 必要に応じてメニュー項目を更新
}

void KDEIntegration::showNotification(const QString& title, 
                                     const QString& text,
                                     const QString& iconName)
{
    KNotification* notification = new KNotification("generalNotification", 
                                                  KNotification::CloseOnTimeout,
                                                  this);
    notification->setTitle(title);
    notification->setText(text);
    
    if (!iconName.isEmpty()) {
        notification->setIconName(iconName);
    } else {
        notification->setIconName("web-browser");
    }
    
    // KF5ではKNotification::activatedにパラメータがある
    connect(notification, QOverload<unsigned int>::of(&KNotification::activated),
            this, [this](unsigned int) { onNotificationActivated(); });
            
    connect(notification, &KNotification::closed, notification, [this, notification]() {
        m_activeNotifications.removeOne(notification);
        notification->deleteLater();
    });
    
    m_activeNotifications.append(notification);
    notification->sendEvent();
}

bool KDEIntegration::registerAsDefaultBrowser()
{
    // ユーザーのアプリケーションディレクトリにデスクトップファイルを作成
    QString applicationsDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    QString desktopFile = applicationsDir + "/kde-browser-picker.desktop";
    
    QFile file(desktopFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to create desktop file:" << desktopFile;
        return false;
    }
    
    QTextStream stream(&file);
    stream << "[Desktop Entry]\n";
    stream << "Version=1.0\n";
    stream << "Type=Application\n";
    stream << "Name=KDE Browser Picker\n";
    stream << "GenericName=Browser Profile Selector\n";
    stream << "Comment=Select browser and profile for opening links\n";
    stream << "Exec=" << QCoreApplication::applicationFilePath() << " %u\n";
    stream << "Icon=web-browser\n";
    stream << "Terminal=false\n";
    stream << "Categories=Network;WebBrowser;\n";
    stream << "MimeType=text/html;text/xml;application/xhtml+xml;x-scheme-handler/http;x-scheme-handler/https;\n";
    stream << "StartupNotify=true\n";
    file.close();
    
    // Register as default browser using xdg-settings
    QProcess process;
    process.start("xdg-settings", QStringList() << "set" << "default-web-browser" << "kde-browser-picker.desktop");
    process.waitForFinished();
    
    return process.exitCode() == 0;
}

bool KDEIntegration::isRegisteredAsDefaultBrowser()
{
    QProcess process;
    process.start("xdg-settings", QStringList() << "get" << "default-web-browser");
    process.waitForFinished();
    
    QString output = process.readAllStandardOutput().trimmed();
    return output == "kde-browser-picker.desktop";
}

void KDEIntegration::registerGlobalShortcuts()
{
    // TODO: Implement KDE global shortcuts using KGlobalAccel
    // This would allow users to trigger the browser picker with a global hotkey
}

void KDEIntegration::unregisterGlobalShortcuts()
{
    // TODO: Unregister global shortcuts
}

void KDEIntegration::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        emit trayActivated();
    }
}

void KDEIntegration::onTrayMenuTriggered(QAction* action)
{
    QString command = action->data().toString();
    
    if (command == "settings") {
        emit openSettingsRequested();
    } else if (command == "quit") {
        emit quitRequested();
    }
}

void KDEIntegration::onNotificationActivated()
{
    // Handle notification click
    emit trayActivated();
}

void KDEIntegration::createTrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }
    
    m_trayIcon = new QSystemTrayIcon(QIcon::fromTheme("web-browser"), this);
    m_trayIcon->setToolTip(tr("KDE Browser Picker"));
    
    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &KDEIntegration::onTrayActivated);
            
    createTrayMenu();
    m_trayIcon->setContextMenu(m_trayMenu);
}

void KDEIntegration::createTrayMenu()
{
    m_trayMenu = new QMenu();
    
    QAction* settingsAction = m_trayMenu->addAction(QIcon::fromTheme("configure"), 
                                                   tr("設定(&S)"));
    settingsAction->setData("settings");
    
    m_trayMenu->addSeparator();
    
    QAction* quitAction = m_trayMenu->addAction(QIcon::fromTheme("application-exit"), 
                                               tr("終了(&Q)"));
    quitAction->setData("quit");
    
    connect(m_trayMenu, &QMenu::triggered,
            this, &KDEIntegration::onTrayMenuTriggered);
}