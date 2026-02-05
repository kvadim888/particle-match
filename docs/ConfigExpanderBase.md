# ConfigExpanderBase та GridConfigExpander

**Файли:**
- `localization/src/ConfigExpanderBase.hpp`, `localization/src/ConfigExpanderBase.cpp`
- `localization/src/GridConfigExpander.hpp`, `localization/src/GridConfigExpander.cpp`

## ConfigExpanderBase

### Призначення

Абстрактний базовий клас, що визначає стратегію генерації та розширення конфігурацій для пошуку. Реалізує патерн **Стратегія (Strategy)**.

### Поля

| Поле | Тип | Опис |
|------|-----|------|
| `net` | `unique_ptr<MatchNet>` | Мережа параметрів простору пошуку |

### Абстрактні методи

#### createListOfConfigs
```cpp
virtual std::vector<MatchConfig> createListOfConfigs(cv::Size templ_size, cv::Size image_size) = 0;
```
Генерує початковий набір конфігурацій на основі розмірів шаблону та зображення.

#### randomExpandConfigs
```cpp
virtual std::vector<MatchConfig> randomExpandConfigs(std::vector<MatchConfig> &configs,
                                                      int level, int no_of_points,
                                                      float delta_factor) = 0;
```
Розширює набір конфігурацій випадковим чином для наступного рівня ієрархічного пошуку.

---

## GridConfigExpander

### Призначення

Конкретна реалізація `ConfigExpanderBase`, що генерує конфігурації на регулярній сітці (grid) по всьому простору пошуку.

### Ієрархія наслідування

```
ConfigExpanderBase
    └── GridConfigExpander
```

### Методи

#### createListOfConfigs
Генерує сітку конфігурацій перебираючи всі комбінації:
- Трансляція X (кроки з `MatchNet`)
- Трансляція Y
- Обертання
- Масштаб

#### randomExpandConfigs
Навколо кожної існуючої конфігурації генерує випадкові варіації з зменшеним діапазоном (визначається `delta_factor` та `level`).

### Використання

`GridConfigExpander` підключається до `FAsTMatch` через `configExpander`:
```cpp
configExpander = std::make_shared<GridConfigExpander>();
```
