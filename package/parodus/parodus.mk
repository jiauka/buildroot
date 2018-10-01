################################################################################
#
# parodus
#
################################################################################

PARODUS_VERSION = cc3c701b4a972afef05149b4f43ba06007c6de2d
PARODUS_SITE_METHOD = git
PARODUS_SITE = https://github.com/HaseenaSainul/parodus
PARODUS_INSTALL_STAGING = YES

PARODUS_DEPENDENCIES = nopoll cimplog nanomsg msgpack-c cjson trower-base64 wrp-c wdmp-c cjson

PARODUS_CONF_OPTS = \
        -DCMAKE_C_FLAGS="$(TARGET_CFLAGS) $(PARODUS_INCLUDE_DIRS)" \
        -DBUILD_BR=ON

ifeq ($(BR2_PACKAGE_PARODUS_SERVICE_APP), y)
PARODUS_CONF_OPTS += -DPARODUS_SERVICE_APP=true
endif

PARODUS_INCLUDE_DIRS = \
    -I$(STAGING_DIR)/usr/include \
    -I$(STAGING_DIR)/usr/include/cjson \
    -I$(STAGING_DIR)/usr/include/nopoll \
    -I$(STAGING_DIR)/usr/include/wdmp-c \
    -I$(STAGING_DIR)/usr/include/wrp-c \
    -I$(STAGING_DIR)/usr/include/cimplog \
    -I$(STAGING_DIR)/usr/include/nanomsg \
    -I$(STAGING_DIR)/usr/include/trower-base64 \
    -I$(STAGING_DIR)/usr/include/cjwt \
    -I$(STAGING_DIR)/usr/include/ucresolv

$(eval $(cmake-package))
