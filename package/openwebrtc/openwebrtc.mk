################################################################################
#
# openwebrtc
#
################################################################################

OPENWEBRTC_VERSION = 35beb4e2fa9436564edfed31e6cf149bbe9c265d
OPENWEBRTC_SITE = $(call github,Metrological,openwebrtc,$(OPENWEBRTC_VERSION))
OPENWEBRTC_LICENSE = BSD-2 Clause
OPENWEBRTC_LICENSE_FILES = LICENSE

OPENWEBRTC_INSTALL_STAGING = YES
OPENWEBRTC_AUTORECONF = YES

OPENWEBRTC_DEPENDENCIES = gst1-openwebrtc libnice pulseaudio

define OPENWEBRTC_RUN_AUTOGEN
        cd $(@D) && PATH=$(BR_PATH) ./autogen.sh --disable-gtk-doc --noconfigure
endef
OPENWEBRTC_PRE_CONFIGURE_HOOKS += OPENWEBRTC_RUN_AUTOGEN


OPENWEBRTC_CONF_OPTS += \
	--enable-owr-gst \
	--disable-bridge \
	--disable-introspection \
	--disable-gtk-doc

$(eval $(autotools-package))
