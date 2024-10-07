# FFT Visualizer

## Overview
The FFT Visualizer is a C++ application that reads audio data from a WAV file and visualizes its frequency spectrum in real time using Fast Fourier Transform (FFT). The visualizer is built with the following libraries:
- **FFTW**: For performing the Fast Fourier Transform.
- **SFML**: For creating a graphical window and rendering the frequency spectrum.
- **libsndfile**: For reading WAV audio files.

## Features
- Reads mono WAV files and performs real-time FFT analysis.
- Displays the frequency spectrum in a graphical window.
- Simple and easy to use.

## Installation
1. **Install Dependencies**: Ensure you have the required libraries installed:
   ```bash
   sudo apt update
   sudo apt install libfftw3-dev libsfml-dev libsndfile1-dev
   ```

2. **Clone the Repository**:
   ```bash
   git clone https://github.com/cschladetsch/fft_visualizer.git
   cd fft_visualizer
   ```

3. **Compile the Application**:
   ```bash
   g++ -o fft_visualizer main.cpp -lfftw3 -lsfml-graphics -lsfml-window -lsfml-system -lsndfile
   ```

## Usage
Run the visualizer by specifying a WAV file:
```bash
./fft_visualizer <path_to_your_audio_file.wav>
```

## Contributing
Contributions are welcome! Feel free to submit a pull request or open an issue.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

