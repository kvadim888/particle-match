# FAsT-Match Submodule

**Директорія:** `localization/FAsT-Match/`

## Призначення

Git-субмодуль з оригінальною C++ реалізацією алгоритму FAsT-Match (Fast Affine Template Matching). Базується на роботі:

> Simon Korman, Daniel Reichman, Gilad Tsur, Shai Avidan.
> "FAsT-Match: Fast Affine Template Matching", CVPR 2013.

## Файли

### FAsTMatch.h / FAsTMatch.cpp (оригінал)

Оригінальний клас алгоритму:

```cpp
namespace fast_match {
    class FAsTMatch {
        void init(float epsilon, float delta, bool photometric_invariance,
                  float min_scale, float max_scale);
        vector<Point2f> apply(Mat &image, Mat &templ, double &best_distance,
                              float min_rotation, float max_rotation);
    };
}
```

**Алгоритм:**
1. Ініціалізує мережу конфігурацій (`MatchNet`) що покриває весь простір пошуку
2. Генерує список конфігурацій (`createListOfConfigs`) на кожному рівні
3. Оцінює конфігурації через випадкове семплювання (`evaluateConfigs`)
4. Відбирає кращі конфігурації по дистанції (`getGoodConfigsByDistance`)
5. Випадково розширює навколо кращих (`randomExpandConfigs`)
6. Повторює з зменшеним простором пошуку

**Примітка:** У проєкті цей файл **не використовується напряму**. Замість нього використовується розширена версія в `localization/src/FastMatch.hpp`.

### MatchConfig.h / MatchConfig.cpp

Конфігурація одного афінного перетворення:

```cpp
namespace fast_match {
    class MatchConfig {
        float translateX, translateY;  // трансляція
        float rotate1, rotate2;        // два обертання
        float scaleX, scaleY;          // масштаб по осях
        float probability;             // ймовірність
        int id;                        // ідентифікатор частинки
    };
}
```

**Методи:**
- `getAffineMatrix()` -- обчислює матрицю 2x3 з параметрів:
  ```
  A = [sx*cos(r1)*cos(r2) - sy*sin(r1)*sin(r2),  sx*cos(r1)*sin(r2) + sy*sin(r1)*cos(r2),  tx]
      [sx*sin(r1)*cos(r2) + sy*cos(r1)*sin(r2), -sx*sin(r1)*sin(r2) + sy*cos(r1)*cos(r2),  ty]
  ```
- `asMatrix()` -- повертає як 1x6 матрицю
- `fromMatrix()` -- створює конфігурації з матриці

### MatchNet.h / MatchNet.cpp

Мережа (сітка) параметрів простору пошуку:

```cpp
namespace fast_match {
    class MatchNet {
        pair<float,float> boundsTransX, boundsTransY;  // межі трансляції
        pair<float,float> boundsRotate;                 // межі обертання
        pair<float,float> boundsScale;                  // межі масштабу
        float stepsTransX, stepsTransY;                 // кроки трансляції
        float stepsRotate;                              // крок обертання
        float stepsScale;                               // крок масштабу
    };
}
```

**Методи:**
- `getXTranslationSteps()` -- повертає вектор кроків по X
- `getYTranslationSteps()` -- вектор кроків по Y
- `getRotationSteps()` -- вектор кроків обертання
- `getScaleSteps()` -- вектор кроків масштабу
- `operator*` / `operator/` -- масштабування мережі (для зміни рівня)

## Взаємодія з основним проєктом

```
FAsTMatch (submodule, оригінал)
    │
    ├── MatchConfig -- використовується напряму для конфігурацій частинок
    ├── MatchNet -- використовується для GridConfigExpander
    │
    └── FAsTMatch (class) -- замінений розширеною версією в localization/src/FastMatch.hpp
                             (додано ConfigExpander, Visualizer, GPU, ієрархічний пошук)
```

## Ініціалізація субмодуля

```bash
git submodule update --init --recursive
```
