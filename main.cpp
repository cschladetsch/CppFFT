#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <fftw3.h>
#include <sndfile.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>

const int SAMPLE_RATE = 44100;
const int N = 4096; // Large FFT size for higher frequency resolution
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;
const int NUM_BARS = 128; // Number of bars in the visualizer

// Function to map intensity to color for visual effects
sf::Color getColor(float percentage) {
    if (percentage < 0.2f)
        return sf::Color(0, 255 * (percentage / 0.2f), 255);
    else if (percentage < 0.4f)
        return sf::Color(0, 255, 255 * ((0.4f - percentage) / 0.2f));
    else if (percentage < 0.6f)
        return sf::Color(255 * ((percentage - 0.4f) / 0.2f), 255, 0);
    else if (percentage < 0.8f)
        return sf::Color(255, 255 * ((0.8f - percentage) / 0.2f), 0);
    else
        return sf::Color(255, 0, 255 * ((percentage - 0.8f) / 0.2f));
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <audio_file.wav>" << std::endl;
        return 1;
    }

    // Open the audio file using libsndfile
    SNDFILE* file;
    SF_INFO sfinfo;
    file = sf_open(argv[1], SFM_READ, &sfinfo);
    if (!file) {
        std::cerr << "Error: could not open file " << argv[1] << std::endl;
        return 1;
    }

    // Only mono audio is supported
    if (sfinfo.channels != 1) {
        std::cerr << "Error: only mono audio is supported" << std::endl;
        sf_close(file);
        return 1;
    }

    // FFTW allocations for input and output
    double* in = (double*)fftw_malloc(sizeof(double) * N);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (N / 2 + 1));
    fftw_plan plan = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);

    // Initialize the SFML window
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "FFT Visualizer");

    // Load the audio file for playback
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(argv[1])) {
        std::cerr << "Error: could not load audio file for playback" << std::endl;
        sf_close(file);
        fftw_destroy_plan(plan);
        fftw_free(in);
        fftw_free(out);
        return 1;
    }

    sf::Sound sound(buffer);
    sound.play();

    // Visualizer setup
    std::vector<sf::RectangleShape> bars(NUM_BARS);
    float barWidth = static_cast<float>(WINDOW_WIDTH) / NUM_BARS;
    for (int i = 0; i < NUM_BARS; ++i) {
        bars[i].setPosition(i * barWidth, WINDOW_HEIGHT);
        bars[i].setSize(sf::Vector2f(barWidth - 1, 0));
    }

    while (window.isOpen() && sound.getStatus() == sf::Sound::Playing) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Seek the audio file to the current playback position
        sf::Time playbackPosition = sound.getPlayingOffset();
        sf_count_t samplePosition = static_cast<sf_count_t>(playbackPosition.asSeconds() * SAMPLE_RATE);
        sf_seek(file, samplePosition, SEEK_SET);

        // Read the audio samples and perform the FFT
        sf_count_t frames_read = sf_readf_double(file, in, N);
        if (frames_read < N) {
            for (int i = frames_read; i < N; ++i) {
                in[i] = 0.0;
            }
        }

        fftw_execute(plan);

        // Process the FFT output and update the visualizer
        for (int i = 0; i < NUM_BARS; ++i) {
            double magnitude = 0;
            int start = i * (N / 4 / NUM_BARS);
            int end = (i + 1) * (N / 4 / NUM_BARS);
            for (int j = start; j < end; ++j) {
                magnitude += std::sqrt(out[j][0] * out[j][0] + out[j][1] * out[j][1]);
            }
            magnitude /= (end - start);

            // Amplify the magnitude scaling for a more responsive visualizer
            float scaledMagnitude = std::log(1 + magnitude) * 350.0f / N;

            // Map the scaled magnitude to a bar height
            float height = std::min(scaledMagnitude, 1.0f) * WINDOW_HEIGHT;
            bars[i].setSize(sf::Vector2f(barWidth - 1, -height));
            bars[i].setFillColor(getColor(static_cast<float>(i) / NUM_BARS));
        }

        // Clear the window and draw the updated bars
        window.clear(sf::Color::Black);
        for (const auto& bar : bars) {
            window.draw(bar);
        }
        window.display();

        // Minimize sleep time to allow more frequent updates
        sf::sleep(sf::milliseconds(1)); // Reduce sleep to make it more responsive
    }

    std::cout << "Playback ended or window closed." << std::endl;

    // Clean up FFTW and close the audio file
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
    sf_close(file);
    return 0;
}

