### FFMPEG Command used in processing function

``` ffmpeg -i inputFile -ar SAMPLE_RATE -f s16le -ac 1 - ```
- -i input.wav: Takes your input audio file
- -ar 48000: Converts the audio to 48000 Hz sampling rate (48kHz)
- -f s16le: Outputs the audio as 16-bit signed integers (raw audio data)
- -ac 1: Converts to mono (single channel) audio
- The final - means "output to stdout" instead of a file


```ffmpeg -y -f s16le -ar SAMPLE_RATE -ac 1 -i - outputFile```
- -y: Overwrites output file if it exists
- -f s16le: Tells FFmpeg the input is 16-bit signed integers
- -ar 48000: Sets the sampling rate to 48kHz
- -ac 1: Specifies mono audio
- -i -: Reads from stdin

### WAV File Header Structure

```wl
Offset  Size  Field         Description
---------------------------------------
0       4     chunkID       "RIFF"
4       4     chunkSize     File size minus 8 bytes
8       4     format        "WAVE"
12      4     subchunk1ID   "fmt "
16      4     subchunk1Size Size of format chunk (16 for PCM)
20      2     audioFormat   Audio format (1 for PCM)
22      2     numChannels   Number of channels
24      4     sampleRate    Sampling rate (e.g., 44100)
28      4     byteRate      Bytes per second
32      2     blockAlign    Bytes per sample frame
34      2     bitsPerSample Bits per sample
36      4     subchunk2ID   "data"
40      4     subchunk2Size Size of audio data
44      -     Audio Data    Actual audio samples
```

### Linear Interpolation

This is a method of estimating values between two known values assuming a linear relationship between the two. The formula is:

```plaintext
x1 - x0     x - x0
───────  =  ──────
y1 - y0     y - y0
```

where (x0, y0) and (x1, y1) are known values, and (x, y) are the values to be estimated.

**Resampling in Audio using Linear Interpolation**

Since the x range will be constantly increasing by 1, we can simplify the formula to:

```plaintext
y = y0 + (y1 - y0) * (x - x0)
```

where `frac = x - x0`:

```plaintext
y = y0 + (y1 - y0) * frac
```