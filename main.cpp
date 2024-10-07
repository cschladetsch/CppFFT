#include <SFML/Graphics.hpp>
#include <fftw3.h>
#include <sndfile.h>
#include <cmath>
#include <iostream>
#include <vector>

const int SAMPLE_RATE = 44100;
const int N = 1024; // FFT size

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <audio_file.wav>" << std::endl;
        return 1;
    }

    // Open the WAV file
    SNDFILE* file;
    SF_INFO sfinfo;
    file = sf_open(argv[1], SFM_READ, &sfinfo);
    if (!file) {
        std::cerr << "Error: could not open file " << argv[1] << std::endl;
        return 1;
    }

    // Check if the file is stereo or mono
    if (sfinfo.channels != 1) {
        std::cerr << "Error: only mono audio is supported" << std::endl;
        sf_close(file);
        return 1;
    }

    // Allocate input and output arrays for FFT
    double* in = (double*)fftw_malloc(sizeof(double) * N);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (N / 2 + 1));
    fftw_plan plan = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);

    // Create a window with specific settings
    sf::ContextSettings settings;
    settings.depthBits = 24; // Set depth bits
    settings.stencilBits = 8; // Set stencil bits
    settings.antialiasingLevel = 2; // Enable anti-aliasing

    sf::RenderWindow window(sf::VideoMode(800, 600), "FFT Visualizer", sf::Style::Default, settings);

    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Read samples from the WAV file
        sf_count_t frames_read = sf_readf_double(file, in, N);
        if (frames_read < N) {
            // If fewer frames than N are read, fill the rest with zeros
            for (int i = frames_read; i < N; ++i) {
                in[i] = 0.0;
            }
        }

        // Execute FFT
        fftw_execute(plan);

        // Prepare for drawing
        window.clear(sf::Color::Black);
        sf::VertexArray spectrum(sf::LinesStrip, N / 2);

        // Draw frequency spectrum
        for (int i = 0; i < N / 2; ++i) {
            double magnitude = std::sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
            // Scale x to fit the window width
            float x = static_cast<float>(i) * (800.0f / (N / 2));
            // Normalize and scale the magnitude for visibility
            float y = 600 - static_cast<float>(magnitude * 50 / N);

            // Avoid negative y values
            if (y < 0) y = 0;

            spectrum[i].position = sf::Vector2f(x, y);
            spectrum[i].color = sf::Color::Green;
        }

        window.draw(spectrum);
        window.display();
    }

    // Clean up
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
    sf_close(file);
    return 0;
}

