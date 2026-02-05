# FastMatcherThread

**Файли:** `localization/src/FastMatcherThread.hpp`, `localization/src/FastMatcherThread.cpp`

## Призначення

Обгортка для асинхронного виконання FAsT-Match алгоритму в окремому потоці. Дозволяє паралельну обробку із синхронізацією через mutex.

## Поля

| Поле | Тип | Опис |
|------|-----|------|
| `matcher` | `FAsTMatch` | Екземпляр FAsT-Match алгоритму |
| `directionPrecision` | `double` | Точність пошуку по обертанню (за замовчуванням π/4) |
| `mtex` | `std::mutex` | М'ютекс для синхронізації |
| `lock` | `std::unique_lock<std::mutex>` | Блокування для м'ютекса |
| `processingResult` | `cv::Point2f` | Результат обчислень |
| `resultAvailable` | `bool` | Чи доступний результат |
| `scaleDownFactor` | `double` | Фактор зменшення масштабу (0.5) |

## Методи

### match (синхронний)
```cpp
cv::Point2f match(cv::Mat image, cv::Mat templ, double direction);
```
Виконує пошук шаблону в зображенні синхронно. Повертає знайдену позицію.

### matchAsync (асинхронний)
```cpp
void matchAsync(cv::Mat image, cv::Mat templ, double direction);
```
Запускає пошук в окремому потоці. Результат можна отримати через `getResultIfAvailable`.

### getResultIfAvailable
```cpp
bool getResultIfAvailable(cv::Point2f& result);
```
Перевіряє чи завершилось обчислення і повертає результат. Неблокуючий виклик.

### isRunning
```cpp
bool isRunning();
```
Перевіряє чи виконується обчислення зараз.

## Примітка

У поточній реалізації основний алгоритм (`ParticleFastMatch`) використовує TBB `parallel_for_each` для паралельної оцінки частинок, а `FastMatcherThread` залишається як альтернативний механізм паралельності.
