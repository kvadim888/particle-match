# ParticleFilterWorkspace

**Файли:** `localization/exec/ParticleFilterWorkspace.hpp`, `localization/exec/ParticleFilterWorkspace.cpp`

## Призначення

Менеджер життєвого циклу фільтру частинок. Організовує ініціалізацію, оновлення, візуалізацію та збір результатів. Є посередником між основною програмою (`dataset-test.cpp`) та алгоритмом (`ParticleFastMatch`).

## Поля

| Поле | Тип | Опис |
|------|-----|------|
| `pfm` | `shared_ptr<ParticleFastMatch>` | Екземпляр алгоритму |
| `svoCurPosition` | `cv::Point` | Поточна позиція SVO (візуальна одометрія) |
| `direction` | `double` | Поточний напрямок БПЛА (з IMU) |
| `startLocation` | `cv::Point` | Початкова позиція |
| `map` | `cv::Mat` | Зображення карти |
| `corners` | `vector<Point>` | Кути найкращого шаблону |
| `bestTransform` | `cv::Mat` | Найкраще афінне перетворення |
| `currentScale` | `float` | Поточний масштаб |
| `bestView` | `cv::Mat` | Зображення з найкращої частинки |
| `writeImageToDisk` | `bool` | Чи зберігати кадри на диск |
| `displayImage` | `bool` | Чи показувати вікно попереднього перегляду |
| `outputDirectory` | `string` | Директорія для збереження зображень |
| `svoCoordinates` | `shared_ptr<LocalCartesian>` | Локальна декартова система координат |
| `affineMatching` | `bool` | Чи використовувати GPU-афінний пошук |

## Ключові методи

### initialize
```cpp
void initialize(const MetadataEntry &metadata);
```
Ініціалізація фільтра:
1. Зчитує напрямок з IMU кватерніона (`toRPY().getZ()`)
2. Створює локальну систему координат (GeographicLib::LocalCartesian) з GPS
3. Створює `ParticleFastMatch` з параметрами:
   - Початкова позиція на карті
   - Радіус розсіювання: 500 пікселів
   - epsilon: 0.1 (~1000 точок семплювання)
   - 200 початкових частинок
   - KLD: quantile=0.99, error=0.5, binSize=5
   - Гаусівський розподіл
4. Встановлює шаблон та зображення карти
5. Обчислює масштаб за висотою та FOV

### update
```cpp
void update(const MetadataEntry &metadata);
```
Оновлення для кожного кадру:
1. Обчислює вектор руху з SVO через `getMovementFromSvo`
2. Оновлює масштаб за поточною висотою
3. Оновлює напрямок з IMU
4. Встановлює новий шаблон (аеро-зображення)
5. Запускає `filterParticles` (або `filterParticlesAffine` на GPU)

### preview
```cpp
bool preview(const MetadataEntry &metadata, cv::Mat image, std::stringstream &stringOutput) const;
```
Візуалізація та збір статистики:
1. Отримує зважену оцінку позиції
2. Вирізає ROI карти 3000x2000 навколо оцінки
3. Малює частинки на карті
4. Малює контур найкращого шаблону (червоний прямокутник)
5. Малює ground truth (жовтий маркер) та оцінку (білий маркер)
6. Показує вид з камери та найкращу частинку
7. Обчислює та записує помилку позиції
8. Опціонально зберігає на диск / показує у вікні
9. Повертає `false` при натисканні ESC

### getMovementFromSvo
```cpp
cv::Point getMovementFromSvo(const MetadataEntry &metadata);
```
Обчислює вектор руху з візуальної одометрії:
1. Конвертує SVO-позицію в GPS через `LocalCartesian::Reverse`
2. Конвертує GPS в пікселі карти
3. Обчислює зсув відносно попередньої позиції
4. **Важливо**: використовує відстань з одометрії, але **напрямок з компаса** (IMU більш надійний)

### updateScale
```cpp
void updateScale(float hfov, float altitude, uint32_t imageWidth);
```
Обчислює масштаб: `scale = tan(hfov/2) * altitude / (imageWidth/2)`. Встановлює діапазон масштабу [0.9*scale, 1.1*scale].

## Виведення даних

CSV формат виводу:
```
Iteration, ImageName, ParticleCount, PosX, PosY, Distance, SVODistance
```
- `Distance` -- евклідова відстань між оцінкою та ground truth
- `SVODistance` -- евклідова відстань між SVO та ground truth
