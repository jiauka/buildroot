################################################################################
#
# engelcec
#
################################################################################
ENGELCEC_VERSION = 1.0
ENGELCEC_SITE = ./package/engelcec/src
ENGELCEC_SITE_METHOD = local
define ENGELCEC_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef
define ENGELCEC_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/engelcec $(TARGET_DIR)/usr/bin
	$(INSTALL) -D -m 0755 package/engelcec/engelcec-init $(TARGET_DIR)/etc/init.d/S90engelcec

endef
$(eval $(generic-package))

