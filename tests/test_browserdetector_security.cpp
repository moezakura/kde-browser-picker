/**
 * @file test_browserdetector_security.cpp
 * @brief BrowserDetectorクラスのセキュリティテスト
 * 
 * このテストファイルは、ブラウザピッカーアプリケーションの
 * セキュリティを確保するためのテストを含んでいます。
 * 特にURLやプロファイル名に対するインジェクション攻撃への
 * 対策が正しく機能しているかを検証します。
 */

#include <gtest/gtest.h>
#include "../src/browserdetector.h"

/**
 * @brief BrowserDetectorセキュリティテストクラス
 * 
 * Google Testフレームワークを使用したテストフィクスチャ
 */
class BrowserDetectorSecurityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // テスト前の初期化処理
    }
    
    void TearDown() override {
        // テスト後のクリーンアップ処理
    }
};

/**
 * @brief 正しいURLの検証テスト
 * 
 * 一般的な正しいURLパターンが受け入れられることを確認します。
 */
TEST_F(BrowserDetectorSecurityTest, ValidUrls)
{
    // 正しいURLのテスト
    EXPECT_TRUE(BrowserDetector::isValidUrl("https://www.example.com"));
    EXPECT_TRUE(BrowserDetector::isValidUrl("http://example.com"));
    EXPECT_TRUE(BrowserDetector::isValidUrl("https://example.com/path/to/page"));
    EXPECT_TRUE(BrowserDetector::isValidUrl("https://example.com?query=value"));
    EXPECT_TRUE(BrowserDetector::isValidUrl("file:///home/user/document.html"));
    EXPECT_TRUE(BrowserDetector::isValidUrl("about:blank"));
    EXPECT_TRUE(BrowserDetector::isValidUrl("chrome://settings"));
    EXPECT_TRUE(BrowserDetector::isValidUrl("edge://settings"));
    EXPECT_TRUE(BrowserDetector::isValidUrl("www.example.com")); // No scheme
    EXPECT_TRUE(BrowserDetector::isValidUrl("example.com")); // No scheme
}

/**
 * @brief 不正なURLの検証テスト
 * 
 * コマンドインジェクション、XSS攻撃、その他の悪意ある
 * パターンを含むURLが拒否されることを確認します。
 */
TEST_F(BrowserDetectorSecurityTest, InvalidUrls)
{
    // インジェクション攻撃を含むURLのテスト
    EXPECT_FALSE(BrowserDetector::isValidUrl("https://example.com; rm -rf /"));
    EXPECT_FALSE(BrowserDetector::isValidUrl("https://example.com && echo hack"));
    EXPECT_FALSE(BrowserDetector::isValidUrl("https://example.com | cat /etc/passwd"));
    EXPECT_FALSE(BrowserDetector::isValidUrl("https://example.com`whoami`"));
    EXPECT_FALSE(BrowserDetector::isValidUrl("https://example.com$(whoami)"));
    EXPECT_FALSE(BrowserDetector::isValidUrl("javascript:alert('xss')"));
    EXPECT_FALSE(BrowserDetector::isValidUrl("data:text/html,<script>alert('xss')</script>"));
    EXPECT_FALSE(BrowserDetector::isValidUrl("")); // Empty URL
    EXPECT_FALSE(BrowserDetector::isValidUrl("https://example.com{malicious}"));
    EXPECT_FALSE(BrowserDetector::isValidUrl("https://example.com[malicious]"));
    EXPECT_FALSE(BrowserDetector::isValidUrl("https://example.com<script>"));
    EXPECT_FALSE(BrowserDetector::isValidUrl("https://example.com>redirect"));
}

/**
 * @brief 正しいプロファイル名の検証テスト
 * 
 * 一般的なプロファイル名が受け入れられることを確認します。
 */
