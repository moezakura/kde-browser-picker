/**
 * @file profileitem.cpp
 * @brief ProfileItemクラスの実装
 *
 * ブラウザプロファイル項目のウィジェットを実装しています。
 * マウスインタラクション、キーボード操作、視覚的フィードバックなどを
 * 提供します。
 */

#include "profileitem.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOption>
#include <QPropertyAnimation>
#include <QPixmap>
#include <QIcon>
#include <QTimer>
#include <QFile>

ProfileItem::ProfileItem(QWidget* parent)
    : QWidget(parent)
    , m_shortcutNumber(0)
    , m_selected(false)
    , m_hovered(false)
    , m_pressed(false)
{
    setupUI();
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
}

void ProfileItem::setProfileData(const QString& browser,
                                const QString& profileId,
                                const QString& profileName,
                                const QString& iconPath,
                                const QDateTime& lastUsed,
                                bool isDefault)
{
    m_browser = browser;
    m_profileId = profileId;
    m_profileName = profileName;
    m_lastUsed = lastUsed;
    m_isDefault = isDefault;
    
    // UIを更新
    if (!iconPath.isEmpty() && QFile::exists(iconPath)) {
        QPixmap pixmap(iconPath);
        m_iconLabel->setPixmap(pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_iconLabel->setPixmap(QIcon::fromTheme("web-browser").pixmap(32, 32));
    }
    
    m_browserLabel->setText(browser);
    
    QString displayName = profileName;
    if (isDefault) {
        displayName += " " + tr("(Default)");  // デフォルトプロファイルの表示
    }
    m_profileLabel->setText(displayName);
    
    m_lastUsedLabel->setText(formatLastUsed(lastUsed));
}

void ProfileItem::setShortcutNumber(int number)
{
    m_shortcutNumber = number;
    if (number > 0 && number <= 9) {
        m_shortcutLabel->setText(QString::number(number));
        m_shortcutLabel->setVisible(true);
    } else {
        m_shortcutLabel->setVisible(false);
    }
}

void ProfileItem::setSelected(bool selected)
{
    if (m_selected != selected) {
        m_selected = selected;
        updateStyle();
        update();
    }
}

void ProfileItem::animateClick()
{
    // キーボード操作時の視覚的フィードバック
    m_pressed = true;
    update();
    QTimer::singleShot(100, this, [this]() {
        m_pressed = false;
        update();
        emit clicked();
    });
}

void ProfileItem::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QStyleOption opt;
    opt.initFrom(this);
    
    // Draw background
    QRect rect = contentsRect();
    qreal radius = 6.0;
    
    if (m_selected) {
        painter.setBrush(palette().highlight());
        painter.setPen(Qt::NoPen);
    } else if (m_pressed) {
        painter.setBrush(palette().highlight().color().darker(120));
        painter.setPen(Qt::NoPen);
    } else if (m_hovered) {
        QColor hoverColor = palette().highlight().color();
        hoverColor.setAlpha(50);
        painter.setBrush(hoverColor);
        painter.setPen(Qt::NoPen);
    } else {
        painter.setBrush(palette().alternateBase());
        painter.setPen(QPen(palette().mid().color(), 1));
    }
    
    painter.drawRoundedRect(rect, radius, radius);
}

void ProfileItem::enterEvent(QEvent* event)
{
    Q_UNUSED(event)
    m_hovered = true;
    update();
}

void ProfileItem::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
    m_hovered = false;
    update();
}

void ProfileItem::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        update();
    }
    QWidget::mousePressEvent(event);
}

void ProfileItem::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_pressed) {
        m_pressed = false;
        update();
        if (rect().contains(event->pos())) {
            emit clicked();
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void ProfileItem::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked();
    }
    QWidget::mouseDoubleClickEvent(event);
}

void ProfileItem::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        animateClick();
    } else if (event->key() == Qt::Key_Space) {
        emit clicked();
    }
    QWidget::keyPressEvent(event);
}

void ProfileItem::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setSpacing(12);
    m_mainLayout->setContentsMargins(12, 8, 12, 8);
    
    // Shortcut number
    m_shortcutLabel = new QLabel(this);
    m_shortcutLabel->setFixedSize(24, 24);
    m_shortcutLabel->setAlignment(Qt::AlignCenter);
    m_shortcutLabel->setStyleSheet(
        "QLabel {"
        "  background-color: palette(highlight);"
        "  color: palette(highlighted-text);"
        "  border-radius: 12px;"
        "  font-weight: bold;"
        "}"
    );
    m_shortcutLabel->hide();
    m_mainLayout->addWidget(m_shortcutLabel);
    
    // Browser icon
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(32, 32);
    m_iconLabel->setScaledContents(true);
    m_mainLayout->addWidget(m_iconLabel);
    
    // Text info
    QVBoxLayout* textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);
    
    QHBoxLayout* topLine = new QHBoxLayout();
    topLine->setSpacing(8);
    
    m_browserLabel = new QLabel(this);
    QFont browserFont = m_browserLabel->font();
    browserFont.setBold(true);
    m_browserLabel->setFont(browserFont);
    topLine->addWidget(m_browserLabel);
    
    m_profileLabel = new QLabel(this);
    topLine->addWidget(m_profileLabel);
    topLine->addStretch();
    
    textLayout->addLayout(topLine);
    
    m_lastUsedLabel = new QLabel(this);
    m_lastUsedLabel->setStyleSheet("QLabel { color: palette(mid); font-size: 9pt; }");
    textLayout->addWidget(m_lastUsedLabel);
    
    m_mainLayout->addLayout(textLayout, 1);
    
    // Settings button
    m_settingsButton = new QPushButton(this);
    m_settingsButton->setIcon(QIcon::fromTheme("configure"));
    m_settingsButton->setToolTip(tr("Profile settings"));
    m_settingsButton->setFlat(true);
    m_settingsButton->setFixedSize(24, 24);
    m_settingsButton->setCursor(Qt::PointingHandCursor);
    m_settingsButton->hide(); // Show on hover
    
    connect(m_settingsButton, &QPushButton::clicked, this, &ProfileItem::settingsClicked);
    
    m_mainLayout->addWidget(m_settingsButton);
    
    setMinimumHeight(60);
}

void ProfileItem::updateStyle()
{
    // Update text colors based on selection state
    QPalette pal = palette();
    
    if (m_selected) {
        m_browserLabel->setPalette(pal);
        m_profileLabel->setPalette(pal);
        
        QPalette lastUsedPal = pal;
        lastUsedPal.setColor(QPalette::WindowText, pal.highlightedText().color());
        m_lastUsedLabel->setPalette(lastUsedPal);
    } else {
        m_browserLabel->setPalette(pal);
        m_profileLabel->setPalette(pal);
        m_lastUsedLabel->setPalette(pal);
    }
}

QString ProfileItem::formatLastUsed(const QDateTime& lastUsed) const
{
    if (!lastUsed.isValid()) {
        return tr("Never used");
    }
    
    QDateTime now = QDateTime::currentDateTime();
    qint64 daysAgo = lastUsed.daysTo(now);
    
    if (daysAgo == 0) {
        return tr("Used today");
    } else if (daysAgo == 1) {
        return tr("Used yesterday");
    } else if (daysAgo < 7) {
        return tr("Used %1 days ago").arg(daysAgo);
    } else if (daysAgo < 30) {
        return tr("Used %1 weeks ago").arg(daysAgo / 7);
    } else {
        return tr("Last used: %1").arg(lastUsed.toString("yyyy-MM-dd"));
    }
}