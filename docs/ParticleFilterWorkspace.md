# Runtime Architecture

**Файли:**
- `localization/runtime/RuntimeBase.hpp`, `RuntimeBase.cpp` -- спільна логіка
- `localization/runtime/WorkspaceRuntime.hpp`, `WorkspaceRuntime.cpp` -- GUI режим
- `localization/exec/dataset-test.cpp` -- HeadlessRuntime (headless режим)

## Призначення

Менеджери життєвого циклу фільтру частинок. Організовують ініціалізацію, оновлення та збір результатів. Є посередниками між основною програмою (`dataset-test.cpp`) та алгоритмом (`ParticleFastMatch`).

## Ієрархія класів

```
RuntimeBase (abstract)
├── WorkspaceRuntime   -- GUI візуалізація через PreviewRenderer
└── HeadlessRuntime    -- headless режим, тільки CSV вивід
```

## RuntimeBase

Базовий клас, що містить спільну логіку для обох режимів:

### Поля

| Поле | Тип | Опис |
|------|-----|------|
| `core_` | `ParticleFilterCore` | Обгортка алгоритму ParticleFastMatch |
| `svoCurPosition_` | `cv::Point` | Поточна позиція SVO (візуальна одометрія) |
| `direction_` | `double` | Поточний напрямок БПЛА (з IMU) |
| `startLocation_` | `cv::Point` | Початкова позиція |
| `corners_` | `vector<Point>` | Кути найкращого шаблону |
| `bestTransform_` | `cv::Mat` | Найкраще афінне перетворення |
| `bestView_` | `cv::Mat` | Зображення з найкращої частинки |
| `currentScale_` | `float` | Поточний масштаб |
| `svoCoordinates_` | `shared_ptr<LocalCartesian>` | Локальна декартова система координат |
| `motionModel_` | `MotionModelSvo` | Модель руху (SVO одометрія) |
| `scaleModel_` | `ScaleModel` | Модель масштабу (висота -> масштаб) |
| `affineMatching_` | `bool` | Чи використовувати GPU-афінний пошук |

### Методи

#### initialize
```cpp
void initialize(const MetadataEntry &metadata, const ParticleFilterConfig &config);
```
Ініціалізація фільтра:
1. **Валідація** конфігурації через `config.validate()`
2. Зчитує напрямок з IMU кватерніона (`toRPY().getZ()`)
3. Створює локальну систему координат (GeographicLib::LocalCartesian) з GPS
4. Створює `ParticleFastMatch` через `ParticleFilterCore::initialize`
5. Обчислює масштаб за висотою та FOV (з реальної ширини шаблону)

#### update
```cpp
void update(const MetadataEntry &metadata);
```
Оновлення для кожного кадру:
1. Обчислює вектор руху з SVO через `MotionModelSvo::getMovementFromSvo`
2. Оновлює масштаб за поточною висотою
3. Оновлює напрямок з IMU
4. Встановлює новий шаблон (аеро-зображення)
5. Запускає `filterParticles` (або `filterParticlesAffine` на GPU)

### Чисто віртуальні методи
```cpp
virtual bool preview(...) = 0;
virtual bool isDisplayImage() const = 0;
virtual void setDisplayImage(bool) = 0;
virtual void setWriteImageToDisk(bool) = 0;
virtual void setOutputDirectory(const std::string&) = 0;
```

## WorkspaceRuntime

GUI-режим з `PreviewRenderer`:

```cpp
bool preview(const MetadataEntry &metadata, const cv::Mat &image, std::stringstream &stringOutput) override;
```
Створює `RenderContext` і делегує відображення в `PreviewRenderer::render()`.

## HeadlessRuntime

Headless-режим без GUI залежностей:

```cpp
bool preview(const MetadataEntry &metadata, const cv::Mat &image, std::stringstream &stringOutput) override;
```
Обчислює відстань до ground truth та записує результати через `ResultWriter::appendRow()`. Попереджає один раз якщо `--write-images` передано в headless режимі.

## Допоміжні класи

### ParticleFilterCore
**Файли:** `localization/core/ParticleFilterCore.hpp`, `ParticleFilterCore.cpp`

Обгортка `ParticleFastMatch`, що спрощує ініціалізацію з `ParticleFilterConfig`.

### ParticleFilterConfig
**Файл:** `localization/core/ParticleFilterConfig.hpp`

Структура конфігурації з методом `validate()`:

| Поле | Тип | За замовчуванням | Валідація |
|------|-----|------------------|-----------|
| `radius` | `double` | 500.0 | > 0 |
| `epsilon` | `float` | 0.1 | (0, 1) |
| `particleCount` | `int` | 200 | > 0 |
| `quantile` | `float` | 0.99 | (0, 1] |
| `kld_error` | `float` | 0.5 | > 0 |
| `binSize` | `int` | 5 | > 0 |
| `use_gaussian` | `bool` | true | -- |

### MotionModelSvo
**Файли:** `localization/models/MotionModelSvo.hpp`, `MotionModelSvo.cpp`

Обчислює вектор руху з візуальної одометрії (SVO). Повертає `SvoMovementResult{movement, updatedPosition}` замість мутації out-параметра.

### ScaleModel
**Файли:** `localization/models/ScaleModel.hpp`, `ScaleModel.cpp`

Обчислює масштаб: `scale = tan(hfov/2) * altitude / (imageWidth/2)`. Встановлює діапазон `[kScaleMarginLow * scale, kScaleMarginHigh * scale]` (за замовчуванням 0.9-1.1).

### PreviewRenderer
**Файли:** `localization/io/PreviewRenderer.hpp`, `PreviewRenderer.cpp`

GUI рендеринг візуалізації. Приймає `RenderContext` та `stringstream` для виводу. Малює частинки, ground truth, оцінку позиції, найкращий шаблон.

### ResultWriter
**Файли:** `localization/io/ResultWriter.hpp`, `ResultWriter.cpp`

Статичний клас для запису CSV результатів (заголовки та рядки даних).
