# Audio Gain Control (AGC - WebRTC)

## Overview
This project provides a simple tool for processing audio files with features like:
- **Automatic Gain Control (AGC):** Dynamically adjusts the volume of the audio to ensure consistent levels using WebRTC's GainController2.
- **Custom Resampling:** Converts audio to a desired sample rate (e.g., 48000 Hz) - using linear interpolation for resampling.
- **Flexible Input/Output:** Supports reading and writing raw audio files, with optional integration of FFmpeg for advanced audio handling.

## How It Works
1. **Audio Input:**
   - Reads raw PCM audio from a file or uses FFmpeg for format decoding.
2. **Processing:**
   - Applies AGC using WebRTC's GainController2.
   - Optionally resamples the audio to a specified sample rate.
3. **Audio Output:**
   - Writes processed audio to a raw PCM file or uses FFmpeg for encoding.

### Build Instructions
1. Ensure you have a C++ compiler that supports C++17 or later and cmake installed.
2. Clone the repository:
   ```bash
   git clone <repository_url>
   cd <repository_folder>
   ```
3.  Initialize the thirdparty agc libs and make sure to have meson and ninja installed.
    ```bash
    cd thirdparty
    sh install.sh
    ```
3. Check the agc/libs folder consist of .so or .a files. If they are present inside x86_64-linux-gnu folder, get them out and place them in the agc/libs folder. Once done,you can go back to the root folder and continue with the build process.

4. Compile the code:
   ```bash
   cmake -S . -B build
   cmake --build build
   ```

### Running the Program
To process an audio file, use the following command:
```bash
./bin/audioProcessor <input_file> <output_file>
```
- `<input_file>`: Path to the raw PCM input file (with a WAV header).
- `<output_file>`: Path to the processed raw PCM output file.

## Dependencies
- **C++17:** For modern C++ features.
- **CMake:** For building the project.
- **Meson and Ninja:** For building the thirdparty agc libs.
- **WebRTC:** For GainController2 and audio buffer management.
- **FFmpeg (optional):** For reading and writing non-raw audio formats.

## Contributing
Feel free to open issues or submit pull requests for enhancements or bug fixes.