#############################################################
# $Id$
#
# Makefile for the proprietary Broadcom drivers
#
#############################################################

LINUX_BINARY_DRIVER_SITE=http://openwrt.org/downloads/

# proprietary driver, extracted from asus GPL sourcetree GPL_1941.zip
LINUX_BINARY_WL_DRIVER=kernel-binary-wl-0.3.tar.gz
LINUX_BINARY_WL_MD5SUM=cc45df670bcfb4e74a709b9d7beba636
LINUX_ET_DRIVER=kernel-source-et-0.8.tar.gz
LINUX_ET_MD5SUM=6a5cedaf88c7660380e0ac8c89fc6318

$(DL_DIR)/$(LINUX_BINARY_WL_DRIVER):
	$(SCRIPT_DIR)/download.pl $(DL_DIR) $(LINUX_BINARY_WL_DRIVER) $(LINUX_BINARY_WL_MD5SUM) $(LINUX_BINARY_DRIVER_SITE)

$(DL_DIR)/$(LINUX_ET_DRIVER):
	$(SCRIPT_DIR)/download.pl $(DL_DIR) $(LINUX_ET_DRIVER) $(LINUX_ET_MD5SUM) $(LINUX_BINARY_DRIVER_SITE)
	
$(LINUX_DIR)/.patched: $(LINUX_DIR)/.drivers-unpacked

$(LINUX_DIR)/.drivers-unpacked:
	-mkdir -p $(BUILD_DIR)
	zcat $(DL_DIR)/$(LINUX_BINARY_WL_DRIVER) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	zcat $(DL_DIR)/$(LINUX_ET_DRIVER) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	# copy binary wlan driver
	mkdir $(LINUX_DIR)/drivers/net/{et,wl}
	cp -a $(BUILD_DIR)/wl/*.o $(LINUX_DIR)/drivers/net/wl
	# copy proprietary et source
	cp -a $(BUILD_DIR)/et/* $(LINUX_DIR)/drivers/net/et
	mkdir -p $(LINUX_DIR)/arch/mips/bcm947xx/include/
	cp -a $(BUILD_DIR)/et/*.h $(LINUX_DIR)/arch/mips/bcm947xx/include/
	touch $@

linux-dirclean: drivers-clean

drivers-clean:
	rm -rf $(BUILD_DIR)/wl
	rm -rf $(BUILD_DIR)/et

