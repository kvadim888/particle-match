# docs/ -- Документація проєкту particle-match

Детальна документація кожної структурної одиниці кодової бази.

## Навігація

### Архітектура та загальне

| Документ | Опис |
|----------|------|
| [Architecture.md](Architecture.md) | Архітектура системи, потік даних, взаємодія компонентів |
| [Glossary.md](Glossary.md) | Глосарій термінів (KLD, HPRELU, affine, epsilon/delta тощо) |

### Ядро алгоритму (`localization/src/`)

| Документ | Клас | Опис |
|----------|------|------|
| [ParticleFastMatch.md](ParticleFastMatch.md) | `ParticleFastMatch` | Центральний клас -- фільтр частинок + FAsT-Match |
| [Particle.md](Particle.md) | `Particle` | Одна частинка: позиція, ймовірність, афінні конфігурації |
| [Particles.md](Particles.md) | `Particles` | Контейнер частинок: ресемплінг, нормалізація, зважена сума |
| [FastMatch.md](FastMatch.md) | `FAsTMatch` | Розширена обгортка FAsT-Match алгоритму |
| [ImageSample.md](ImageSample.md) | `ImageSample` | Семплювання зображень для швидкого обчислення кореляції |
| [AffineTransformation.md](AffineTransformation.md) | `AffineTransformation` | Обгортка афінної матриці з ідентифікатором частинки |
| [Utilities.md](Utilities.md) | `Utilities` | Допоміжні функції: кореляція, шум, геометрія, обробка зображень |
| [ConfigExpanderBase.md](ConfigExpanderBase.md) | `ConfigExpanderBase`, `GridConfigExpander` | Стратегія генерації та розширення конфігурацій |
| [ConfigVisualizer.md](ConfigVisualizer.md) | `ConfigVisualizer` | Візуалізація частинок та конфігурацій на карті |
| [FastMatcherThread.md](FastMatcherThread.md) | `FastMatcherThread` | Асинхронне виконання FAsT-Match у потоці |

### Бібліотека роботи з даними (`dataset_reader/`)

| Документ | Клас | Опис |
|----------|------|------|
| [MetadataEntry.md](MetadataEntry.md) | `MetadataEntry` | Структура одного кадру БПЛА (зображення, GPS, IMU) |
| [MetadataEntryReader.md](MetadataEntryReader.md) | `MetadataEntryReader` | Послідовний читач CSV-набору даних |
| [GeotiffMap.md](GeotiffMap.md) | `Map`, `GeotiffMap` | Карта з конвертацією GPS <-> пікселі через GeoTIFF/UTM |
| [Vector3d_Quaternion.md](Vector3d_Quaternion.md) | `Vector3d`, `Quaternion` | 3D вектор позиції та кватерніон орієнтації |

### Виконувані програми (`localization/exec/`)

| Документ | Файл | Опис |
|----------|------|------|
| [DatasetTest.md](DatasetTest.md) | `dataset-test.cpp` | Головна програма: CLI, запуск фільтра, формат результатів |
| [ParticleFilterWorkspace.md](ParticleFilterWorkspace.md) | `ParticleFilterWorkspace` | Менеджер життєвого циклу фільтра частинок |

### Зовнішні залежності

| Документ | Опис |
|----------|------|
| [FAsT-Match-Submodule.md](FAsT-Match-Submodule.md) | Git-субмодуль: MatchConfig, MatchNet, оригінальний FAsTMatch |
