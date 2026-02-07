# Particles

**Файли:** `localization/src/Particles.hpp`, `localization/src/Particles.cpp`

## Призначення

Контейнер (колекція) частинок. Використовує **композицію** з `std::vector<Particle>` (замість наслідування) і додає методи для ініціалізації, пропагації, оцінки, нормалізації та ресемплінгу.

## Архітектура

```
class Particles {
    std::vector<Particle> data_;         // внутрішній контейнер
    shared_ptr<ParticleConfig> particleConfig;  // спільна конфігурація
    shared_ptr<vector<float>> s_initial; // спільні кроки масштабу
    std::mt19937 rng_;                   // генератор випадкових чисел
};
```

Клас надає повний ітераторний інтерфейс (begin/end, rbegin/rend, operator[], size, empty) через делегування до `data_`.

## Поля

| Поле | Тип | Опис |
|------|-----|------|
| `data_` | `vector<Particle>` | Внутрішній вектор частинок |
| `particleConfig` | `shared_ptr<ParticleConfig>` | Спільна конфігурація для всіх частинок |
| `s_initial` | `shared_ptr<vector<float>>` | Спільний вектор кроків масштабування |
| `rng_` | `std::mt19937` | Генератор випадкових чисел (стандартна бібліотека) |

## Ключові методи

### init
```cpp
void init(cv::Point2i startLocation, const cv::Size mapSize, double radius,
          int particleCount, bool use_gaussian);
```
Ініціалізація частинок:
- Розміщує `particleCount` частинок у колі радіусом `radius` навколо `startLocation`
- `use_gaussian=true`: гаусівський розподіл (щільніше до центру)
- `use_gaussian=false`: рівномірний розподіл
- Використовує суму двох випадкових величин для трикутного розподілу радіуса
- Перевіряє дублікати позицій через `isLocationOccupied`
- Початкова ймовірність кожної частинки = 0.5
- Налаштовує `ParticleConfig::setMapDimensions` для центру карти

### propagate
```cpp
void propagate(const cv::Point2f& movement, float alpha = 2.f);
```
Масова пропагація: до кожної частинки застосовує рух із випадковим масштабуванням `alpha * uniform(-1, 1)`.

### normalize
```cpp
void normalize();
```
Нормалізує ваги частинок:
1. Обчислює суму ймовірностей
2. Вага кожної частинки = `probability / sum`
3. `samplingFactor = 1 - cumulative_weight`

Це створює структуру для стохастичного ресемплінгу: частинки з вищою ймовірністю займають більший діапазон `samplingFactor`.

### sample
```cpp
Particle sample();
```
Вибірка однієї частинки пропорційно до ваг:
- Генерує випадковий поріг `[0, 1)`
- Якщо поріг < 0.5 -- шукає з початку (ітератор вперед)
- Якщо поріг >= 0.5 -- шукає з кінця (зворотній ітератор)
- Повертає копію знайденої частинки
- Fallback: повертає копію останньої частинки якщо жодна не відповідає порогу

### evaluate
```cpp
std::vector<cv::Point> evaluate(cv::Mat image, cv::Mat templ, int no_of_points);
```
Оцінка всіх частинок: генерує випадкові точки семплювання, послідовно оцінює кожну частинку, повертає кути найкращого перетворення.

### getWeightedSum
```cpp
cv::Point2i getWeightedSum() const;
```
Зважена сума позицій -- фінальна оцінка позиції БПЛА:
```
result.x = Σ(particle.x * particle.weight)
result.y = Σ(particle.y * particle.weight)
```

### setScale
```cpp
void setScale(float min, float max, uint32_t steps = 5);
```
Встановлює діапазон масштабів для всіх частинок. Генерує `steps` рівномірних значень від `min` до `max` і передає їх у кожну частинку.

## Послідовність роботи

```
init() -> [цикл:] propagate() -> evaluate/normalize -> sample() -> getWeightedSum()
```
