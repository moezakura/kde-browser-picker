/**
 * @file profileitem.h
 * @brief ブラウザプロファイル項目のウィジェット
 *
 * このファイルは、ブラウザ選択ダイアログ内の各プロファイル項目を
 * 表示するカスタムウィジェットを定義します。
 * ブラウザアイコン、プロファイル名、最終使用日時などを表示し、
 * ユーザーのインタラクションを処理します。
 */

#ifndef PROFILEITEM_H
#define PROFILEITEM_H

#include <QWidget>
#include <QDateTime>

// Forward declarations
class QLabel;
class QPushButton;
class QHBoxLayout;

/**
 * @class ProfileItem
 * @brief プロファイル項目ウィジェット
 *
 * メインウィンドウのプロファイルリスト内の各項目を表示する
 * カスタムウィジェットです。
 *
 * 機能:
 * - ブラウザアイコンとプロファイル情報の表示
 * - 数字キーショートカット(1-9)の表示
 * - 選択状態とホバーエフェクト
 * - クリック、ダブルクリック、設定ボタンのイベント処理
 */
class ProfileItem : public QWidget {
    Q_OBJECT

public:
    explicit ProfileItem(QWidget* parent = nullptr);
    ~ProfileItem() override = default;
    
    // プロファイルデータの設定
    /**
     * @brief プロファイル情報を設定
     * @param browser ブラウザID
     * @param profileId プロファイルID
     * @param profileName プロファイル表示名
     * @param iconPath アイコンファイルのパス
     * @param lastUsed 最終使用日時
     * @param isDefault デフォルトプロファイルかどうか
     */
    void setProfileData(const QString& browser,
                       const QString& profileId,
                       const QString& profileName,
                       const QString& iconPath,
                       const QDateTime& lastUsed,
                       bool isDefault);
                       
    /**
     * @brief 数字ショートカットを設定
     * @param number ショートカット番号（1-9）
     */
    void setShortcutNumber(int number);
    
    // 選択状態
    /**
     * @brief 選択状態を設定
     * @param selected true: 選択中, false: 非選択
     */
    void setSelected(bool selected);
    
    /**
     * @brief 選択状態を取得
     * @return true: 選択中, false: 非選択
     */
    bool isSelected() const { return m_selected; }
    
    // プロファイル識別子の取得
    /**
     * @brief ブラウザIDを取得
     */
    QString browser() const { return m_browser; }
    
    /**
     * @brief プロファイルIDを取得
     */
    QString profileId() const { return m_profileId; }
    
    /**
     * @brief プロファイル表示名を取得
     */
    QString profileName() const { return m_profileName; }
    
    // 視覚的フィードバック
    /**
     * @brief クリックアニメーションを実行
     */
    void animateClick();

signals:
    /**
     * @brief 項目がクリックされたときに発行されるシグナル
     */
    void clicked();
    
    /**
     * @brief 項目がダブルクリックされたときに発行されるシグナル
     */
    void doubleClicked();
    
    /**
     * @brief 設定ボタンがクリックされたときに発行されるシグナル
     */
    void settingsClicked();

protected:
    /**
     * @brief ペイントイベントの処理
     * @note 選択状態やホバー状態に応じた描画を行う
     */
    void paintEvent(QPaintEvent* event) override;
    
    /**
     * @brief マウスエンターイベント
     */
    void enterEvent(QEvent* event) override;
    
    /**
     * @brief マウスリーブイベント
     */
    void leaveEvent(QEvent* event) override;
    
    /**
     * @brief マウスプレスイベント
     */
    void mousePressEvent(QMouseEvent* event) override;
    
    /**
     * @brief マウスリリースイベント
     */
    void mouseReleaseEvent(QMouseEvent* event) override;
    
    /**
     * @brief マウスダブルクリックイベント
     */
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    
    /**
     * @brief キープレスイベント
     * @note EnterキーやSpaceキーでの選択を処理
     */
    void keyPressEvent(QKeyEvent* event) override;

private:
    /**
     * @brief UIの初期化
     */
    void setupUI();
    
    /**
     * @brief 状態に応じたスタイルを更新
     */
    void updateStyle();
    
    /**
     * @brief 最終使用日時をフォーマット
     * @param lastUsed 最終使用日時
     * @return フォーマットされた文字列
     */
    QString formatLastUsed(const QDateTime& lastUsed) const;
    
    // UI要素
    QLabel* m_iconLabel;          ///< ブラウザアイコン
    QLabel* m_browserLabel;       ///< ブラウザ名
    QLabel* m_profileLabel;       ///< プロファイル名
    QLabel* m_lastUsedLabel;      ///< 最終使用日時
    QLabel* m_shortcutLabel;      ///< ショートカット番号
    QPushButton* m_settingsButton; ///< 設定ボタン
    QHBoxLayout* m_mainLayout;    ///< メインレイアウト
    
    // データ
    QString m_browser;            ///< ブラウザID
    QString m_profileId;          ///< プロファイルID
    QString m_profileName;        ///< プロファイル表示名
    QDateTime m_lastUsed;         ///< 最終使用日時
    bool m_isDefault;             ///< デフォルトプロファイルかどうか
    int m_shortcutNumber;         ///< ショートカット番号
    
    // 状態
    bool m_selected;              ///< 選択状態
    bool m_hovered;               ///< ホバー状態
    bool m_pressed;               ///< プレス状態
};

#endif // PROFILEITEM_H