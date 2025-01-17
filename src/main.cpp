#include <fstream>

#include "audioProcessor.hpp"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    AudioProcessor audioProcessor(inputFile, outputFile);
    audioProcessor.processWithCustomResampler();
    // audioProcessor.processWithCustomResampler(16000);
    // audioProcessor.processWithFFmpeg();

    return 0;
}