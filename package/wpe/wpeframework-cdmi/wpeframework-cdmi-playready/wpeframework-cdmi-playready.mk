################################################################################
#
# wpeframework-cdmi-playready
#
################################################################################

WPEFRAMEWORK_CDMI_PLAYREADY_VERSION = 21fe7ac8fec4dc7cbd39fcb43cf0a132986001d4
WPEFRAMEWORK_CDMI_PLAYREADY_SITE_METHOD = git
WPEFRAMEWORK_CDMI_PLAYREADY_SITE = git@github.com:WebPlatformForEmbedded/OCDM-Playready.git
WPEFRAMEWORK_CDMI_PLAYREADY_INSTALL_STAGING = YES
WPEFRAMEWORK_CDMI_PLAYREADY_DEPENDENCIES = wpeframework playready

$(eval $(cmake-package))
