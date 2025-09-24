# SIT102 Habit Tracker

Simple habit tracker (C++ / SplashKit) for SIT102 — Erin.

## Features
- Add/remove/update habits
- 28-day rolling log + streak calculation
- Save/load to file (persistence)
- Simple text UI (terminal) — optional GUI in `src/gui/`

## Build (Ubuntu)
clang++ habit-tracker.cpp ../Utilities/utilities.cpp -I../Utilities -l SplashKit -o habit
./habit
