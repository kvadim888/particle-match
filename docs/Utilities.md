# Utilities

**Файли:** `localization/src/Utilities.hpp`, `localization/src/Utilities.cpp`

## Призначення

Статичний клас з допоміжними функціями для обробки зображень, обчислення кореляції, генерації шуму та геометричних перетворень.

## Методи

### Обробка зображень

#### preprocessImage
```cpp
static cv::Mat preprocessImage(const cv::Mat &image);
```
Конвертує зображення в сіре, нормалізує значення до `[0.0, 1.0]`, робить розміри непарними.

#### makeOdd
```cpp
static cv::Mat makeOdd(cv::Mat &image);
```
Обрізає зображення до непарних розмірів (потрібно для симетричних перетворень навколо центру).

#### photometricNormalization
```cpp
static cv::Mat photometricNormalization(cv::Mat scene, cv::Mat templ);
```
Фотометрична нормалізація: лінійне перетворення шаблону щоб відповідав статистикам сцени (mean/sigma matching).

### Обчислення кореляції

#### calculateCorrelation
```cpp
static double calculateCorrelation(cv::Mat scene, cv::Mat templ);
```
Ручне обчислення нормалізованої кросс-кореляції через скалярні операції OpenCV. Обробляє NaN через `zeroIfNan`.

#### calculateCorrCoeff
```cpp
static float calculateCorrCoeff(cv::Mat scene, cv::Mat templ);
```
Обчислення коефіцієнта кореляції через `cv::matchTemplate` з `TM_CCOEFF_NORMED`. Більш надійний та швидкий варіант.

### Геометрія та перетворення

#### calcCorners
```cpp
static std::vector<cv::Point> calcCorners(cv::Size image_size, cv::Size templ_size, cv::Mat &affine);
```
Обчислює 4 кути прямокутника шаблону після афінного перетворення. Враховує центрування шаблону та зображення.

#### calculateLocationInMap
```cpp
static cv::Point calculateLocationInMap(const cv::Size &image_size, const cv::Size &templ_size,
                                         const cv::Mat &affine, const cv::Point &templPoint);
```
Перетворює точку шаблону в координати карти через афінну матрицю.

#### configsToAffine
```cpp
static std::vector<cv::Mat> configsToAffine(std::vector<MatchConfig> &configs, std::vector<bool> &insiders,
                                             const cv::Size &imageSize, const cv::Size &templSize);
```
Масова конвертація конфігурацій в афінні матриці з фільтрацією за межами. Паралелізується через TBB.

#### extractWarpedMapPart
```cpp
static cv::Mat extractWarpedMapPart(cv::InputArray map, const cv::Size &templ_size, const cv::Mat &affine);
```
Вирізає та трансформує частину карти за афінною матрицею. Використовує `cv::warpAffine` з інверсним маппінгом.

#### extractMapPart
```cpp
static cv::Mat extractMapPart(const cv::Mat &map, const cv::Size &size, const cv::Point &position,
                               double angle, float scale);
```
Вирізає прямокутник з карти з обертанням та масштабуванням. Обчислює ROI більше за потрібний для безпечного обертання.

#### getMapRoiMask
```cpp
static cv::Mat getMapRoiMask(const cv::Size &image_size, const cv::Size &templ_size, cv::Mat &affine);
```
Створює бінарну маску ROI для афінно-трансформованого шаблону.

### Генерація шуму

#### normal_dist
```cpp
static double normal_dist();
```
Нормальний розподіл N(0, 0.333). Використовує `std::minstd_rand`.

#### gausian_noise
```cpp
static double gausian_noise(double u);
```
Гаусівський шум з обмеженням `[-u, u]`. Правило трьох сигм з клемпінгом `[-1, 1]`.

#### uniform_dist
```cpp
static double uniform_dist();
```
Рівномірний розподіл U(0, 1).

### Геометрична математика

#### eulerAnglesToRotationMatrix
```cpp
static cv::Mat eulerAnglesToRotationMatrix(const cv::Point3d &angles);
```
Конвертує кути Ейлера (roll, pitch, yaw) в матрицю обертання 3x3: `R = Rz * Ry * Rx`.

#### intersectPlaneV3
```cpp
static cv::Point3d intersectPlaneV3(const cv::Point3d &a, const cv::Point3d &b,
                                     const cv::Point3d &p_co, const cv::Point3d &p_no, float epsilon);
```
Знаходить точку перетину прямої (a, b) з площиною (p_co, p_no). Використовується для проєкції камери на площину землі.

### Діагностика

#### matType
```cpp
static std::string matType(int type);
```
Повертає рядкове представлення типу `cv::Mat` (напр. `"8UC3"`, `"32FC1"`).
