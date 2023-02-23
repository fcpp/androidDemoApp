
Android FCPP Sample Application
===================================

Sample demonstrating of how to implement FCPP applications that exchange data using the Bluetooth
Low Energy API (requires 2 devices to see full operation).

Introduction
------------

This project uses the Bluetooth Low Energy (BLE) [Advertisement][1] and [Scanning][2] APIs.
The functionality is split into several fragments - one for Advertising, one for Scanning, and then
an additional one with the user interface for each particular application/experiment.

Note: A device cannot detect its own BLE advertisements. You will need two devices to see this
sample in action.

[1]:https://developer.android.com/reference/android/bluetooth/le/BluetoothLeAdvertiser.html
[2]:https://developer.android.com/reference/android/bluetooth/le/BluetoothLeScanner.html

Getting Started
---------------

This project requires the `fcpp/fcpp` submodule. Either do a `git submodule update` and use the
`./gradlew build` command, or simply use "Import Project" in Android Studio which should take care of all the steps.

You can run the corresponding simulations in the [`fcpp-android`](fcpp-android)-folder through `./make.sh gui run -O simulation`.
