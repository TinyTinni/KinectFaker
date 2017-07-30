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

## Requirements
- [Kinect SDK v1.8](https://www.microsoft.com/en-us/download/details.aspx?id=40278) (GUI requires KinectSDK + runtime, the proxy just requires the Kinect Headers. Proxy does not require the Kinect runtime, but provides more functionality)
- [Google's Protobuf](https://github.com/google/protobuf)
- Qt5 for the GUI (Record & Play Skeleton Data)

used libs which will be auto downloaded, when not specified
- spdlog
- nlohmann/json
- catch for tests
- outcome

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
