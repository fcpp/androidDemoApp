
Android BluetoothAdvertisements Sample
===================================

Sample demonstrating how to advertise small amounts of data using the Bluetooth
Low Energy API. Also demonstrates how to scan for those Advertisements. (requires
2 devices to see full operation)

Introduction
------------

This sample demonstrates use of the Bluetooth Low Energy (BLE) [Advertisement][1] and [Scanning][2] APIs.
The functionality is split into two fragments - one for Advertising, one for Scanning.

ScannerFragment activates BLE Scanning for 5 seconds and displays a list of found devices which are advertising
using this sample. It shows the device type, Bluetooth address, and when it was last seen. User can
refresh to scan again and update the list.

AdvertiserFragment allows the user to toggle BLE Advertising of that device. It broadcasts basic
information about the device along with a UUID specific to this app so the ScannerFragment on other
devices can filter by it.

Note: A device cannot detect its own BLE advertisements. You will need two devices to see this
sample in action.

[1]:https://developer.android.com/reference/android/bluetooth/le/BluetoothLeAdvertiser.html
[2]:https://developer.android.com/reference/android/bluetooth/le/BluetoothLeScanner.html

Pre-requisites
--------------

- Android SDK 28
- Android Build Tools v28.0.3
- Android Support Repository
- CMake 3.22.1 (via Android)

Getting Started
---------------

First, use the terminal to go into `fcpp-android` and invoke `make`, then use the
"gradlew build" command or use "Import Project" in Android Studio.

