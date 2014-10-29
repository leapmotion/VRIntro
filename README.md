VRIntro
=======

Leap Motion's intro to VR

##### Usage

In VR you can do anything.  You can fly through space, manipulate the stars and swim through bubbles.

Press 1 for Grid, 2 for Spheres, 3 for Magical Stars and 4 for Aerobatics.

Be sure to select "Allow Images" and "Optimize for top-down tracking" in the Leap Motion Control Panel before launching this example.

* This demo supports both the Oculus DK1 and DK2.
* Includes exectuables for both Windows and OSX.
* Please use the 0.4.2+ runtime and set the Rift Display Mode to "Extend Desktop to the HMD"
* Designed to be used with [Leap Motion VR Developer Mount](https://developer.leapmotion.com/vr).
* Requires Leap SDK Version 2.1.3 or newer
* Source code available now!
* Works best with discrete graphics cards (NVIDIA, AMD)

##### How to build (Win32)

1. Get CMake 3.0+ and Microsoft Visual Studio 2013
2. Download libraries here https://leapmotion.box.com/s/hy6cq89y2gze2o97s8t1
3. Extract the zip file into a local folder
4. Obtain Oculus Rift SDK 0.4.2+, and the latest LeapSDK, and copy "OculusSDK" and "LeapSDK" into the folder mentioned in step 3
5. Run CMake and set the source directory to this repository
6. Set EXTERNAL_LIBRARY_DIR to the folder mentioned in step 3
7. Open the generated VRIntro.sln file, and build
8. Copy any needed DLLs (SDL, SFML) from within the libraries folder to your binary folder. (We are planning to automate this soon!)

##### How to build (OSX)

1. Get CMake 3.0+ and Xcode 6.0+
2. Download libraries here https://leapmotion.box.com/s/5gmz97ldcxaf3d5sneob
3. Extract the zip file into a local folder
4. Obtain Oculus Rift SDK 0.4.2+, and the latest LeapSDK, and copy "OculusSDK" and "LeapSDK" into the folder mentioned in step 3
5. cd into the VRintro repository and run ccmake. Set the source directory to ./VRIntro
6. Set EXTERNAL_LIBRARY_DIR to the folder mentioned in step 3
7. Open the generated VRIntro.xcodeproj file, and build
