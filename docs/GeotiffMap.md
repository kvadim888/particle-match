# GeotiffMap та Map

**Файли:**
- `dataset_reader/include/fastmatch-dataset/Map.hpp`, `dataset_reader/src/classes/Map.cpp`
- `dataset_reader/include/fastmatch-dataset/GeotiffMap.hpp`, `dataset_reader/src/classes/GeotiffMap.cpp`

## Map (базовий клас)

### Призначення

Базовий клас карти з підтримкою конвертації координат GPS <-> пікселі та вирізання підрегіонів.

### Поля

| Поле | Тип | Опис |
|------|-----|------|
| `geoRegion` | `vector<GeoCoords>(2)` | Географічні координати: [0] -- верхній лівий кут, [1] -- нижній правий кут |
| `image` | `cv::Mat` | Зображення карти |
| `dimensions` | `cv::Size` | Розміри карти в пікселях |
| `scale` | `double` | Масштаб (метри/піксель) |
| `valid` | `bool` | Чи завантажена карта коректно |

### Методи

#### toPixels
```cpp
virtual cv::Point2i toPixels(double latitude, double longitude) const;
```
Конвертує GPS-координати в піксельні координати на карті.

#### toCoords
```cpp
virtual void toCoords(const cv::Point2i& loc, double &latitude, double &longitude);
```
Зворотна конвертація: пікселі -> GPS.

#### subregion
```cpp
cv::Mat subregion(const cv::Size& size, double latitude, double longitude,
                  double heading, double scale = 1.0);
```
Вирізає підрегіон карти навколо заданих GPS-координат з обертанням за напрямком.

#### isWithinMap
```cpp
virtual bool isWithinMap(double latitude, double longitude);
```
Перевіряє чи GPS-координати знаходяться в межах карти.

---

## GeotiffMap (наслідник Map)

### Призначення

Реалізація `Map` для GeoTIFF файлів. Використовує GDAL для читання та GeographicLib для конвертації координат через UTM-проєкцію.

### Ієрархія наслідування

```
Map
    └── GeotiffMap
```

### Додаткові поля

| Поле | Тип | Опис |
|------|-----|------|
| `adfGeoTransform[6]` | `double[6]` | GDAL GeoTransform: 6 коефіцієнтів афінного перетворення пікселі<->координати |
| `zoneNumber` | `int` | Номер UTM-зони |
| `northp` | `bool` | Північна (true) або Південна (false) півкуля |

### Методи

#### open
```cpp
void open(const std::string& filename);
```
Відкриває GeoTIFF файл:
1. Завантажує через GDAL (`GDALOpen`)
2. Парсить проєкцію (UTM зона та півкуля) через OGR regex
3. Читає GeoTransform коефіцієнти
4. Завантажує зображення через OpenCV
5. Обчислює geoRegion (кутові координати)

#### toPixels (override)
```cpp
cv::Point2i toPixels(double latitude, double longitude) const override;
```
GPS -> пікселі через UTM:
1. Конвертує lat/lon в UTM (Easting/Northing) через `GeographicLib::GeoCoords`
2. Застосовує зворотне GeoTransform для отримання пікселів

#### toCoords (override)
```cpp
void toCoords(const cv::Point2i &loc, double &latitude, double &longitude) override;
```
Пікселі -> GPS через UTM:
1. Застосовує GeoTransform для отримання Easting/Northing
2. Конвертує UTM -> lat/lon через GeographicLib

#### pixelCoordinates
```cpp
GeographicLib::GeoCoords pixelCoordinates(const cv::Point2i &loc) const;
```
Повертає об'єкт `GeoCoords` для заданого пікселя.

### GeoTransform формат

```
adfGeoTransform[0] = X координата верхнього лівого кута
adfGeoTransform[1] = роздільна здатність X (метрів/піксель)
adfGeoTransform[2] = обертання (зазвичай 0)
adfGeoTransform[3] = Y координата верхнього лівого кута
adfGeoTransform[4] = обертання (зазвичай 0)
adfGeoTransform[5] = роздільна здатність Y (від'ємна, метрів/піксель)
```

### Типізований вказівник

```cpp
typedef std::shared_ptr<GeotiffMap> GeoMapPtr;
typedef std::shared_ptr<Map> MapPtr;
```
