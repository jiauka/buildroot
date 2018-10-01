################################################################################
#
# wpeframework-webpa
#
################################################################################

WPEFRAMEWORK_WEBPA_VERSION = eb5032aa82dd61332a81a27716b19606ac0f2771
WPEFRAMEWORK_WEBPA_SITE_METHOD = git
WPEFRAMEWORK_WEBPA_SITE = git@github.com:WebPlatformForEmbedded/WPEPluginWebPA.git
WPEFRAMEWORK_WEBPA_INSTALL_STAGING = YES
WPEFRAMEWORK_WEBPA_DEPENDENCIES = wpeframework libparodus tinyxml

WPEFRAMEWORK_WEBPA_CONF_OPTS += -DBUILD_REFERENCE=${WPEFRAMEWORK_WEBPA_VERSION}

ifeq ($(BR2_PACKAGE_WPEFRAMEWORK_DEBUG),y)
WPEFRAMEWORK_WEBPA_CONF_OPTS += -DCMAKE_CXX_FLAGS='-g -Og'
endif

ifeq ($(BR2_PACKAGE_WPEFRAMEWORK_WEBPA_AUTOSTART),y)
WPEFRAMEWORK_WEBPA_CONF_OPTS += -DWPEFRAMEWORK_PLUGIN_WEBPA_AUTOSTART=true
else
WPEFRAMEWORK_WEBPA_CONF_OPTS += -DWPEFRAMEWORK_PLUGIN_WEBPA_AUTOSTART=false
endif

ifeq ($(BR2_PACKAGE_WPEFRAMEWORK_WEBPA_OOP),y)
WPEFRAMEWORK_WEBPA_CONF_OPTS += -DWPEFRAMEWORK_PLUGIN_WEBPA_OOP=true
else
WPEFRAMEWORK_WEBPA_CONF_OPTS += -DWPEFRAMEWORK_PLUGIN_WEBPA_OOP=false
endif
ifeq ($(BR2_PACKAGE_WPEFRAMEWORK_WEBPA_DEVICE_INFO),y)
WPEFRAMEWORK_WEBPA_CONF_OPTS += -DWPEFRAMEWORK_PLUGIN_WEBPA_DEVICE_INFO=true
else
WPEFRAMEWORK_WEBPA_CONF_OPTS += -DWPEFRAMEWORK_PLUGIN_WEBPA_DEVICE_INFO=false
endif
ifeq ($(BR2_PACKAGE_WPEFRAMEWORK_WEBPA_SERVICE),y)
WPEFRAMEWORK_WEBPA_CONF_OPTS += -DWPEFRAMEWORK_PLUGIN_WEBPA_SERVICE=true
WPEFRAMEWORK_WEBPA_CONF_OPTS += -DCMAKE_CXX_FLAGS=-DWPEFRAMEWORK_PLUGIN_WEBPA_SERVICE
else
WPEFRAMEWORK_WEBPA_CONF_OPTS += -DWPEFRAMEWORK_PLUGIN_WEBPA_SERVICE=false
endif

$(eval $(cmake-package))

