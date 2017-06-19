# Kinect Skeleton Tracking Emulator using DLL-Proxy

Emulates a Kinect Device (Version 1) with Skeleton Tracking Enabled.
The Emulator enables testing of Kinect Applications without connected Kinect 
by reading Skeleton Animation files.
Currently, only a self defined file format (defined with protobuf) is used and
must be recorded first.

Recompiling or relinking of the Kinect program is not needed.

__WIP: interface and file definition are not stable.__

## Requirements
- [Kinect SDK v1.8](https://www.microsoft.com/en-us/download/details.aspx?id=40278)
- [Google's Protobuf](https://github.com/google/protobuf)

## Using DLL-Proxy
Copy Kinect10.dll + dependencies (protobuf dlls) into the binary folder of your program.

## LICENSE
[GPLv3](./License)© Matthias Möller. Made with ♥ in Germany.