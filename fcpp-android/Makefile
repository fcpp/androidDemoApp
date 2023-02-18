-include local.mk
ANDROID_HOME	?= /Users/vs/Library/Android/sdk
NDK_VERSION	?= 22.1.7171670
PLATFORM	?= 30
CMAKE		?= $(ANDROID_HOME)/cmake/3.22.1/bin/cmake

abis = arm64-v8a x86_64

.PHONY:	$(abis)

all: $(abis)

fcpp/src:
	git submodule init
	git submodule update

$(abis): fcpp/src
	mkdir -p $@
	$(CMAKE) -S . -B $@ -DCMAKE_TOOLCHAIN_FILE=$(ANDROID_HOME)/ndk/$(NDK_VERSION)/build/cmake/android.toolchain.cmake   -DANDROID_ABI=$@     -DANDROID_PLATFORM=android-$(PLATFORM) -DCMAKE_MAKE_PROGRAM=$(MAKE) -G "Unix Makefiles"
	cd $@ && $(MAKE) main
