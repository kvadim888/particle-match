# Output CSV Format

This document describes the CSV output produced by `dataset-match` (see `localization/exec/dataset-test.cpp`).
The file contains one row per processed dataset entry and uses the following columns.

## Columns

| Column | Description | Units |
| --- | --- | --- |
| `Iteration` | Zero-based index of the processed entry. | count |
| `ImageName` | Image filename from the dataset metadata. | n/a |
| `ParticleCount` | Number of particles used by the filter. | count |
| `RelativePosX` | Predicted X position relative to the start location. | map px |
| `RelativePosY` | Predicted Y position relative to the start location. | map px |
| `LocationError` | Euclidean distance between ground truth map location and predicted location. | map px |
| `SVODistance` | Euclidean distance between ground truth map location and the SVO-reported position. | map px |

## Header formatting

The header is emitted with explicit units inside brackets, for example:

```
"Iteration","ImageName","ParticleCount [count]","RelativePosX [map px]","RelativePosY [map px]","LocationError [map px]","SVODistance [map px]"
```

## Row formatting

Each data row uses plain CSV values; `ParticleCount`, `RelativePosX`, and `RelativePosY` are integers, while
`LocationError` and `SVODistance` are formatted with two decimal places.
