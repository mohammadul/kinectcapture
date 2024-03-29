KinectCapture
=============

KinectCapture is a user-friendly modified libfreenect front-end software to capture Kinect raw data.

-----------
Compilation
-----------
KinectCapture requires the following libraries and IDEs to get compiled
a) freenect (modified) according to cameras.h
b) pthread
c) gtk
d) gtkglext
e) gl, glu
f) code-blocks.


--------------
User Interface
--------------
The user-interface consists of the real-time depth and rgb/ir display with optional histogram display for rgb/ir. The menu consists of 'Load Program' item to load a valid program. The lower end of the user interface contains the Write Register, Write CMOS Register, Run program, Brightness Set input options.


-------------------
Keyboard Shortcuts
-------------------
1 - mode 1 (RGB 640X480 8bit + Depth 640X480 8bit).  
2 - mode 2 (YUV-RGB 640X480 8bit + Depth 640X480 8bit).  
3 - mode 3 (IR 640X480 8bit + Depth 640X480 8bit).  
4 - mode 4 (RGB 1280X1024 8bit + Depth 640X480 8bit).  
5 - mode 5 (IR 1280X1024 8bit).  
6 - mode 6 (IR 1280X1024 10bit).  
c - captures current mode.  
d - toggles depth display.  
e - toggles exposure display.  
f - cycles through all modes.  
h - toggles histogram display.  
m - depth smoothing off.  
n - depth smoothing on.  
p - toggles projector.  
q - switches among multiple kinects.  
r - runs loaded program if any.  
s - resets angle to 0.  
w - increases tilt angle.  
x - decreases tilt angle.  


----------------------------
Programming in KinectCapture
----------------------------
To load a program in memory, check program mode on the main window.
Goto File menu>Load Program and select the program file.
Only on valid program, the Run button 'r' keyboard shortcut will be enabled.


------------------------
Structure of the program
------------------------
Every program must start with the keyword START and end with the keyword STOP.

Comments are allowed at any line except lines that contain codes. e.g.
% this a demo program.


----------------
Program keywords
----------------
All keywords are case-sensitive. The valid keywords are given below.

START - start of a program.  
STOP - end of a program.  
PAUSE <milliseconds> - pauses the code for <milliseconds> ms.  
MODE <value> - changes the current mode to <value> which must be in 1-6 range.  
CAPTURE - captures current mode in memory.  
SAVE - saves captured data to raw file format.  
PROJECTOR - toggles projector.  
BRIGHTNESS <value> - changes the ir brightness to <value> level which must be in 0-50 range.  
SWITCH <value> - change current device to kinect number <value> for multiple kinects. (-1 for next kinect)  
LOOP <value> - beginning of a loop for <value> times.  
END - end of a loop.  


------------------------------
Capturing data through program
------------------------------
For example to capture all the depth, rgb and ir images, all the required modes need to be captured. e.g. in 640X480 resolution, one must capture both mode 1/2 (for depth and rgb) and mode 3 (for depth and ir) as in the demo program below.

> 
% this a demo program to capture all data  
START  
% change to mode 1  
MODE 1  
% pause to stabilize the images  
PAUSE 1000  
% capture depth and rgb  
CAPTURE  
% change to mode 3  
MODE 3  
% pause to stabilize the images  
PAUSE 1000  
% capture depth and ir, note depth will be overwritten  
CAPTURE  
% now save the captured data  
SAVE  
STOP  
>


Copyright (c) 2014 Sk. Mohammadul Haque


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.


For more details and updates, visit http://mohammadulhaque.alotspace.com
