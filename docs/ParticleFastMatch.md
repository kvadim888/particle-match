# ParticleFastMatch

**Файли:** `localization/src/ParticleFastMatch.hpp`, `localization/src/ParticleFastMatch.cpp`

## Призначення

Центральний клас системи, який поєднує фільтр частинок (Particle Filter) з алгоритмом FAsT-Match для локалізації БПЛА. Наслідує `fast_match::FAsTMatch`.

## Ієрархія наслідування

```
fast_match::FAsTMatch (localization/FAsT-Match/)
    └── ParticleFastMatch (localization/src/)
```

## Перерахування (Enums)

### MatchMode
Режим обчислення подібності між зображеннями:
- `PearsonCorrelation` -- кореляція Пірсона, інваріантна до фотометричних умов (за замовчуванням)
- `BriskMatch` -- пошук за BRISK-дескрипторами (потребує GPU)
- `ORBMatch` -- пошук за ORB-дескрипторами (потребує GPU)

### ConversionMode
Режим конвертації подібності в ймовірність:
- `HPRELU` -- Half Parametric Rectified Linear Unit
- `GLF` -- Generalized Logistic Function (нормалізована)
- `Softmax` -- експоненціальна функція

## Ключові поля

| Поле | Тип | Опис |
|------|-----|------|
| `particles` | `Particles` | Колекція частинок |
| `conversionMode` | `ConversionMode` | Поточний режим конвертації ймовірності |
| `matching` | `MatchMode` | Поточний режим пошуку подібності |
| `templateSample` | `ImageSample` | Попередньо обчислений семпл шаблону |
| `samplingPoints` | `vector<Point>` | Точки семплювання (10% пікселів шаблону) |
| `lowBound` | `float` | Нижня межа активації кореляції |
| `kld_error` | `float` | Допустима помилка KLD-семплювання |
| `binSize` | `int` | Розмір бін для KLD |
| `ztable` | `vector<float>` | Z-таблиця для статистичних розрахунків |
| `zvalue` | `float` | Z-значення для заданого квантиля |
| `minParticles` | `int` | Мінімальна кількість частинок (50) |

## Ключові методи

### Конструктор
```cpp
ParticleFastMatch(
    const cv::Point2i& startLocation,  // початкова позиція на карті
    const cv::Size& mapSize,           // розмір карти
    double radius,                      // радіус розсіювання частинок
    float epsilon,                      // точність (визначає к-сть точок семплювання)
    int particleCount,                  // початкова к-сть частинок
    float quantile_,                    // квантиль для KLD
    float kld_error_,                   // помилка KLD
    int bin_size_,                      // розмір бін
    bool use_gaussian                   // гаусівське vs рівномірне розподілення
);
```
Ініціалізує частинки, будує Z-таблицю, обчислює кількість точок семплювання за формулою `10 / epsilon^2`, створює детектор для вибраного режиму.

### filterParticles
```cpp
vector<Point> filterParticles(const cv::Point2f &movement, cv::Mat &bestTransform);
```
Основний цикл фільтру частинок:
1. Сортує частинки за ймовірністю
2. У циклі KLD-семплювання: вибірка частинки -> пропагація -> серіалізація в бін
3. Кількість частинок адаптивно визначається за KLD-формулою
4. Паралельна оцінка всіх частинок через TBB (`parallel_for_each`)
5. Обчислює кореляцію між семплом карти та шаблоном
6. Конвертує кореляцію в ймовірність
7. Нормалізує ваги частинок

### filterParticlesAffine (GPU)
```cpp
vector<Point> filterParticlesAffine(const cv::Point2f &movement, cv::Mat &bestTransform);
```
GPU-версія: додатково обчислює повне афінне перетворення для кожної частинки. Доступна тільки з `USE_OPENCV_CUDA`.

### evaluateConfigs
```cpp
vector<double> evaluateConfigs(Mat& templ, vector<AffineTransformation>& affine_matrices,
                               Mat& xs, Mat& ys, bool photometric_invariance);
```
Оцінка конфігурацій: для кожної афінної матриці обчислює відстань до шаблону. Фотометрично інваріантний режим нормалізує по середньому та стандартному відхиленню. Використовує TBB для паралельності.

### calculateSimilarity
```cpp
float calculateSimilarity(cv::Mat im) const;
```
Обчислює подібність між зображеннями залежно від `MatchMode`:
- `PearsonCorrelation`: `cv::matchTemplate` з `TM_CCOEFF_NORMED`
- `BriskMatch`/`ORBMatch`: детекція ключових точок, обчислення дескрипторів, GPU-матчинг

### convertProbability
```cpp
float convertProbability(float in) const;
```
Конвертує числове значення подібності в ймовірність:
- **HPRELU**: кусково-лінійна функція з нижньою межею `lowBound`
- **GLF**: `A + (K-A) / (C + Q*exp(-B*x))^(1/v)`, нормалізована до `glf(1.0)`
- **Softmax**: `exp(x)` (нормалізація відбувається в `Particles::normalize`)

### setTemplate / setImage
Перевизначають методи `FAsTMatch`:
- `setTemplate`: ініціалізує семплінг-точки (10% пікселів), обчислює `templateSample`
- `setImage`: встановлює зображення та будує padded-версію для безпечного доступу

### buildZTable
Зчитує файл `ztable.data` -- таблицю z-значень нормального розподілу для KLD-семплювання.

## Залежності

- `FastMatch.hpp` -- базовий клас
- `Particles.hpp` -- контейнер частинок
- `AffineTransformation.hpp` -- афінні матриці
- `Utilities.hpp` -- допоміжні функції
- `ImageSample.hpp` -- семплювання зображень
- Intel TBB -- паралельне обчислення
