# Rainbow Table Builder

A high-performance Rainbow Table generator that supports both CPU and GPU-based hash computation for UUIDs using SHA3. This tool is designed to efficiently process large volumes of UUIDs and generate their corresponding hash values.

## Features

- Dual implementation support:
  - CPU-based hash computation using OpenMP for parallelization
  - GPU-based hash computation using CUDA for massive parallelization
- Custom SHA3 hashing algorithm that runs on NVIDIA GPUs in parallel
- Compact binary output format option for efficient storage
- Configurable batch processing for optimized memory usage
- File comparison utility to verify hash outputs with CPU-based version
- Built-in UUID processing capabilities

## Prerequisites

To build and run this project, you need:

- C++ compiler with OpenMP support (for CPU implementation)
- CUDA toolkit and NVCC (for GPU implementation)
- Python 3 (optional - for dataset generation)
- Make build system

## Building

Use the provided Makefile to build the project:

```bash
# Build all targets (CPU, GPU, and file comparator)
make all

# Build specific targets
make          # Build CPU version only
make gpu      # Build GPU version only
make filecomp # Build file comparator only

# Build static CUDA version
make static

# Clean built files
make clean
```

## Usage

1. Generate test UUID dataset (optional - a sample dataset is already present in this repository):
```bash
make dataset
```

2. Run the hash generator:
```bash
# CPU Version
./binary <input_file>

# GPU Version
./binary_cuda <input_file>
```

3. Compare output files (if needed):
```bash
./filecomp <file1> <file2>
```

## Output Formats

The tool supports two output formats (needs to be toggled in the main source file):

1. Text Format (default): Newline-delimited hex strings
2. Binary Format (compressed): Raw byte format for efficient storage

Output files are saved in the `hashed_output` directory with the following naming convention:
- CPU Version: `<input_filename>_hashed_byCPU.[txt|bin]`
- GPU Version: `<input_filename>_hashed_byGPU.[txt|bin]`

## Implementation Details

- Uses SHA3 (Keccak) hashing algorithm
- Processes UUIDs in configurable batch sizes for memory efficiency
- GPU implementation utilizes CUDA thread blocks for parallel processing
- Supports endianness conversion for cross-platform compatibility

## Performance Optimization

The implementation includes several optimizations:
- Batch processing to minimize I/O, as well as CPU<->GPU communication overhead
- OpenMP parallelization for CPU version
- CUDA-based parallel processing for GPU version
- Compact binary output option for reduced storage requirements

## License

This project is licensed under the MIT License - see the LICENSE file for details.
