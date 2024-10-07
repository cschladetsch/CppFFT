#!/bin/bash

# Update package list
sudo apt-get update

# Install required packages
sudo apt-get install -y libsfml-dev libfftw3-dev libsndfile1-dev g++

# Check if main.cpp exists
if [ ! -f "main.cpp" ]; then
    echo "Error: main.cpp not found in the current directory."
    exit 1
fi

# Compile the program
g++ -o fft_visualiser main.cpp -lfftw3 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsndfile

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful. The program 'fft_visualiser' has been created."
    echo ""
    echo "Usage instructions:"
    echo "1. Make sure you have a mono WAV file for visualization."
    echo "2. Run the program with the following command:"
    echo "   ./fft_visualiser path_to_your_audio_file.wav"
    echo ""
    echo "Note: If you're using WSL, you may need to set up X11 forwarding to see the graphical output."
else
    echo "Compilation failed. Please check for errors in the code."
fi
