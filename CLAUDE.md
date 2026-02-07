# CLAUDE.md - AI Assistant Guide for particle-match

This document provides guidance for AI assistants working with the particle-match codebase.

## Project Overview

**particle-match** is a C++ implementation of UAV (Unmanned Aerial Vehicle) localization using a particle filter algorithm combined with FAsT-Match (Fast Affine Template Matching). The system tracks UAV position on maps by matching aerial images against reference satellite/map imagery.

**Primary Use Case**: Real-time position estimation of drones by comparing onboard camera images to georeferenced map tiles.

**Academic Context**: This is a research project with accompanying dataset available from Zenodo (https://zenodo.org/record/1211730).

## Build System

### Prerequisites

- CMake 3.12+
- C++14 compiler (GCC recommended)
- Required libraries:
  - OpenCV 4.0+ (computer vision)
  - Boost 1.70+ (system, program_options, filesystem, thread)
  - Eigen3 (linear algebra)
  - GeographicLib (GPS coordinate conversions)
  - GDAL (GeoTIFF support)
  - libcurl (HTTP requests)
  - Intel TBB (parallelization)

### Build Commands

```bash
# From project root
mkdir build
cd build
cmake ..
make
```

### Build Outputs

- `libdatasetreader.so` - Dataset reading library
- `libfastmatch.so` - Matching algorithm library
- `dataset-match` - Main executable for UAV localization
- `image-sampler` - Image sampling utility

### Optional GPU Support

GPU acceleration is disabled by default. Enable with:
```bash
cmake -DUSE_OPENCV_CUDA=ON ..
```

## Project Structure

```
particle-match/
├── localization/               # Core algorithm implementation
│   ├── src/                    # Main source files
│   ├── exec/                   # Executable programs
│   └── FAsT-Match/             # Git submodule (template matching)
├── dataset_reader/             # Dataset handling library
│   ├── include/fastmatch-dataset/  # Public headers
│   └── src/classes/            # Implementation
├── cmake/                      # CMake modules
├── templates/                  # Template storage (empty)
├── CMakeLists.txt              # Main build configuration
├── download_data.sh            # Dataset download script
└── ztable.data                 # Statistical lookup table
```

## Key Source Files

### Core Algorithm (`localization/src/`)

| File | Purpose |
|------|---------|
| `ParticleFastMatch.hpp/cpp` | Main class combining particle filter with template matching |
| `Particle.hpp/cpp` | Individual particle (position, probability, weight) |
| `Particles.hpp/cpp` | Particle collection manager with resampling |
| `FastMatch.hpp/cpp` | FAsT-Match algorithm wrapper |
| `AffineTransformation.hpp/cpp` | 2D affine transformation support |
| `Utilities.hpp/cpp` | Image processing helpers, correlation computation |
| `ImageSample.hpp/cpp` | Efficient image sampling for similarity calculation |
| `GridConfigExpander.hpp/cpp` | Grid-based configuration expansion |
| `ConfigVisualizer.hpp/cpp` | Visualization tools |
| `FastMatcherThread.hpp/cpp` | Thread pool for parallel evaluation |

### Dataset Handling (`dataset_reader/`)

| File | Purpose |
|------|---------|
| `MetadataEntry.hpp` | UAV frame data structure (image, GPS, IMU, pose) |
| `MetadataEntryReader.hpp` | CSV dataset reader |
| `GeotiffMap.hpp` | GeoTIFF map with coordinate transforms |
| `Map.hpp` | Base map class with geo-region bounds |
| `Vector3d.hpp` | 3D vector for position |
| `Quaternion.hpp` | Orientation quaternion |

### Main Application (`localization/exec/`)

| File | Purpose |
|------|---------|
| `dataset-test.cpp` | Main entry point - runs particle filter on UAV dataset |
| `ParticleFilterWorkspace.hpp/cpp` | Workspace managing particle filter lifecycle |

## Code Conventions

### Style Guidelines

- **Header Guards**: Use `#pragma once`
- **Includes**: Standard library first, then third-party, then local headers
- **Namespaces**: Use `cv::` for OpenCV, `fs::` for boost::filesystem, `po::` for boost::program_options
- **Class Organization**: Public members first, then protected, then private
- **Comments**: Brief creation comments with author and date at file tops

### Naming Conventions

- **Classes**: PascalCase (e.g., `ParticleFastMatch`, `MetadataEntry`)
- **Methods**: camelCase (e.g., `filterParticles`, `setImage`)
- **Variables**: camelCase (e.g., `templateSample`, `lowBound`)
- **Constants/Enums**: PascalCase for enum values (e.g., `PearsonCorrelation`, `HPRELU`)
- **Member variables**: No prefix, but occasionally trailing underscore for constructor parameters

### Common Patterns

- **OpenCV Mat**: Used extensively for images and matrices
- **Smart Pointers**: `std::shared_ptr` for shared ownership
- **Boost Program Options**: CLI argument parsing
- **Boost Filesystem**: Cross-platform file operations

## Algorithm Overview

The particle filter localization works as follows:

1. **Initialization**: Particles distributed around estimated starting GPS location
2. **Propagation**: Particles move based on estimated UAV motion (from IMU/odometry)
3. **Evaluation**: Each particle's position evaluated by matching aerial image against map region
4. **Probability Conversion**: Similarity scores converted to probabilities using configurable functions:
   - `HPRELU` - Half Parametric Rectified Linear Unit
   - `GLF` - Generalized Linear Function
   - `Softmax`
5. **Resampling**: KLD (Kullback-Leibler Distance) sampling for particle selection
6. **Output**: Weighted mean of particle positions gives estimated location

### Matching Modes

- `PearsonCorrelation` - Photometric invariant correlation (default)
- `BriskMatch` - BRISK feature-based matching
- `ORBMatch` - ORB feature-based matching

## Running the Application

### Download Sample Dataset
```bash
bash download_data.sh
```

### Run Localization
```bash
./build/dataset-match \
    --map-image dataset/urban/m_3809028_ne_15_1_20140720/m_3809028_ne_15_1_20140720.tif \
    --dataset dataset/UL-200 \
    --preview
```

### Command-Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `--map-image, -m` | Path to GeoTIFF map image | Required |
| `--dataset, -d` | Path to dataset directory | Required |
| `--results, -r` | Results output directory | `results` |
| `--skip-rate, -s` | Skip N entries per iteration | 10 |
| `--preview, -p` | Display preview window | Off |
| `--write-images, -w` | Write preview images to disk | Off |
| `--write-histograms, -H` | Write correlation histograms | Off |
| `--correlation-bound, -c` | Correlation activation bound | 0.2 |
| `--conversion-method, -M` | Probability conversion (hprelu/glf/softmax) | glf |

## Testing

The project includes several test executables in `localization/exec/`:

- `test-image-sampling.cpp` - Image sampling tests
- `particle-match-test.cpp` - Particle matching tests
- `match-test.cpp` - Template matching tests
- `map-tests.cpp` - Map functionality tests
- `test-slopes.cpp` - Slope calculation tests

No formal testing framework (Google Test, Catch2, etc.) is used. Tests are standalone programs.

## Dependencies & External Libraries

### Git Submodules

The project uses FAsT-Match as a git submodule in `localization/FAsT-Match/`. After cloning, initialize with:
```bash
git submodule update --init --recursive
```

### Key External Data

- `ztable.data` - Z-table lookup values for KLD sampling statistics
- Datasets from Zenodo contain:
  - GeoTIFF reference maps
  - UAV flight imagery with GPS/IMU metadata in CSV format

## Common Development Tasks

### Adding New Particle Evaluation Methods

1. Add enum value to `ParticleFastMatch::MatchMode`
2. Implement calculation in `ParticleFastMatch::calculateSimilarity()`
3. Update CLI parsing in `dataset-test.cpp` if exposing via command line

### Modifying Probability Conversion

1. Add enum value to `ParticleFastMatch::ConversionMode`
2. Implement in `ParticleFastMatch::convertProbability()`
3. Update `conversionModeString()` for logging

### Working with Maps

- Maps are loaded via `GeotiffMap` class
- Coordinate conversion: GPS (lat/lon) ↔ pixel coordinates via GeographicLib
- Subregion extraction via `Map::getSubregion()`

## Performance Considerations

- Image sampling uses pre-computed sampling points for efficiency
- Thread pool (`FastMatcherThread`) enables parallel particle evaluation
- Optional GPU support via OpenCV CUDA for feature matching
- KLD sampling adapts particle count based on distribution complexity

## Detailed Documentation

The `docs/` directory contains per-class documentation with fields, methods, algorithms, and usage patterns:

- [docs/README.md](docs/README.md) - Navigation index for all documentation
- [docs/Architecture.md](docs/Architecture.md) - System architecture, data flow diagrams, component interactions
- [docs/Glossary.md](docs/Glossary.md) - Glossary of domain-specific terms (KLD, HPRELU, UTM, etc.)
- Per-class documentation for all 15+ classes in the codebase

## Troubleshooting

### Missing GeographicLib

The CMake searches multiple paths for GeographicLib. If not found:
```bash
sudo apt-get install libgeographic-dev
```

### OpenCV Version Issues

Project updated for OpenCV 4.x compatibility. If encountering API issues, check OpenCV version matches 4.0+.

### GCC Include Errors

Recent commits fixed missing includes for newer GCC versions. Ensure you have the latest code.
