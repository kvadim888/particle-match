# Particle

**Файли:** `localization/src/Particle.hpp`, `localization/src/Particle.cpp`

## Призначення

Представляє одну частинку у фільтрі частинок. Кожна частинка -- це гіпотеза про позицію БПЛА на карті з координатами `(x, y)`, ймовірністю, вагою та набором афінних конфігурацій.

## Конфігурація

Замість статичних полів, частинка зберігає посилання на спільну конфігурацію:

| Поле | Тип | Опис |
|------|-----|------|
| `config` | `shared_ptr<ParticleConfig>` | Спільна конфігурація: напрямок, центр карти, кроки обертання |
| `s_initial` | `shared_ptr<vector<float>>` | Спільний вектор кроків масштабування |

Структура `ParticleConfig` (файл `localization/src/ParticleConfig.hpp`):

| Поле | Тип | Опис |
|------|-----|------|
| `direction` | `double` | Глобальний напрямок БПЛА (з IMU/компаса) |
| `mapCenter` | `Point2i` | Центр карти (половина розмірів) |
| `r_initial` | `vector<float>` | Початкові кроки обертання: 13 значень від -3*step до +3*step |
| `r_step` | `float` | Крок обертання = 0.05 рад (~2.86 град) |

## Поля екземпляра

| Поле | Тип | Опис |
|------|-----|------|
| `x`, `y` | `int` | Координати частинки на карті (пікселі) |
| `probability` | `float` | Поточна ймовірність (ковзне середнє за останні 5 ітерацій) |
| `weight` | `float` | Нормалізована вага після ресемплінгу |
| `samplingFactor` | `float` | Кумулятивний фактор для вибірки (1 - cumulative_weight) |
| `correlation` | `float` | Значення кореляції з картою |
| `configs` | `vector<MatchConfig>` | Набір афінних конфігурацій для оцінки |
| `bestTransform` | `Mat` | Найкраще знайдене афінне перетворення |
| `accumulatedProbability` | `float` | Накопичена ймовірність для ковзного середнього |
| `oldProbabilities` | `vector<float>` | Історія ймовірностей (до 5 останніх) |
| `iteration` | `uint32_t` | Лічильник ітерацій для середнього |

## Ключові методи

### Конструктор
```cpp
Particle(int x, int y, const std::shared_ptr<ParticleConfig>& config);
```
Створює частинку з координатами, посиланням на спільну конфігурацію та початковою ймовірністю 1.0. Одразу генерує набір конфігурацій через `updateConfigs()`.

Підтримує move-семантику: `Particle(Particle&&) noexcept = default`.

### propagate
```cpp
void propagate(const cv::Point2f& movement);
```
Зсуває частинку згідно з вектором руху з додаванням гаусівського шуму:
- Якщо рух = (0,0) -- одометрія втрачена, шум за `alpha * min_movement`
- Інакше -- пропорційний гаусівський шум (`alpha=4.0`)
- Після зсуву перебудовує конфігурації

### updateConfigs
```cpp
void updateConfigs();
```
Генерує повний набір афінних конфігурацій = `scale_steps^2 * rotation_steps * r2_steps`:
- Масштаби: з `s_initial` (спільний вектор)
- Обертання r1: 13 кроків навколо поточного `config->direction`
- Обертання r2: 3 кроки (-3*step, 0, +3*step)
- Трансляція: позиція частинки відносно `config->mapCenter`

### setProbability
```cpp
void setProbability(float probability);
```
Використовує ковзне середнє за останні 5 ітерацій для згладжування ймовірності. Це стабілізує оцінку частинки між кадрами.

### serialize
```cpp
std::string serialize(int binSize);
```
Серіалізує позицію в рядок типу `"120x240"` з квантуванням за `binSize`. Використовується для KLD-семплювання -- визначення унікальних бінів.

### evaluate
```cpp
double evaluate(cv::Mat& image, cv::Mat& templ, cv::Mat& xs, cv::Mat& ys);
```
Оцінка частинки: конвертує конфігурації в афінні матриці, обчислює відстані через `FAsTMatch::evaluateConfigs`, зберігає найкраще перетворення.

### mapTransformation / staticTransformation
```cpp
cv::Mat mapTransformation() const;
cv::Mat staticTransformation() const;
```
- `mapTransformation`: матриця обертання навколо позиції частинки з поточним масштабом та напрямком (-75 градусів корекції)
- `staticTransformation`: матриця обертання навколо фіксованого центру (320, 240)

### getCorners
```cpp
std::vector<cv::Point> getCorners() const;
```
Повертає 4 кути прямокутника 640x480, трансформовані через `mapTransformation`. Використовується для візуалізації.

## Оператори

- `operator<` / `operator>` -- порівняння за `samplingFactor` (для сортування)
