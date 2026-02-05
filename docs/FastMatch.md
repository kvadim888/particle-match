# FastMatch

**Файли:** `localization/src/FastMatch.hpp`, `localization/src/FastMatch.cpp`

## Призначення

Розширена обгортка алгоритму FAsT-Match (Fast Affine Template Matching). Перевизначає оригінальний клас `fast_match::FAsTMatch` з підтримкою конфігураційних розширювачів, візуалізатора та GPU.

## Ієрархія наслідування

```
fast_match::FAsTMatch (localization/FAsT-Match/FAsTMatch.h)
    └── fast_match::FAsTMatch (localization/src/FastMatch.hpp) -- розширена версія
        └── ParticleFastMatch
```

Зверніть увагу: у проєкті є **два** файли `FAsTMatch.h` -- оригінальний (в субмодулі) та розширений (в `localization/src/`). Розширена версія замінює оригінальну.

## Ключові відмінності від оригіналу

| Аспект | Оригінал (submodule) | Розширена версія |
|--------|---------------------|------------------|
| `apply()` | Повертає `vector<Point2f>` | Повертає `void`, зберігає стан |
| Конфігурації | Через `MatchNet` | Через `ConfigExpanderBase` (стратегія) |
| Візуалізація | Відсутня | Через `ConfigVisualizer` |
| GPU | Ні | Опціональна через `USE_CV_GPU` |
| Методи | Повний пошук | Ієрархічний з `calculateLevel()` |

## Ключові поля

| Поле | Тип | Опис |
|------|-----|------|
| `configExpander` | `shared_ptr<ConfigExpanderBase>` | Стратегія розширення конфігурацій |
| `visualizer` | `ConfigVisualizer` | Візуалізатор конфігурацій та частинок |
| `no_of_points` | `int` | Кількість точок семплювання |
| `level` | `int` | Поточний рівень ієрархічного пошуку |
| `original_image` | `Mat` | Оригінальне зображення |
| `imageGray`, `templGray` | `Mat` | Сірі версії зображень |
| `best_config` | `MatchConfig` | Найкраща знайдена конфігурація |
| `best_trans` | `Mat` | Найкраще афінне перетворення |
| `configs` | `vector<MatchConfig>` | Поточні конфігурації |

## Ключові методи

### init
```cpp
virtual void init(float epsilon, float delta, bool photometric_invariance,
                  float min_scale, float max_scale);
```
Ініціалізація параметрів пошуку.

### apply
```cpp
virtual void apply(Mat &image, Mat &templ, double &best_distance,
                   float min_rotation, float max_rotation);
```
Запуск пошуку шаблону в зображенні.

### calculateLevel
```cpp
bool calculateLevel();
```
Один крок ієрархічного пошуку: розширює конфігурації, оцінює, фільтрує кращі.

### evaluateConfigs (static, оригінал)
```cpp
static vector<double> evaluateConfigs(Mat& image, Mat& templ, vector<Mat>& affine_matrices,
                                      Mat& xs, Mat& ys, bool photometric_invariance);
```
Статичний метод оцінки конфігурацій з матрицями `cv::Mat`.

### configsToAffine
```cpp
vector<Mat> configsToAffine(vector<MatchConfig>& configs, vector<bool>& insiders);
```
Конвертує конфігурації в афінні матриці, фільтруючи ті, що виходять за межі зображення.

### setImage / setTemplate
```cpp
virtual void setImage(const Mat &image);
virtual void setTemplate(const Mat &templ);
```
Встановлюють зображення/шаблон, обчислюють середні значення, конвертують у градації сірого.

## Залежності

- `FAsT-Match/MatchNet.h` -- параметри простору пошуку
- `FAsT-Match/MatchConfig.h` -- конфігурація афінного перетворення
- `ConfigExpanderBase.hpp` -- абстрактна стратегія розширення
- `ConfigVisualizer.hpp` -- візуалізація
