# fcpp-android

## Current instructions to build for Android

1. Make sure you've done `git submodule init/update` so that the `fcpp`-folder is populated.
2. Install Android Studio. Make sure that you have the required versions of SDK, NDK and CMake.
5. Android Studio should now correctly build the libraries.

This should cross-compile the fcpp-code (library + actual application from `src/`) for the Android ABIs configured in the Makefile. 
These can then be connected via C++ to e.g. an Android app.
