# KDE Browser Picker

KDE環境でリンクを開く際に、ブラウザとプロファイルを選択できるGUIツール。

## 機能

- **マルチブラウザ対応**: Firefox、Google Chrome、Chromiumをサポート
- **プロファイル自動検出**: 各ブラウザのプロファイルを自動的に検出
- **KDE統合**: KDE Plasmaのルック&フィールに完全統合
- **キーボードショートカット**: 数字キー（1-9）で素早くプロファイル選択
- **自動選択**: 設定可能なタイムアウトで最後に使用したプロファイルを自動選択
- **システムトレイ対応**: バックグラウンドで動作（オプション）

## ビルド要件

### ArchLinux
```bash
sudo pacman -S base-devel cmake qt6-base qt6-tools \
               extra-cmake-modules kf6-kconfig kf6-knotifications kf6-ki18n \
               gcc
```

### ビルド手順
```bash
# リポジトリをクローン
git clone https://github.com/yourusername/kde-browser-picker.git
cd kde-browser-picker

# ビルドディレクトリを作成
mkdir build && cd build

# CMakeで設定
cmake .. -DCMAKE_BUILD_TYPE=Release

# ビルド
make -j$(nproc)

# インストール（オプション）
sudo make install
```

## AppImage作成

```bash
# AppImage作成スクリプトを実行
./build_appimage.sh
```

## 使用方法

### デフォルトブラウザとして設定
```bash
kde-browser-picker --register-default
```

### 直接実行
```bash
kde-browser-picker https://example.com
```

### キーボードショートカット
- `1-9`: 対応する番号のプロファイルを選択して開く
- `↑/↓`: プロファイル選択を移動
- `Enter`: 選択したプロファイルで開く
- `Escape`: キャンセル
- `Alt+S`: 設定ダイアログを開く

## 設定

設定は `~/.config/kde-browser-pickerrc` に保存されます。

### 設定可能な項目
- 自動選択タイムアウト（0-60秒）
- 最後に使用したプロファイルの記憶
- システムトレイアイコンの表示
- プロファイルごとの有効/無効設定
- プロファイルの表示名カスタマイズ
- プロファイルの表示順序

## 技術仕様

- **言語**: C++20
- **GUIフレームワーク**: Qt6 (6.5+)
- **KDE統合**: KDE Frameworks 6
- **ビルドシステム**: CMake 3.25+

## ライセンス

GPL v3

## 貢献

プルリクエストを歓迎します。大きな変更を行う場合は、まずissueを開いて変更内容について議論してください。

## バグ報告

バグを見つけた場合は、[GitHub Issues](https://github.com/yourusername/kde-browser-picker/issues)で報告してください。