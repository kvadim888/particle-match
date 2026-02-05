# ConfigVisualizer

**Файли:** `localization/src/ConfigVisualizer.hpp`, `localization/src/ConfigVisualizer.cpp`

## Призначення

Відповідає за візуалізацію конфігурацій та частинок на зображенні карти. Використовується для відлагодження та демонстрації роботи алгоритму.

## Методи

### visualiseConfigs
```cpp
void visualiseConfigs(cv::Mat image, const std::vector<fast_match::MatchConfig>& configs);
```
Відображає конфігурації (афінні перетворення) на зображенні. Кожна конфігурація малюється як точка або прямокутник з відповідним кольором.

### visualiseParticles
```cpp
void visualiseParticles(cv::Mat image, const Particles& particles, const cv::Point2i& offset);
```
Відображає частинки на зображенні карти:
- Кожна частинка малюється як точка
- Колір/розмір може відображати ймовірність
- `offset` -- зсув для відображення в ROI карти

## Використання

Викликається з `ParticleFastMatch::visualizeParticles`, який передає зображення та зсув для поточного вікна перегляду.
