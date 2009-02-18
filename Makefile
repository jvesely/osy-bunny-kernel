#
#Kernel & Loader global Makefile n-th version
#

.PHONY: kernel loader librt apps disk
all: kernel loader librt apps disk

kernel:
	@echo "Building kernel";
	$(MAKE) -C kernel kernel "KERNEL_TEST=$(KERNEL_TEST)" "USER_TEST=$(USER_TEST)"

loader:
	@echo "Building loader"
	$(MAKE) -C loader loader

librt:
	@echo "Building librt"
	$(MAKE) -C librt librt

apps: librt
	@echo "Building apps"
	$(MAKE) -C apps apps "USER_TEST=$(USER_TEST)"

disk: apps
	@ls apps/bin/*.bin > /dev/null 2>&1 || touch apps/bin/tmp.bin
	@echo "Creating disk including: " `ls apps/bin/*.bin`
	@tar -C apps/bin -cf disk.tar `ls apps/bin/*.bin | cut -f 3 -d "/"`

### cleaning stuff ###
.PHONY: clean
clean: clean-kernel clean-loader clean-librt clean-apps clean-disk

clean-kernel:
	@echo "Cleaning kernel";
	$(MAKE) -C kernel clean
clean-loader:
	@echo "Cleaning loader"
	$(MAKE) -C loader clean
clean-librt:
	@echo "Cleaning librt"
	$(MAKE) -C librt clean
clean-apps:
	@echo "Cleaning apps"
	$(MAKE) -C apps clean
clean-disk:
	@echo "Cleaning disk"
	@rm -f disk.tar

### distcleaning stuff ###
.PHONY: distclean
distclean: distclean-kernel distclean-loader distclean-librt distclean-apps clean-disk
distclean-kernel:
	@echo "Distcleaning kernel"
	$(MAKE) -C kernel distclean
distclean-loader:
	@echo "Distcleaning loader"
	$(MAKE) -C loader distclean
distclean-librt:
	@echo "Distcleaning librt"
	$(MAKE) -C librt distclean
distclean-apps:
	@echo "Distcleaning apps"
	$(MAKE) -C apps distclean

doc:
	@echo "Creating documentation :)";
	@doxygen
