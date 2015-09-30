This project currently functions as a beat detector written in C++. (Windows only)

There a lot of "extra" files are kept in this repo for conveninece due to the direction it will take in the future.
Current code is contained in source/ConsoleApplication1/BeatDetector/Main.cpp and source/ConsoleApplication1/BeatDetector/SimpleBeatDetection.h.

The beat detection algorithm will first split the main signal into several frequency bands and perform beat detection on each band. The beats detected from each band are then merged back into one set of beats and displayed with the music playing. A beat is detected when the current signal strength is greater than the average previous signals by a sensitivity constant generated from variance.


Background
This project is an exercise for creating an auto .sm generator for Stepmania through machine learning. Likely only parts of this project will be reused.
The project currently faces concurrency issues in syncing the music playing thread with the waveform drawing thread.