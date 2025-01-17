#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "audio_buffer.h"
#include "gain_controller2.h"

#define MAX_PROCESSABLE_MS 10
#define SUPPORTED_SAMPLE_RATES {8000, 16000, 32000, 48000}

struct WAVHeader {
    char chunkID[4];  // "RIFF"
    uint32_t chunkSize;
    char format[4];      // "WAVE"
    char subchunkID[4];  // "fmt "
    uint32_t subchunkSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char dataID[4];  // "data"
    uint32_t dataSize;

    void print() {
        std::cout << "chunkID: " << std::string(chunkID, 4) << std::endl;
        std::cout << "chunkSize: " << chunkSize << std::endl;
        std::cout << "format: " << std::string(format, 4) << std::endl;
        std::cout << "subchunkID: " << std::string(subchunkID, 4) << std::endl;
        std::cout << "subchunkSize: " << subchunkSize << std::endl;
        std::cout << "audioFormat: " << audioFormat << std::endl;
        std::cout << "numChannels: " << numChannels << std::endl;
        std::cout << "sampleRate: " << sampleRate << std::endl;
        std::cout << "byteRate: " << byteRate << std::endl;
        std::cout << "blockAlign: " << blockAlign << std::endl;
        std::cout << "bitsPerSample: " << bitsPerSample << std::endl;
        std::cout << "dataID: " << std::string(dataID, 4) << std::endl;
        std::cout << "dataSize: " << dataSize << std::endl;
    }
};

struct AudioData {
    WAVHeader header;
    std::vector<int16_t> samples;
    double duration;
};

class AGC {
   private:
    std::unique_ptr<webrtc::GainController2> gainController_m;  // Use smart pointer
    std::unique_ptr<webrtc::AudioBuffer> audioBuffer_m;         // Use smart pointer
    webrtc::StreamConfig streamConfig_m;
    int sampleRate;

   public:
    AGC(int sampleRate) : streamConfig_m(sampleRate, 1), sampleRate(sampleRate) {
        audioBuffer_m =
            std::make_unique<webrtc::AudioBuffer>(sampleRate, 1, sampleRate, 1, sampleRate, 1);
        initialize();
    }
    ~AGC() = default;

    void initialize() {
        webrtc::AudioProcessing::Config::GainController2 config;
        webrtc::InputVolumeController::Config inputVolumeControllerConfig;
        config.enabled = true;

        // config.input_volume_controller.enabled = true;  enable only if direct mic input is used

        config.adaptive_digital.enabled = true;
        config.adaptive_digital.headroom_db = 5.0f;
        config.adaptive_digital.max_gain_db = 30.0f;
        config.adaptive_digital.initial_gain_db = 10.0f;
        config.adaptive_digital.max_gain_change_db_per_second = 5.0f;
        config.adaptive_digital.max_output_noise_level_dbfs = -50.f;

        config.fixed_digital.gain_db = 2.0f;

        gainController_m = std::make_unique<webrtc::GainController2>(
            config, inputVolumeControllerConfig, sampleRate, 1, false);
    }

    void process(std::vector<int16_t> &frame) {
        size_t expectedSize = (size_t)(sampleRate * MAX_PROCESSABLE_MS / 1000);
        if (frame.empty() || frame.size() != expectedSize) {
            std::cerr << "Invalid frame size" << std::endl;
            return;
        }
        audioBuffer_m->CopyFrom(frame.data(), streamConfig_m);
        gainController_m->Process(std::nullopt, false, audioBuffer_m.get());
        audioBuffer_m->CopyTo(streamConfig_m, frame.data());
    }
};

class AudioProcessor {
   private:
    AudioData audioData;
    std::string inputFile;
    std::string outputFile;
    std::unique_ptr<AGC> agcManager;

    void processFrame(const int16_t *input, int16_t *output, int frameSize);
    void readAudioWithFFmpeg();
    void writeAudioWithFFmpeg();
    void readRawAudioFile(bool headerOnly = false);
    void writeRawAudioFile();
    void performAGC();
    double calculateDuration(const WAVHeader &header) {
        return static_cast<double>(header.dataSize) / (header.byteRate);
    }

   public:
    AudioProcessor(std::string inputFile, std::string outputFile)
        : inputFile(inputFile), outputFile(outputFile) {
        std::cout << "Input file: " << inputFile << std::endl;
        std::cout << "Output file: " << outputFile << std::endl;
    }
    ~AudioProcessor() = default;
    void resample(uint32_t expectedSampleRate);

    void processWithFFmpeg() {
        readRawAudioFile(true);
        readAudioWithFFmpeg();
        performAGC();
        writeAudioWithFFmpeg();
    }

    void processWithCustomResampler(uint32_t expectedSampleRate = 48000) {
        readRawAudioFile();
        resample(expectedSampleRate);
        performAGC();
        writeRawAudioFile();
    }
};
