# dataset-test (Main Application)

**Файл:** `localization/exec/dataset-test.cpp`

## Призначення

Головна точка входу програми. Парсить аргументи командного рядка, завантажує набір даних БПЛА та запускає фільтр частинок для локалізації. Підтримує два режими: GUI (`WorkspaceRuntime`) та headless (`HeadlessRuntime`).

## Аргументи командного рядка

| Опція | Скорочення | Тип | За замовчуванням | Опис |
|-------|-----------|-----|------------------|------|
| `--map-image` | `-m` | string | -- | Шлях до GeoTIFF карти |
| `--dataset` | `-d` | string | Обов'язковий | Шлях до директорії набору даних |
| `--results` | `-r` | string | `"results"` | Назва директорії результатів |
| `--skip-rate` | `-s` | uint32 | 10 | Пропуск кадрів |
| `--preview` | `-p` | flag | Off | Показувати вікно перегляду |
| `--no-gui` | -- | flag | Off | Запуск без GUI (headless) |
| `--write-images` | `-w` | flag | Off | Зберігати зображення на диск |
| `--write-histograms` | `-H` | flag | Off | Записувати гістограми кореляцій |
| `--correlation-bound` | `-c` | float | 0.2 | Нижня межа активації кореляції |
| `--conversion-method` | `-M` | string | `"glf"` | Метод конвертації: `hprelu`, `glf`, `softmax` |
| `--affine-matching` | `-a` | flag | Off | Афінне співставлення (потребує GPU) |
| `--particle-radius` | -- | double | 500.0 | Радіус фільтра частинок |
| `--epsilon` | -- | float | 0.1 | Epsilon фільтра |
| `--particle-count` | -- | int | 200 | Кількість частинок |
| `--quantile` | -- | float | 0.99 | KLD квантиль |
| `--kld-error` | -- | float | 0.5 | KLD похибка |
| `--bin-size` | -- | int | 5 | Розмір біна KLD |
| `--use-gaussian` | -- | bool | true | Гаусівський розподіл |

## Послідовність роботи

```
1. Парсинг аргументів (Boost.Program_Options)
2. Створення ParticleFilterConfig з CLI параметрів + валідація
3. Завантаження GeoTIFF карти через MetadataEntryReader
4. Відкриття директорії набору даних (metadata.csv)
5. Створення директорії результатів з timestamp
6. Вибір runtime (WorkspaceRuntime або HeadlessRuntime)
7. Цикл по кадрах через runDataset(RuntimeBase&, ...):
   ├── Перший кадр: runtime.initialize()
   └── Наступні кадри:
       ├── runtime.update()
       ├── [опціонально] Запис гістограм кореляцій
       ├── runtime.preview()
       └── Запис CSV результатів
```

## Формат результатів

### data.csv
```
"Iteration","ImageName","ParticleCount [count]","RelativePosX [map px]","RelativePosY [map px]","LocationError [map px]","SVODistance [map px]"
0,"frame_001.jpg",200,120,340,15.23,45.67
1,"frame_011.jpg",87,125,345,12.10,42.31
...
```

### histograms.csv (опціонально)
Кожен рядок -- значення кореляцій всіх частинок на поточному кадрі:
```
0.45,0.32,0.67,0.21,...
0.51,0.38,0.71,0.25,...
```

### Зображення (опціонально)
Файли `preview_00001.jpg`, `preview_00002.jpg`, ... -- візуалізація карти з частинками та оцінкою позиції.

## Запуск

```bash
# GUI режим
./build/dataset-match \
    --map-image dataset/urban/m_3809028_ne_15_1_20140720/m_3809028_ne_15_1_20140720.tif \
    --dataset dataset/UL-200 \
    --preview

# Headless режим
./build/dataset-match \
    --map-image dataset/urban/m_3809028_ne_15_1_20140720/m_3809028_ne_15_1_20140720.tif \
    --dataset dataset/UL-200 \
    --no-gui \
    --particle-count 300 \
    --particle-radius 800
```
