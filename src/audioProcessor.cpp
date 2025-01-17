#include "audioProcessor.hpp"

#include <iomanip>
/* Reading and writing audio files using ffmpeg */
void AudioProcessor::readAudioWithFFmpeg() {
    size_t readSamples = 0;
    const int sample = 48 * 1000;
    const size_t totalSamples = audioData.duration * sample;
    audioData.samples.resize(totalSamples);
    audioData.header.sampleRate = sample;

    const std::string ffmpegInputCmd =
        "ffmpeg -i " + inputFile + " -ar " + std::to_string(sample) + " -f s16le -ac 1 -";
    if (FILE *pipein = popen(ffmpegInputCmd.c_str(), "r")) {
        readSamples = fread(audioData.samples.data(), sizeof(int16_t), totalSamples, pipein);
        pclose(pipein);

        if (readSamples == 0) {
            throw std::runtime_error("Failed to read input audio");
        }
    } else {
        throw std::runtime_error("Failed to open input audio");
    }
}

void AudioProcessor::writeAudioWithFFmpeg() {
    const int sample = 48 * 1000;
    const size_t readSamples = audioData.samples.size();
    const std::vector<int16_t> &outputBuffer = audioData.samples;
    const std::string ffmpegOutputCmd =
        "ffmpeg -y -f s16le -ar " + std::to_string(sample) + " -ac 1 -i - " + outputFile;

    if (FILE *pipeout = popen(ffmpegOutputCmd.c_str(), "w")) {
        fwrite(outputBuffer.data(), sizeof(int16_t), readSamples, pipeout);
        pclose(pipeout);
    } else {
        throw std::runtime_error("Failed to open output pipe");
    }
}

void AudioProcessor::resample(uint32_t expectedSampleRate) {
    if (audioData.header.sampleRate == expectedSampleRate) {
        return;
    }
    uint32_t oldSampleRate = audioData.header.sampleRate;
    float ratio = static_cast<float>(expectedSampleRate) / oldSampleRate;
    size_t newLength = static_cast<size_t>(std::ceil(audioData.samples.size() * ratio));
    std::vector<int16_t> output(newLength);

    // Linear interpolation
    for (size_t i = 0; i < newLength; i++) {
        float pos = i / ratio;
        size_t idx = static_cast<size_t>(pos);
        float frac = pos - idx;
        output[i] = (idx + 1 < audioData.samples.size())
                        ? static_cast<int16_t>(
                              audioData.samples[idx] +
                              frac * (audioData.samples[idx + 1] - audioData.samples[idx]))
                        : audioData.samples[idx];
    }
    audioData.samples = output;
    audioData.header.sampleRate = expectedSampleRate;
    audioData.header.dataSize = output.size() * sizeof(int16_t);
    audioData.header.chunkSize = audioData.header.dataSize + 36;
    audioData.header.byteRate =
        audioData.header.sampleRate * audioData.header.numChannels * sizeof(int16_t);
    audioData.duration = calculateDuration(audioData.header);
    /* we don't need to change this coz we are not changing the number of channels
    newHeader.blockAlign = newHeader.numChannels * sizeof(int16_t); */

    std::cout << "Resampled audio from " << oldSampleRate << " to " << expectedSampleRate << "Hz"
              << std::endl;
}

void AudioProcessor::readRawAudioFile(bool headerOnly) {
    std::ifstream file(inputFile, std::ios::binary);
    if (!file) throw std::runtime_error("File not found!");
    if (!file.is_open()) throw std::runtime_error("Failed to open file!");

    file.read(reinterpret_cast<char *>(&audioData.header), sizeof(WAVHeader));
    if (std::string(audioData.header.chunkID, 4) != "RIFF" ||
        std::string(audioData.header.format, 4) != "WAVE")
        throw std::runtime_error("Not a valid WAV file!");

    while (std::string(audioData.header.dataID, 4) != "data") {
        file.seekg(audioData.header.dataSize, std::ios::cur);
        file.read(audioData.header.dataID, 4);
        file.read(reinterpret_cast<char *>(&audioData.header.dataSize),
                  sizeof(audioData.header.dataSize));
        if (file.eof()) {
            std::cerr << "No 'data' chunk found in the WAV file." << std::endl;
            throw std::runtime_error("Invalid WAV file");
        }
    }

    audioData.duration = calculateDuration(audioData.header);
    audioData.samples.resize(audioData.header.dataSize / sizeof(int16_t));
    if (!headerOnly)
        file.read(reinterpret_cast<char *>(audioData.samples.data()), audioData.header.dataSize);
    file.close();
}

void AudioProcessor::writeRawAudioFile() {
    std::ofstream file(outputFile, std::ios::binary);
    file.write(reinterpret_cast<char *>(&audioData.header), sizeof(WAVHeader));
    file.write(reinterpret_cast<char *>(audioData.samples.data()), audioData.header.dataSize);
    file.close();
}

void AudioProcessor::processFrame(const int16_t *input, int16_t *output, int frameSize) {
    std::vector<int16_t> frame(input, input + frameSize);
    size_t expectedFrameSize = audioData.header.sampleRate / 1000 * MAX_PROCESSABLE_MS;

    if (frameSize < (int)expectedFrameSize) {
        frame.resize(expectedFrameSize, 0);
    }
    agcManager->process(frame);
    std::copy(frame.begin(), frame.begin() + frameSize, output);
}

void AudioProcessor::performAGC() {
    size_t frameCount = 0;
    size_t readSamples = audioData.samples.size();
    size_t frameSize = audioData.header.sampleRate / 1000 * MAX_PROCESSABLE_MS;
    std::vector<int16_t> outputBuffer(readSamples);

    if (readSamples == 0) {
        throw std::runtime_error("No audio samples found");
    }
    bool supportedSampleRate = false;
    for (uint32_t i : SUPPORTED_SAMPLE_RATES) {
        if (audioData.header.sampleRate == i) {
            supportedSampleRate = true;
            break;
        }
    }
    if (!supportedSampleRate) {
        throw std::runtime_error("Unsupported sample rate");
    }

    agcManager = std::make_unique<AGC>(audioData.header.sampleRate);
    auto processStart = std::chrono::steady_clock::now();

    std::cout << "Processing audio..." << std::endl;
    double percentage = 0.0;
    for (size_t i = 0; i < readSamples; i += frameSize) {
        size_t remainingSamples = std::min(frameSize, readSamples - i);
        processFrame(audioData.samples.data() + i, outputBuffer.data() + i, remainingSamples);
        percentage = (static_cast<double>(i) / readSamples) * 100;
        std::cout << "Processed " << std::fixed << std::setprecision(2) << percentage << "%\r";
        frameCount++;
    }
    audioData.samples = std::move(outputBuffer);
    auto processEnd = std::chrono::steady_clock::now();

    std::cout
        << "Time taken to process " << frameCount << " frames: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(processEnd - processStart).count()
        << "ms" << std::endl;
}