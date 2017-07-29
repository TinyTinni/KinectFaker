# Kinect Skeleton Tracking Emulator using DLL-Proxy

https://ci.appveyor.com/api/projects/status/or1c1gl2kpycc2i9?svg=true

Emulates a Kinect Device (Version 1) with Skeleton Tracking Enabled.
The Emulator enables testing of Kinect Applications without connected Kinect 
by reading Skeleton Animation files.
Currently, only a self defined file format (defined with protobuf) is used and
must be recorded first.

Recompiling or relinking of the Kinect program is not needed.

_WIP: GUI is WIP. Currently, it can record and play skeleton animations._

Emulated Features:
- Skeleton Positions

## Requirements
- [Kinect SDK v1.8](https://www.microsoft.com/en-us/download/details.aspx?id=40278)
- [Google's Protobuf](https://github.com/google/protobuf)
- Qt5 for the GUI (Record & Play Skeleton Data)

auto download
- spdlog
- catch for tests

## Using DLL-Proxy
- Record your skeleton using the prototyped recorder (not recommended) or the GUI (recommended).
- Configure your emulated scene in _fake_kinect.config_, example can be found in [tests](.tests/fake_kinect.config).
- Put the configure file and the skeleton animation into the folder with the binary, which should recieve the emulated
skeleton animation.
- Copy Kinect10.dll + dependencies (e.g. protobuf dlls) into the binary folder of your program.

Run the program.

## Implemented Functions
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

## LICENSE
[GPLv3](./License)© Matthias Möller. Made with ♥ in Germany.