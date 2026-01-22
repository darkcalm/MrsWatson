# VST3 Plugin Demo

This directory contains demonstration audio files showing VST3 plugin processing with MrsWatson.

## Files

- **input.wav**: Original audio file (drum brush grooves)
- **output.wav**: Processed audio file using HPL Processor Ultimate.vst3

## Test Command

To reproduce the test, run:

```bash
cd build
./main/mrswatson64 -p "HPL Processor Ultimate" -i "test/demo/input.wav" -o "test/demo/output.wav"
```

## Plugin Location

The test was performed with:
- Plugin: HPL Processor Ultimate.vst3
- Location: `/Library/Audio/Plug-Ins/VST3/HPL Processor Ultimate.vst3`

## Results

- Plugin successfully loaded
- Factory obtained
- Component instance created
- Audio processing completed
- Output file generated successfully
