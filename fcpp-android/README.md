# fcpp-android

## Current instructions to build for Android

1. Make sure you've done `git submodule init/update` so that the `fcpp`-folder is populated.
2. I can't remember at the moment if the `plot` step in `make.sh` (which we're not using) is still necessary.
3. Install Android Studio. Make sure that you have the versions of SDK, NDK and CMake that are hardcoded in the `Makefile` installed within Android Studio.
4. Configure your `ANDROID_HOME`-variable correctly.
5. `make` (on Windows: find `make.exe` within Android Studio).

This should cross-compile the fcpp-code (library + actual application from `src/`) for the Android ABIs configured in the Makefile. 
These can then be connected via C++ to e.g. an Android app.
