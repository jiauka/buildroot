#!/bin/bash

set -u
set -e

echo "Post-build: processing $@"

BOARD_DIR="$(dirname $0)"

# Add Gstreamer software based AC3 Decoder
if [ -f "${BOARD_DIR}/libgstfluac3dec.so" ]; then
	mkdir -p "${TARGET_DIR}/usr/lib/gstreamer-1.0/"
	cp -pf "${BOARD_DIR}/libgstfluac3dec.so" "${TARGET_DIR}/usr/lib/gstreamer-1.0/"
fi

# Copy index.html page for boot up
if [ -f "${BOARD_DIR}/index.html" ]; then
	mkdir -p "${TARGET_DIR}/www/"
	cp -pf "${BOARD_DIR}/index.html" "${TARGET_DIR}/www/"
fi

# Copy config.json to webserver
if [ -f "${BOARD_DIR}/config.json" ]; then
	mkdir -p "${TARGET_DIR}/www/"
	cp -pf "${BOARD_DIR}/config.json" "${TARGET_DIR}/www/"
fi

# Copy keymap for OSMC remote
if [ -f "${BOARD_DIR}/osmc-devinput-remote.json" ]; then
	mkdir -p "${TARGET_DIR}/usr/share/WPEFramework/RemoteControl/"
	cp -pf "${BOARD_DIR}/osmc-devinput-remote.json" "${TARGET_DIR}/usr/share/WPEFramework/RemoteControl/devinput-remote.json"
fi

# Copy network stuff
if [ -f "${BOARD_DIR}/interfaces-init" ]; then
	mkdir -p "${TARGET_DIR}/etc/network"
	cp -pf "${BOARD_DIR}/interfaces-init" "${TARGET_DIR}/etc/network/interfaces"
fi
if [ -f "${BOARD_DIR}/wpa-init" ]; then
	cp -pf "${BOARD_DIR}/wpa-init" "${TARGET_DIR}/etc//wpa_supplicant.conf"
fi
echo "*********************"
echo "*********************aaaaaaaaaaa ${BOARD_DIR}"
    ls ${BOARD_DIR}
# Copy thttpd stuff
if [ -d "${BOARD_DIR}/thttpd" ]; then
echo "*********************aaaaaaaaaaa ${BOARD_DIR}"
    ls ${BOARD_DIR}
	mkdir -p "${TARGET_DIR}/var/www/test"
	cp -rpf "${BOARD_DIR}/thttpd/." "${TARGET_DIR}/var/www"
fi
