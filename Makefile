#
#Kernel & Loader global Makefile 1st version
#

all: kernel loader
.PHONY: kernel loader
kernel:
	@echo "Building kernel";
	$(MAKE) -C kernel kernel "KERNEL_TEST=$(KERNEL_TEST)"

loader:
	@echo "Building loader"
	$(MAKE) -C loader loader

### cleaning stuff ###
.PHONY: clean
clean: clean-kernel clean-loader
clean-kernel:
	@echo "Cleaning kernel";
	$(MAKE) -C kernel clean
clean-loader:
	@echo "Cleaning loader"
	$(MAKE) -C loader clean

### distcleaning stuff ###
.PHONY: distclean
distclean: distclean-kernel distclean-loader
distclean-kernel:
	@echo "Distcleaning kernel"
	$(MAKE) -C kernel distclean
distclean-loader:
	@echo "Distcleaning loader"
	$(MAKE) -C loader distclean

doc:
	@echo "Creating documentation :)";
	@doxygen
