/**
 * @file mainwindow.cpp
 * @brief MainWindowクラスの実装
 *
 * ブラウザ選択ダイアログのメインロジックを実装しています。
 * キーボードショートカット、プロファイル管理、タイムアウト処理などを含みます。
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "profilemanager.h"
#include "configmanager.h"
#include "ui/profileitem.h"
#include "constants.h"

#include <QKeyEvent>
#include <QShortcut>
#include <QDebug>
#include <QUrl>
#include <QMessageBox>

MainWindow::MainWindow(const QString& url, QWidget* parent)
    : QDialog(parent)
    , m_ui(std::make_unique<Ui::MainWindow>())
    , m_profileManager(nullptr)
    , m_configManager(std::make_unique<ConfigManager>(this))
    , m_url(url)
    , m_selectedItem(nullptr)
{
    m_ui->setupUi(this);
    
    // ConfigManagerの後にProfileManagerを初期化
    m_profileManager = std::make_unique<ProfileManager>(m_configManager.get(), this);
    
    setupUI();
    setupShortcuts();
    
    // シグナルの接続
    connect(m_ui->openButton, &QPushButton::clicked, this, &MainWindow::onOpenClicked);
    connect(m_ui->cancelButton, &QPushButton::clicked, this, &MainWindow::onCancelClicked);
    connect(m_ui->settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    
    connect(m_profileManager.get(), &ProfileManager::profilesRefreshed,
            this, &MainWindow::onProfilesRefreshed);
    connect(m_configManager.get(), &ConfigManager::configChanged,
            this, &MainWindow::onConfigChanged);
            
    // タイマーの設定
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &MainWindow::onTimeout);
    
    m_tickTimer = new QTimer(this);
    m_tickTimer->setInterval(1000); // 1秒ごとに更新
    connect(m_tickTimer, &QTimer::timeout, this, &MainWindow::onTimeoutTick);
    
    // プロファイルの読み込み
    loadProfiles();
    
    // 設定されていればタイムアウトを開始
    m_remainingSeconds = m_configManager->defaultTimeout();
    if (m_remainingSeconds > 0) {
        m_timeoutTimer->start(m_remainingSeconds * 1000);
        m_tickTimer->start();
        updateTimeoutLabel();
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    // 検索フィールドにフォーカスがあり、空でない場合は数字キー処理をスキップ
    bool searchHasFocus = m_ui->searchLineEdit->hasFocus();
    bool searchNotEmpty = !m_ui->searchLineEdit->text().isEmpty();
    
    // 数字キーによるクイックプロファイル選択の処理
    if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9 && 
        !(searchHasFocus && searchNotEmpty)) {
        int number = event->key() - Qt::Key_0;
        selectProfileByNumber(number);
        return;
    }
    
    // ナビゲーションキーの処理
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
        // 必要に応じて検索からプロファイルにフォーカスを移動
        if (searchHasFocus && event->key() == Qt::Key_Down) {
            if (!m_profileItems.isEmpty() && m_selectedItem) {
                m_selectedItem->setFocus();
            }
            return;
        }
        
        // 表示中のアイテムから現在の選択インデックスを検索
        QList<ProfileItem*> visibleItems;
        for (ProfileItem* item : m_profileItems) {
            if (item->isVisible()) {
                visibleItems.append(item);
            }
        }
        
        int currentIndex = -1;
        for (int i = 0; i < visibleItems.size(); ++i) {
            if (visibleItems[i] == m_selectedItem) {
                currentIndex = i;
                break;
            }
        }
        
        int newIndex = currentIndex;
        if (event->key() == Qt::Key_Up && currentIndex > 0) {
            newIndex = currentIndex - 1;
        } else if (event->key() == Qt::Key_Down && currentIndex < visibleItems.size() - 1) {
            newIndex = currentIndex + 1;
        }
        
        if (newIndex != currentIndex && newIndex >= 0 && newIndex < visibleItems.size()) {
            selectProfile(visibleItems[newIndex]);
        }
        return;
    }
    
    // Handle Enter key
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (m_selectedItem && m_ui->openButton->isEnabled()) {
            openSelectedProfile();
            return;
        }
    }
    
    // Ctrl+F to focus search
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_F) {
        m_ui->searchLineEdit->setFocus();
        m_ui->searchLineEdit->selectAll();
        return;
    }
    
    QDialog::keyPressEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    saveWindowGeometry();
    QDialog::closeEvent(event);
}

void MainWindow::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    restoreWindowGeometry();
    
    // Focus on first profile if available
    if (!m_profileItems.isEmpty()) {
        selectProfile(m_profileItems.first());
    }
    
    // Raise and activate the window
    raise();
    activateWindow();
    
    // Set focus to search line edit
    m_ui->searchLineEdit->setFocus();
}

void MainWindow::onProfileClicked()
{
    ProfileItem* item = qobject_cast<ProfileItem*>(sender());
    if (item) {
        selectProfile(item);
    }
}

void MainWindow::onProfileDoubleClicked()
{
    ProfileItem* item = qobject_cast<ProfileItem*>(sender());
    if (item) {
        selectProfile(item);
        openSelectedProfile();
    }
}

void MainWindow::onProfileSettingsClicked()
{
    ProfileItem* item = qobject_cast<ProfileItem*>(sender());
    if (item) {
        // TODO: Show profile settings dialog
        qDebug() << "Settings for" << item->browser() << item->profileId();
    }
}

void MainWindow::onSettingsClicked()
{
    // TODO: Show main settings dialog
    qDebug() << "Show settings dialog";
}

void MainWindow::onOpenClicked()
{
    openSelectedProfile();
}

void MainWindow::onCancelClicked()
{
    reject();
}

void MainWindow::onTimeout()
{
    // Auto-select default profile
    auto defaultProfile = m_profileManager->getDefaultProfile();
    if (!defaultProfile.browser.isEmpty()) {
        m_profileManager->launchProfile(defaultProfile, m_url);
        accept();
    } else {
        // No default profile, just close
        reject();
    }
}

void MainWindow::onTimeoutTick()
{
    m_remainingSeconds--;
    updateTimeoutLabel();
    
    if (m_remainingSeconds <= 0) {
        m_tickTimer->stop();
    }
}

void MainWindow::onProfilesRefreshed()
{
    // Clear existing items
    for (ProfileItem* item : m_profileItems) {
        m_ui->profilesLayout->removeWidget(item);
        item->deleteLater();
    }
    m_profileItems.clear();
    m_selectedItem = nullptr;
    
    // Get enabled profiles
    auto profiles = m_profileManager->getAllProfiles(true);
    
    // Create profile items
    int shortcutNumber = 1;
    for (const auto& profile : profiles) {
        ProfileItem* item = new ProfileItem(this);
        item->setProfileData(profile.browser,
                           profile.profileId,
                           profile.profileDisplayName,
                           profile.iconPath,
                           profile.lastUsed,
                           profile.isDefault);
                           
        if (shortcutNumber <= 9) {
            item->setShortcutNumber(shortcutNumber++);
        }
        
        connect(item, &ProfileItem::clicked, this, &MainWindow::onProfileClicked);
        connect(item, &ProfileItem::doubleClicked, this, &MainWindow::onProfileDoubleClicked);
        connect(item, &ProfileItem::settingsClicked, this, &MainWindow::onProfileSettingsClicked);
        
        m_ui->profilesLayout->addWidget(item);
        m_profileItems.append(item);
    }
    
    // Add stretch at the end
    m_ui->profilesLayout->addStretch();
    
    // Select default profile
    auto defaultProfile = m_profileManager->getDefaultProfile();
    if (!defaultProfile.browser.isEmpty()) {
        for (ProfileItem* item : m_profileItems) {
            if (item->browser() == defaultProfile.browser && 
                item->profileId() == defaultProfile.profileId) {
                selectProfile(item);
                break;
            }
        }
    } else if (!m_profileItems.isEmpty()) {
        selectProfile(m_profileItems.first());
    }
}

void MainWindow::onConfigChanged()
{
    // Update timeout
    int newTimeout = m_configManager->defaultTimeout();
    if (newTimeout != m_remainingSeconds) {
        m_remainingSeconds = newTimeout;
        
        if (m_remainingSeconds > 0) {
            m_timeoutTimer->start(m_remainingSeconds * 1000);
            m_tickTimer->start();
        } else {
            m_timeoutTimer->stop();
            m_tickTimer->stop();
        }
        
        updateTimeoutLabel();
    }
}

void MainWindow::setupUI()
{
    // Set window properties
    setWindowTitle(tr("ブラウザプロファイル選択"));
    setWindowIcon(QIcon::fromTheme("web-browser"));
    
    // Set URL
    m_ui->urlDisplayLabel->setText(truncateUrl(m_url));
    m_ui->urlDisplayLabel->setToolTip(m_url);
    
    // Set icon
    m_ui->urlIconLabel->setPixmap(QIcon::fromTheme("text-html").pixmap(16, 16));
    
    // Connect search line edit
    connect(m_ui->searchLineEdit, &QLineEdit::textChanged, 
            this, &MainWindow::onSearchTextChanged);
    
    // Disable open button initially
    m_ui->openButton->setEnabled(false);
}

void MainWindow::setupShortcuts()
{
    // Escape to cancel
    QShortcut* escapeShortcut = new QShortcut(Qt::Key_Escape, this);
    connect(escapeShortcut, &QShortcut::activated, this, &MainWindow::reject);
    
    // Alt+S for settings
    QShortcut* settingsShortcut = new QShortcut(QKeySequence("Alt+S"), this);
    connect(settingsShortcut, &QShortcut::activated, this, &MainWindow::onSettingsClicked);
}

void MainWindow::loadProfiles()
{
    m_profileManager->refreshProfiles();
}

void MainWindow::selectProfileByNumber(int number)
{
    if (number > 0 && number <= m_profileItems.size()) {
        selectProfile(m_profileItems[number - 1]);
        // Also open it immediately
        openSelectedProfile();
    }
}

void MainWindow::selectProfile(ProfileItem* item)
{
    if (m_selectedItem == item) {
        return;
    }
    
    // Deselect previous
    if (m_selectedItem) {
        m_selectedItem->setSelected(false);
    }
    
    // Select new
    m_selectedItem = item;
    if (m_selectedItem) {
        m_selectedItem->setSelected(true);
        m_selectedItem->setFocus();
        m_ui->openButton->setEnabled(true);
        
        // Stop timeout when user selects a profile
        m_timeoutTimer->stop();
        m_tickTimer->stop();
        m_ui->timeoutLabel->hide();
    } else {
        m_ui->openButton->setEnabled(false);
    }
}

void MainWindow::openSelectedProfile()
{
    if (!m_selectedItem) {
        return;
    }
    
    bool success = m_profileManager->launchProfile(
        m_selectedItem->browser(),
        m_selectedItem->profileId(),
        m_url
    );
    
    if (success) {
        accept();
    } else {
        QMessageBox::critical(this, tr("エラー"), 
                            tr("ブラウザの起動に失敗しました。"));
    }
}

void MainWindow::updateTimeoutLabel()
{
    if (m_remainingSeconds > 0) {
        m_ui->timeoutLabel->setText(tr("自動選択: %1秒").arg(m_remainingSeconds));
        m_ui->timeoutLabel->show();
    } else {
        m_ui->timeoutLabel->hide();
    }
}

void MainWindow::saveWindowGeometry()
{
    m_configManager->setWindowGeometry(saveGeometry());
}

void MainWindow::restoreWindowGeometry()
{
    QByteArray geometry = m_configManager->windowGeometry();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    } else {
        // Default size
        resize(Constants::DEFAULT_WINDOW_WIDTH, Constants::DEFAULT_WINDOW_HEIGHT);
    }
}

QString MainWindow::truncateUrl(const QString& url) const
{
    if (url.length() <= Constants::MAX_URL_DISPLAY_LENGTH) {
        return url;
    }
    
    QUrl qurl(url);
    QString display = qurl.scheme() + "://" + qurl.host();
    
    if (display.length() < url.length()) {
        display += "/...";
    }
    
    return display;
}

void MainWindow::onSearchTextChanged(const QString& text)
{
    const QString searchText = text.trimmed().toLower();
    
    // Filter profile items based on search text
    for (ProfileItem* item : m_profileItems) {
        bool visible = searchText.isEmpty() || 
                      item->browser().toLower().contains(searchText) ||
                      item->profileName().toLower().contains(searchText);
        item->setVisible(visible);
    }
    
    // If current selection is hidden, select the first visible item
    if (m_selectedItem && !m_selectedItem->isVisible()) {
        ProfileItem* firstVisible = nullptr;
        for (ProfileItem* item : m_profileItems) {
            if (item->isVisible()) {
                firstVisible = item;
                break;
            }
        }
        if (firstVisible) {
            selectProfile(firstVisible);
        } else {
            m_selectedItem = nullptr;
            m_ui->openButton->setEnabled(false);
        }
    }
}