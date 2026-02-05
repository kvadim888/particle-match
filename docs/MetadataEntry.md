# MetadataEntry

**Файли:** `dataset_reader/include/fastmatch-dataset/MetadataEntry.hpp`, `dataset_reader/src/classes/MetadataEntry.cpp`

## Призначення

Структура даних, що представляє один кадр набору даних БПЛА. Містить зображення, GPS-координати, дані IMU, ground truth та позицію на карті.

## Поля

| Поле | Тип | Опис |
|------|-----|------|
| `imageFileName` | `string` | Ім'я файлу зображення |
| `imageFullPath` | `string` | Повний шлях до файлу зображення |
| `latitude` | `double` | GPS широта |
| `longitude` | `double` | GPS довгота |
| `altitude` | `double` | Відносна висота (барометр) |
| `imuOrientation` | `Quaternion` | Орієнтація з IMU (кватерніон) |
| `groundTruthPose` | `Vector3d` | Еталонна позиція (x, y, z) |
| `groundTruthOrientation` | `Quaternion` | Еталонна орієнтація |
| `mapLocation` | `cv::Point2i` | Позиція на карті в пікселях |
| `svoPose` | `Vector3d` | Позиція з візуальної одометрії (SVO) |
| `map` | `cv::Mat` | Зображення карти |
| `imageBuffer` | `cv::Mat` | Завантажене зображення з камери |
| `mapper` | `shared_ptr<Map>` | Об'єкт карти для конвертації координат |

## Методи

### getImage
```cpp
cv::Mat getImage() const;
```
Повертає зображення з камери в градаціях сірого.

### getImageColored
```cpp
cv::Mat getImageColored() const;
```
Повертає кольорове зображення з камери (BGR).

### getImageSharpened
```cpp
cv::Mat getImageSharpened(bool smooth = false) const;
```
Повертає покращене зображення (збільшена різкість, опціональне згладжування).

## Формат CSV

Поля читаються з `metadata.csv` з колонками:
```
Filename, Latitude, Longitude, RelativeAltitude,
ImuX, ImuY, ImuZ, ImuW,
PoseX, PoseY, PoseZ,
OrientationX, OrientationY, OrientationZ, OrientationW,
MapX, MapY,
SvoX, SvoY, SvoZ
```
