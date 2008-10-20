#
#Kernel & Loader global Makefile 1st version
#

all: kernel loader
.PHONY: kernel loader
kernel:
	@echo "Building kernel";
	$(MAKE) -C kernel kernel

loader:
	@echo "Building loader"
	$(MAKE) -C loader loader


clean: clean-kernel clean-loader
clean-kernel:
	@echo "Cleaning kernel";
	$(MAKE) -C kernel clean
clean-loader:
	@echo "Cleaning loader"
	$(MAKE) -C loader clean
doc:
	@echo "Creating documentation :)";
	@doxygen
