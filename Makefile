#
#Kernel & Loader global Makefile n-th version
#

all: kernel loader librt disk
.PHONY: kernel loader librt disk
kernel:
	@echo "Building kernel";
	$(MAKE) -C kernel kernel "KERNEL_TEST=$(KERNEL_TEST)"

loader:
	@echo "Building loader"
	$(MAKE) -C loader loader

librt:
	@echo "Building librt"
	$(MAKE) -C librt librt

disk:
	@echo "Creating disk including: " `ls disk`
	@touch disk/tmp
	@tar -C disk -cf disk.tar `ls disk`

### cleaning stuff ###
.PHONY: clean
clean: clean-kernel clean-loader clean-librt clean-disk
clean-kernel:
	@echo "Cleaning kernel";
	$(MAKE) -C kernel clean
clean-loader:
	@echo "Cleaning loader"
	$(MAKE) -C loader clean
clean-librt:
	@echo "Cleaning librt"
	$(MAKE) -C librt clean
clean-disk:
	@echo "Cleaning disk"
	@rm -f disk.tar

### distcleaning stuff ###
.PHONY: distclean
distclean: distclean-kernel distclean-loader distclean-librt clean-disk
distclean-kernel:
	@echo "Distcleaning kernel"
	$(MAKE) -C kernel distclean
distclean-loader:
	@echo "Distcleaning loader"
	$(MAKE) -C loader distclean
distclean-librt:
	@echo "Distcleaning librt"
	$(MAKE) -C librt distclean

doc:
	@echo "Creating documentation :)";
	@doxygen
