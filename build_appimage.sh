#!/bin/bash
set -e

# Script to build AppImage for KDE Browser Picker
# Requires: linuxdeploy, linuxdeploy-plugin-qt

APP_NAME="kde-browser-picker"
APP_VERSION="1.0.0"
ARCH=$(uname -m)

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building KDE Browser Picker AppImage v${APP_VERSION}${NC}"

# Check dependencies
echo -e "${YELLOW}Checking dependencies...${NC}"
for cmd in cmake make g++ linuxdeploy; do
    if ! command -v $cmd &> /dev/null; then
        echo -e "${RED}Error: $cmd is not installed${NC}"
        exit 1
    fi
done

# Create build directory
BUILD_DIR="build-appimage"
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DENABLE_CLANG_TIDY=OFF \
    -DENABLE_CPPCHECK=OFF \
    -DBUILD_TESTS=OFF

# Build
echo -e "${YELLOW}Building...${NC}"
make -j$(nproc)

# Create AppDir
echo -e "${YELLOW}Creating AppDir...${NC}"
make install DESTDIR=AppDir

# Copy additional files
mkdir -p AppDir/usr/share/metainfo
cat > AppDir/usr/share/metainfo/${APP_NAME}.appdata.xml << EOF
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop">
  <id>io.github.moezakura.${APP_NAME}</id>
  <metadata_license>CC0-1.0</metadata_license>
  <project_license>MIT</project_license>
  <name>KDE Browser Picker</name>
  <summary>Select browser and profile for opening links</summary>
  <description>
    <p>KDE Browser Picker allows you to choose which browser and profile to use when opening links.</p>
    <p>Features:</p>
    <ul>
      <li>Support for Firefox, Chrome, and Chromium</li>
      <li>Automatic profile detection</li>
      <li>Keyboard shortcuts for quick selection</li>
      <li>KDE Plasma integration</li>
    </ul>
  </description>
  <screenshots>
    <screenshot type="default">
      <caption>Main window</caption>
    </screenshot>
  </screenshots>
  <url type="homepage">https://github.com/moezakura/kde-browser-picker</url>
  <url type="bugtracker">https://github.com/moezakura/kde-browser-picker/issues</url>
  <releases>
    <release version="${APP_VERSION}" date="2025-07-04">
      <description>
        <p>Initial release</p>
      </description>
    </release>
  </releases>
</component>
EOF

# Download linuxdeploy if not present
if [ ! -f ../linuxdeploy-${ARCH}.AppImage ]; then
    echo -e "${YELLOW}Downloading linuxdeploy...${NC}"
    wget -c "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${ARCH}.AppImage" -P ..
    chmod +x ../linuxdeploy-${ARCH}.AppImage
fi

# Download Qt plugin if not present
if [ ! -f ../linuxdeploy-plugin-qt-${ARCH}.AppImage ]; then
    echo -e "${YELLOW}Downloading linuxdeploy Qt plugin...${NC}"
    wget -c "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-${ARCH}.AppImage" -P ..
    chmod +x ../linuxdeploy-plugin-qt-${ARCH}.AppImage
fi

# Create AppImage
echo -e "${YELLOW}Creating AppImage...${NC}"
export LDAI_OUTPUT="${APP_NAME}-${APP_VERSION}-${ARCH}.AppImage"
export LDAI_UPDATE_INFORMATION="gh-releases-zsync|moezakura|kde-browser-picker|latest|${APP_NAME}-*-${ARCH}.AppImage.zsync"

../linuxdeploy-${ARCH}.AppImage \
    --appdir AppDir \
    --plugin qt \
    --output appimage \
    --desktop-file=AppDir/usr/share/applications/${APP_NAME}.desktop \
    --icon-file=AppDir/usr/share/icons/hicolor/128x128/apps/${APP_NAME}.png \
    --executable=AppDir/usr/bin/${APP_NAME}

# Move AppImage to parent directory
mv *.AppImage ..

echo -e "${GREEN}Build complete!${NC}"
echo -e "${GREEN}AppImage created: ${APP_NAME}-${APP_VERSION}-${ARCH}.AppImage${NC}"

# Optional: Create zsync file for updates
if command -v zsyncmake &> /dev/null; then
    echo -e "${YELLOW}Creating zsync file for updates...${NC}"
    cd ..
    zsyncmake ${APP_NAME}-${APP_VERSION}-${ARCH}.AppImage
fi

echo -e "${GREEN}Done!${NC}"
