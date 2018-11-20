# JNI-Code-Caving
Allows user to call JNI from a code-cave

Simple code caving example, allowing the user to call JNI functions. This code was written for x64 Release and as far as I know shouldn't work on x86.

# NOTE
This is only a POC, this is not meant to be *actually* used, rather it should be **applied**, thus the code isn't fully worked out and there are major problems with it.

# main.cpp
Entry point for the program, detects target process by their executable name and writes, executes the code cave.
Also retrieves the necessary function addresses the cave needs.

# utils.hpp
Handles all imports, and required functions. 
It has a namespace called "threads" where if I were to add any more code caving examples, the examples would be stored.
