#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <fftw3.h>
#include <sndfile.h>
#include <cmath>
#include <iostream>
#include <vector>

const int SAMPLE_RATE = 44100;
const int N = 1024; // FFT size
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

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

    // Check if the file is mono
    if (sfinfo.channels != 1) {
        std::cerr << "Error: only mono audio is supported" << std::endl;
        sf_close(file);
        return 1;
    }

    // Allocate input and output arrays for FFT
    double* in = (double*)fftw_malloc(sizeof(double) * N);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (N / 2 + 1));
    fftw_plan plan = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);

    // Create a window
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "FFT Visualizer");

    // Load the audio file for playback
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(argv[1])) {
        std::cerr << "Error: could not load audio file for playback" << std::endl;
        sf_close(file);
        return 1;
    }

    sf::Sound sound(buffer);
    sound.play();

    std::cout << "Sound started playing." << std::endl;

    // Create a vector to store our visualization bars
    const int NUM_BARS = 64;
    std::vector<sf::RectangleShape> bars(NUM_BARS);
    float barWidth = static_cast<float>(WINDOW_WIDTH) / NUM_BARS;
    for (int i = 0; i < NUM_BARS; ++i) {
        bars[i].setFillColor(sf::Color::Green);
        bars[i].setPosition(i * barWidth, WINDOW_HEIGHT);
        bars[i].setSize(sf::Vector2f(barWidth - 1, 0));
    }

    // Main loop
    sf::Clock clock;
    while (window.isOpen() && sound.getStatus() == sf::Sound::Playing) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Get current playback position
        sf::Time playbackPosition = sound.getPlayingOffset();
        sf_count_t samplePosition = static_cast<sf_count_t>(playbackPosition.asSeconds() * SAMPLE_RATE);

        // Seek to the current position in the file
        sf_seek(file, samplePosition, SEEK_SET);

        // Read samples from the WAV file
        sf_count_t frames_read = sf_readf_double(file, in, N);
        if (frames_read < N) {
            for (int i = frames_read; i < N; ++i) {
                in[i] = 0.0;
            }
        }

        // Execute FFT
        fftw_execute(plan);

        // Update visualization
        for (int i = 0; i < NUM_BARS; ++i) {
            double magnitude = 0;
            int start = i * (N / 4 / NUM_BARS);
            int end = (i + 1) * (N / 4 / NUM_BARS);
            for (int j = start; j < end; ++j) {
                magnitude += std::sqrt(out[j][0] * out[j][0] + out[j][1] * out[j][1]);
            }
            magnitude /= (end - start);
            
            float height = std::min(static_cast<float>(magnitude * 5.0 / N), 1.0f) * WINDOW_HEIGHT;
            bars[i].setSize(sf::Vector2f(barWidth - 1, -height));
        }

        // Clear the window and draw the bars
        window.clear(sf::Color::Black);
        for (const auto& bar : bars) {
            window.draw(bar);
        }
        window.display();

        // Limit the frame rate
        sf::sleep(sf::milliseconds(16)); // Approximately 60 FPS
    }

    std::cout << "Playback ended or window closed." << std::endl;

    // Clean up
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
    sf_close(file);
    return 0;
}
