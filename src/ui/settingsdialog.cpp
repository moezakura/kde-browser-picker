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
#include "../kdeintegration.h"
#include "../../include/constants.h"

#include <QMessageBox>

SettingsDialog::SettingsDialog(ConfigManager* configManager, QWidget* parent)
    : QDialog(parent)
    , m_ui(std::make_unique<Ui::SettingsDialog>())
    , m_configManager(configManager)
{
    m_ui->setupUi(this);
    
    // シグナルの接続
    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccepted);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::onRejected);
    
    // 現在の設定を読み込み
    loadSettings();
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::onAccepted()
{
    saveSettings();
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