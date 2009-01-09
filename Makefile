#
#Kernel & Loader global Makefile n-th version
#

all: kernel loader librt disc
.PHONY: kernel loader librt disc
kernel:
	@echo "Building kernel";
	$(MAKE) -C kernel kernel "KERNEL_TEST=$(KERNEL_TEST)"

loader:
	@echo "Building loader"
	$(MAKE) -C loader loader

librt:
	@echo "Building librt"
	$(MAKE) -C librt librt

disc:
	@echo "Creating disc including: " `ls disc`
	@touch disc/tmp
	@tar -C disc -cf disc.tar `ls disc`

### cleaning stuff ###
.PHONY: clean
clean: clean-kernel clean-loader clean-librt clean-disc
clean-kernel:
	@echo "Cleaning kernel";
	$(MAKE) -C kernel clean
clean-loader:
	@echo "Cleaning loader"
	$(MAKE) -C loader clean
clean-librt:
	@echo "Cleaning librt"
	$(MAKE) -C librt clean
clean-disc:
	@echo "Cleaning disc"
	@rm -f disc.tar

### distcleaning stuff ###
.PHONY: distclean
distclean: distclean-kernel distclean-loader distclean-librt clean-disc
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
