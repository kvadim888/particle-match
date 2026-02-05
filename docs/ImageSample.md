# ImageSample

**Файли:** `localization/src/ImageSample.hpp`, `localization/src/ImageSample.cpp`

## Призначення

Ефективне семплювання зображень для швидкого обчислення подібності. Замість порівняння всіх пікселів, вибирається підмножина точок і обчислюється нормалізована кореляція.

## Поля

| Поле | Тип | Опис |
|------|-----|------|
| `sample` | `vector<float>` | Вектор значень пікселів (нормалізованих по середньому) |
| `squared_sum` | `double` | Сума квадратів значень: `Σ(val - mean)^2` |
| `standart_deviation` | `double` | Стандартне відхилення: `sqrt(squared_sum)` |

## Конструктори

### З попередньо обчисленим середнім
```cpp
ImageSample(const cv::Mat& image, const std::vector<cv::Point>& samplePoints, float average);
```
Зчитує пікселі за заданими точками, віднімає `average`, обчислює `squared_sum` та `standart_deviation`. Використовується для шаблону (середнє вже відоме).

### З автоматичним обчисленням середнього
```cpp
ImageSample(const cv::Mat& image, const std::vector<cv::Point>& samplePoints);
```
Спершу зчитує всі пікселі, обчислює середнє, потім нормалізує. Два проходи по точках.

### З афінним перетворенням
```cpp
ImageSample(const cv::Mat& image, const std::vector<cv::Point>& samplePoints,
            const cv::Mat& rotation, const cv::Point& offset, float average);
```
Для семплювання з карти: трансформує кожну точку через матрицю обертання з зсувом. Компенсує центр зображення (320, 240). Використовується при оцінці частинок.

### З афінним перетворенням та автосередним
```cpp
ImageSample(const cv::Mat& image, const std::vector<cv::Point>& samplePoints,
            const cv::Mat& rotation, const cv::Point& offset);
```
Аналогічно, але обчислює середнє автоматично (два проходи).

## Ключовий метод

### calcSimilarity
```cpp
double calcSimilarity(const ImageSample& other) const;
```
Обчислює нормалізований коефіцієнт кореляції між двома семплами:

```
similarity = Σ(sample_a[i] * sample_b[i]) / (std_a * std_b)
```

Результат в діапазоні `[-1.0, 1.0]`:
- `1.0` -- ідеальна відповідність
- `0.0` -- немає кореляції
- `-1.0` -- зворотна кореляція

## Оптимізація

- Точки семплювання обчислюються один раз і сортуються за `(y, x)` для кращої локальності кешу
- Використовується ~10% пікселів шаблону
- Семпл шаблону обчислюється один раз; семпли карти -- для кожної частинки паралельно через TBB
