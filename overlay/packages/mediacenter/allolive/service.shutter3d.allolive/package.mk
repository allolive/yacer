# SPDX-License-Identifier: GPL-2.0-or-later

PKG_NAME="service.shutter3d.allolive"
PKG_VERSION="0.1.15"
PKG_SHA256="cd9270001ad97b2df7c3e335216a4582c41409b641787c32b97385d814493069"
PKG_LICENSE="PolyForm-Noncommercial-1.0.0"
PKG_SITE="https://github.com/allolive/kodi_addons"
PKG_URL="https://github.com/allolive/kodi_addons/releases/download/ce22/${PKG_NAME}-${PKG_VERSION}/${PKG_NAME}-${PKG_VERSION}.zip"
PKG_SOURCE_DIR="${PKG_NAME}"
PKG_DEPENDS_TARGET="toolchain"
PKG_LONGDESC="Active 3D glasses for 120 Hz VRR displays."
PKG_TOOLCHAIN="manual"

# the add-on as the allolive repository publishes it; a newer one from there
# installs into the user's add-ons and takes over
makeinstall_target() {
  mkdir -p ${INSTALL}/usr/share/kodi/addons
  cp -a ${PKG_BUILD} ${INSTALL}/usr/share/kodi/addons/${PKG_NAME}
}