TEST_F(BrowserDetectorSecurityTest, ValidProfileNames)
{
    // 正しいプロファイル名のテスト
    EXPECT_TRUE(BrowserDetector::isValidProfileName("Default"));
    EXPECT_TRUE(BrowserDetector::isValidProfileName("Profile 1"));
    EXPECT_TRUE(BrowserDetector::isValidProfileName("Work_Profile"));
    EXPECT_TRUE(BrowserDetector::isValidProfileName("my-profile"));
    EXPECT_TRUE(BrowserDetector::isValidProfileName("user.profile"));
    EXPECT_TRUE(BrowserDetector::isValidProfileName("Profile123"));
}

/**
 * @brief 不正なプロファイル名の検証テスト
 * 
 * コマンドインジェクションやパストラバーサル攻撃を
 * 含むプロファイル名が拒否されることを確認します。
 */
TEST_F(BrowserDetectorSecurityTest, InvalidProfileNames)
{
    // インジェクション攻撃を含むプロファイル名のテスト
    EXPECT_FALSE(BrowserDetector::isValidProfileName("Profile; rm -rf /"));
    EXPECT_FALSE(BrowserDetector::isValidProfileName("Profile && echo hack"));
    EXPECT_FALSE(BrowserDetector::isValidProfileName("Profile | cat /etc/passwd"));
    EXPECT_FALSE(BrowserDetector::isValidProfileName("Profile`whoami`"));
    EXPECT_FALSE(BrowserDetector::isValidProfileName("Profile$(whoami)"));
    EXPECT_FALSE(BrowserDetector::isValidProfileName("")); // Empty profile
    EXPECT_FALSE(BrowserDetector::isValidProfileName("Profile{malicious}"));
    EXPECT_FALSE(BrowserDetector::isValidProfileName("Profile[malicious]"));
    EXPECT_FALSE(BrowserDetector::isValidProfileName("Profile<script>"));
    EXPECT_FALSE(BrowserDetector::isValidProfileName("Profile>redirect"));
    EXPECT_FALSE(BrowserDetector::isValidProfileName("Profile\\nNewline"));
    EXPECT_FALSE(BrowserDetector::isValidProfileName("Profile/../../etc"));
}

/**
 * @brief URLサニタイズ機能のテスト
 * 
 * URLから不要な文字や潜在的に危険な文字が
 * 正しく除去されることを確認します。
 */
TEST_F(BrowserDetectorSecurityTest, UrlSanitization)
{
    // URLサニタイズのテスト
    EXPECT_EQ(BrowserDetector::sanitizeUrl("  https://example.com  "), "https://example.com");
    QString nullByteUrl = "https://example.com";
    nullByteUrl.append(QChar('\0'));
    nullByteUrl.append("malicious");
    EXPECT_EQ(BrowserDetector::sanitizeUrl(nullByteUrl), "https://example.commalicious");
    EXPECT_EQ(BrowserDetector::sanitizeUrl("\nhttps://example.com\n"), "https://example.com");
}

/**
 * @brief プロファイル名サニタイズ機能のテスト
 * 
 * プロファイル名から特殊文字や危険な文字が
 * 正しく除去されることを確認します。
 */
TEST_F(BrowserDetectorSecurityTest, ProfileNameSanitization)
{
    // プロファイル名サニタイズのテスト
    EXPECT_EQ(BrowserDetector::sanitizeProfileName("  Default  "), "Default");
    QString nullByteProfile = "Profile";
    nullByteProfile.append(QChar('\0'));
    nullByteProfile.append("malicious");
    EXPECT_EQ(BrowserDetector::sanitizeProfileName(nullByteProfile), "Profilemalicious");
    EXPECT_EQ(BrowserDetector::sanitizeProfileName("Profile!@#$%^&*()"), "Profile");
    EXPECT_EQ(BrowserDetector::sanitizeProfileName("My Profile 123"), "My Profile 123");
    EXPECT_EQ(BrowserDetector::sanitizeProfileName("Profile;injection"), "Profileinjection");
}

