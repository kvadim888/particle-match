# MetadataEntryReader

**Файли:** `dataset_reader/include/fastmatch-dataset/MetadataEntryReader.hpp`, `dataset_reader/src/classes/MetadataEntryReader.cpp`

## Призначення

Читач набору даних БПЛА. Послідовно читає кадри з CSV-файлу метаданих, завантажує відповідні зображення та заповнює об'єкти `MetadataEntry`.

## Поля

| Поле | Тип | Опис |
|------|-----|------|
| `in` | `ifstream` | Відкритий потік файлу metadata.csv |
| `header` | `vector<string>` | Заголовки колонок CSV |
| `datasetPath` | `string` | Шлях до директорії набору даних |
| `map` | `GeoMapPtr` | Об'єкт GeoTIFF-карти |
| `skipRate` | `uint32_t` | Пропуск кадрів (1 = кожен кадр, 10 = кожен десятий) |
| `lineCounter` | `uint64_t` | Лічильник прочитаних рядків |

## Методи

### openDirectory
```cpp
bool openDirectory(const std::string &datasetDir);
```
Відкриває директорію набору даних:
1. Відкриває `datasetDir/metadata.csv`
2. Читає заголовок (перший рядок)
3. Повертає `true` якщо файл відкрито успішно

### readNextEntry
```cpp
bool readNextEntry(MetadataEntry& metadataEntry);
```
Читає наступний кадр:
1. Скидає `metadataEntry` до стандартного стану
2. Пропускає рядки згідно з `skipRate`
3. Парсить CSV-рядок у map ключ-значення
4. Заповнює метадані через `fillMetadata`
5. Повертає `false` коли файл закінчився

### setMap
```cpp
void setMap(const std::string& mapFile);
```
Створює `GeotiffMap` та відкриває GeoTIFF-файл карти. Карта прив'язується до всіх наступних кадрів.

### setSkipRate
```cpp
void setSkipRate(uint32_t skipRate);
```
Встановлює частоту пропуску кадрів. За замовчуванням 1 (без пропуску). Значення 10 означає, що обробляється кожен 10-й кадр.

## Внутрішні методи

### fillMetadata (private)
Заповнює `MetadataEntry` з map значень CSV:
- Парсить GPS координати (`Latitude`, `Longitude`, `RelativeAltitude`)
- Парсить IMU кватерніон (`ImuX/Y/Z/W`)
- Парсить ground truth (`PoseX/Y/Z`, `OrientationX/Y/Z/W`)
- Парсить позицію на карті (`MapX`, `MapY`)
- Парсить візуальну одометрію (`SvoX/Y/Z`)
- Завантажує зображення через `cv::imread`
- Прив'язує карту та mapper

### parseString / parseLine (private static)
Розбивають CSV-рядки за допомогою `boost::split`.

## Формат набору даних

```
dataset/
├── metadata.csv        # CSV з метаданими кожного кадру
└── images/             # Директорія зі зображеннями
    ├── frame_001.jpg
    ├── frame_002.jpg
    └── ...
```
