---
layout: post
title:  "Umfeld in Arduino"
date:   2025-03-27 10:00:00 +0100
---

![2025-03-27-Umfeld_in_Arduino.jpg](/assets/2025-03-27-Umfeld_in_Arduino.jpg)

wait a minute! is this sketch run from the *Arduino IDE*? how cool is that?!?

check out the ( still very experimental, only tested in macOS on Apple Silicon ) repository: [Umfeld-Arduino](https://github.com/dennisppaul/umfeld-arduino)

not sure if i am the only one who finds that funny :) there are of course a few fundamental limitations with this approach in general e.g the *Arduino IDE* build-system is ( no offense ) … well, not good … especially when compared to CMake. so integration of extra libraries is problematic if they need external resources of compile flags. there is no way to slip these into the Arduino build script :(

still as a quick start environment … it might be a nice option.

PS i just found out that it is possible to run files with the extension `.pde` ( probably for historical reasons ), however, it seem more appropiate append `.ino` to a sketch … as in *normal* arduino applications.