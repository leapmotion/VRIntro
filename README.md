VRIntro
=======

Leap Motion's intro to VR

##### Usage

In VR you can do anything.  You can fly through space, manipulate the stars and swim through bubbles.

Press 1 for Grid, 2 for Spheres, 3 for Magical Stars and 4 for Aerobatics.

Be sure to select "Allow Images" and "Optimize for top-down tracking" in the Leap Motion Control Panel before launching this example.

* This demo supports both the Oculus DK1 and DK2.
* Please use the 0.4.1 runtime and set the Rift Display Mode to "Extend Desktop to the HMD"
* Designed to be used with [Leap Motion VR Developer Mount](/vr).

* Sorry, this is **Windows Only** for the time being.
* Works best with NVIDIA graphics cards.

##### How to build (Win32)

1. Get CMake 3.0+
2. Download libraries here https://leapmotion.box.com/s/hy6cq89y2gze2o97s8t1
3. Extract the zip file into a local folder
4. Obtain Oculus Rift SDK 0.4.1+, and the latest LeapSDK, and copy "OculusSDK" and "LeapSDK" into the folder mentioned in step 3
5. Run CMake and set the source directory to /source in this repo
6. Set EXTERNAL_LIBRARY_DIR to the folder mentioned in step 3
7. Open the generated VRIntro.sln file, and build