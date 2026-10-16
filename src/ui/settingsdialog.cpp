/**
 * @file settingsdialog.cpp
 * @brief SettingsDialogクラスの実装
 *
 * アプリケーション設定ダイアログの機能を実装しています。
 * 設定の読み込み、保存、UIの更新などを処理します。
 */

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "../configmanager.h"
#include "../profilemanager.h"
#include "../kdeintegration.h"
#include "../../include/constants.h"

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>

namespace {
constexpr int ProfileBrowserRole = Qt::UserRole;        ///< 項目にbrowserを保持するロール
constexpr int ProfileIdRole = Qt::UserRole + 1;         ///< 項目にprofileIdを保持するロール
}

SettingsDialog::SettingsDialog(ConfigManager* configManager, ProfileManager* profileManager, QWidget* parent)
    : QDialog(parent)
    , m_ui(std::make_unique<Ui::SettingsDialog>())
    , m_configManager(configManager)
    , m_profileManager(profileManager)
{
    m_ui->setupUi(this);
    
    // シグナルの接続
    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccepted);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::onRejected);
    
    // 現在の設定を読み込み
    loadSettings();

    // プロファイル一覧の挙動を明示（内部移動のみ、外部ドロップ不可）
    QListWidget* list = m_ui->profileListWidget;
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    list->setDragEnabled(true);
    list->setDragDropMode(QAbstractItemView::InternalMove);
    list->setDefaultDropAction(Qt::MoveAction);
    list->setDropIndicatorShown(true);

    if (m_profileManager == nullptr) {
        return;
    }

    populateProfileList();
    connect(list, &QListWidget::itemChanged, this, &SettingsDialog::updateOkButtonState);
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::onAccepted()
{
    saveSettings();
    saveProfilesTab();
    accept();
}

void SettingsDialog::onRejected()
{
    reject();
}

void SettingsDialog::onDefaultTimeoutChanged(int value)
{
    // 現在の値を表示するためラベルを更新
    // m_ui->timeoutValueLabel->setText(tr("%1 seconds").arg(value));
}

void SettingsDialog::onRegisterAsDefaultClicked()
{
    if (KDEIntegration::registerAsDefaultBrowser()) {
        QMessageBox::information(this, tr("Success"), 
                               tr("Successfully registered as default browser"));
        updateDefaultBrowserStatus();
    } else {
        QMessageBox::critical(this, tr("Error"), 
                            tr("Failed to register as default browser"));
    }
}

void SettingsDialog::loadSettings()
{
    // TODO: ConfigManagerから設定を読み込み
    // これはスタブ実装です
}

void SettingsDialog::saveSettings()
{
    // TODO: ConfigManagerに設定を保存
    // これはスタブ実装です
}

void SettingsDialog::updateDefaultBrowserStatus()
{
    // TODO: デフォルトブラウザの状態をUIに表示
    // これはスタブ実装です
}

void SettingsDialog::populateProfileList()
{
    QListWidget* list = m_ui->profileListWidget;
    {
        QSignalBlocker blocker(list);
        list->clear();

        const QList<ProfileManager::ProfileEntry> profiles = m_profileManager->getAllProfiles(false);
        for (const ProfileManager::ProfileEntry& entry : profiles) {
            auto* item = new QListWidgetItem(
                QStringLiteral("%1 / %2").arg(entry.browserDisplayName, entry.profileDisplayName), list);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable
                           | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable);
            item->setCheckState(entry.isEnabled ? Qt::Checked : Qt::Unchecked);
            item->setData(ProfileBrowserRole, entry.browser);
            item->setData(ProfileIdRole, entry.profileId);
        }
    }
    updateOkButtonState();
}

void SettingsDialog::updateOkButtonState()
{
    QListWidget* list = m_ui->profileListWidget;
    int checked = 0;
    for (int i = 0; i < list->count(); ++i) {
        if (list->item(i)->checkState() == Qt::Checked) {
            ++checked;
        }
    }

    // 一覧が空でなく、表示ONが0件のときのみOKを無効化
    const bool blocked = list->count() > 0 && checked == 0;
    if (QPushButton* okButton = m_ui->buttonBox->button(QDialogButtonBox::Ok)) {
        okButton->setEnabled(!blocked);
    }
    m_ui->profilesWarningLabel->setVisible(blocked);
    // 検出0件のときは理由を表示（OKは有効のまま）
    m_ui->profilesEmptyLabel->setVisible(list->count() == 0);
}

void SettingsDialog::saveProfilesTab()
{
    if (m_profileManager == nullptr) {
        return;
    }

    QListWidget* list = m_ui->profileListWidget;
    if (list->count() == 0) {
        return;
    }

    QList<ProfileSetting> settings;
    settings.reserve(list->count());
    for (int i = 0; i < list->count(); ++i) {
        const QListWidgetItem* item = list->item(i);
        ProfileSetting setting;
        setting.browser = item->data(ProfileBrowserRole).toString();
        setting.profileId = item->data(ProfileIdRole).toString();
        setting.order = i;
        setting.enabled = (item->checkState() == Qt::Checked);
        settings.append(setting);
    }
    m_profileManager->applyProfileSettings(settings);
}
