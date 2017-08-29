# Kinect Skeleton Tracking Emulator using DLL-Proxy

[![Build status](https://ci.appveyor.com/api/projects/status/or1c1gl2kpycc2i9?svg=true)](https://ci.appveyor.com/project/TinyTinni/kinectfaker)


Emulates a Kinect Device (Version 1/xBox360) with Skeleton Tracking.
The Kinect Toolkit provides a program, called Kinect Studio, which is able to record
and play Skeleton Animations. Sadly, the Kinect Studio program works only
on a PC with a connected Kinect Device.
This emulator enables testing of Kinect applications without connected Kinect 
or Kinect runtime by reading skeleton animation files.
Currently, only a self defined file format (defined with protobuf) is used and
must be recorded first.

Recompiling or relinking of the Kinect application is not needed.

_WIP: GUI is WIP. Currently, it can record and play skeleton animations._

See [ffmpeg branch](https://github.com/TinyTinni/KinectFaker/tree/ffmpeg) for a version for faked color stream.

## Requirements
For the Proxy:
- Headers from the [Kinect SDK v1.8](https://www.microsoft.com/en-us/download/details.aspx?id=40278) 
- [Google's Protobuf](https://github.com/google/protobuf)

For the GUI (Record & Play Skeleton Data):
- [Kinect SDK v1.8](https://www.microsoft.com/en-us/download/details.aspx?id=40278) libraries and runtime
- [Qt5](https://www.qt.io/)  
- [Google's Protobuf](https://github.com/google/protobuf)


used libs which will be auto downloaded, when not specified
- [spdlog](https://github.com/gabime/spdlog)
- [nlohmann/json](https://github.com/nlohmann/json)
- [outcome](https://github.com/ned14/outcome)

for tests:
- [catch](https://github.com/philsquared/Catch)

If you want to use [Conan](https://www.conan.io/) or [Hunter](https://github.com/ruslo/hunter)
as package manger, activate the cmake option `USE_CONAN` resp. `USE_HUNTER` [default=OFF].

When you are using Hunter, only a Kinect SDK installation is required. Rest will be automatically downloaded.
When you are using Conan, make sure you edit the [conanfile.txt](./conanfile.txt) since it is only used by the build system.


## Using DLL-Proxy
- Record your skeleton using the prototyped recorder (not recommended) or the GUI (recommended).
- Configure your emulated scene in _fake_kinect.config_, example can be found in [tests](.tests/fake_kinect.config).
- Put the configure file and the skeleton animation into the folder with the binary, which should recieve the emulated
skeleton animation.
- Copy Kinect10.dll + dependencies (e.g. protobuf dlls) into the binary folder of your program.

Run the program.

## Implemented Functions
Some functions are free functions and INuiSensor functions.
When using functions which also exists in INuiSensor e.g. `NuiInitialize` instead of multi device mode `INuiSensor::NuiInitialize`,
these "free"-functions will be listed in INuiSensor column.
Not implemented functions will return `E_NOIMPL`.


| Free Functions        | INuiSensor        |
|---------------------- |-------------------|
|NuiGetSensorCount      |NuiInitialize
|NuiCreateSensorByIndex |NuiShutdown
|NuiCreateSensorById    |NuiSkeletonTrackingEnable 
|                       |NuiSkeletonTrackingDisable
|                       |NuiSkeletonGetNextFrame 
|                       |NuiTransformSmooth *) 
|                       |NuiDeviceConnectionId 
|                       |NuiUniqueId **) 
|                       |NuiStatus 
|                       |NuiInstanceIndex
|                       |NuiInitializationFlags

*)_NuiTransformSmooth_ Skeleton positions are saved after smoothing, therefore this function does nothing and has to be set in the animation file. 

**) _NuiUniqueId_ [Recommended by MSDN: Don't use!](https://msdn.microsoft.com/en-us/library/hh973101.aspx)

Not all members of a _NUI_SKELETON_FRAME_ instance are filled yet.

Functions which will only be avaiable when a Kinect runtime is present:

| Kinect Runtime Functions  |
|---------------------------|
|NuiCreateDepthFilter       |
|NuiCreateCoordinateMapperFromParameters|
|NuiSkeletonCalculateBoneOrientations|
|NuiSetDeviceStatusCallback|

They are not changed in anyway and not guaranteed to work.

## LICENSE
[GPLv3](./License)© Matthias Möller. Made with ♥ in Germany.
