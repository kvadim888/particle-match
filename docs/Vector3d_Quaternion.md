# Vector3d та Quaternion

**Файли:**
- `dataset_reader/include/fastmatch-dataset/Vector3d.hpp`, `dataset_reader/src/classes/Vector3d.cpp`
- `dataset_reader/include/fastmatch-dataset/Quaternion.hpp`, `dataset_reader/src/classes/Quaternion.cpp`

## Vector3d

### Призначення

3D вектор для представлення позиції (x, y, z). Використовується для ground truth pose та SVO (Semi-direct Visual Odometry) позиції.

### Поля

| Поле | Тип | Опис |
|------|-----|------|
| `x` | `double` | Координата X |
| `y` | `double` | Координата Y |
| `z` | `double` | Координата Z |

### Методи

- Геттери та сеттери для x, y, z
- `toEigen()` -- конвертація в `Eigen::Vector3d`
- Конструктор за замовчуванням та `Vector3d(x, y, z)`

---

## Quaternion

### Призначення

Кватерніон для представлення орієнтації (x, y, z, w). Використовується для IMU-орієнтації та ground truth орієнтації БПЛА.

### Поля

| Поле | Тип | Опис |
|------|-----|------|
| `x` | `double` | Компонент X |
| `y` | `double` | Компонент Y |
| `z` | `double` | Компонент Z |
| `w` | `double` | Скалярний компонент W |

### Методи

#### toRPY
```cpp
Vector3d toRPY() const;
```
Конвертує кватерніон в кути Ейлера (Roll, Pitch, Yaw) в радіанах:
- Roll (X) -- нахил вліво/вправо
- Pitch (Y) -- нахил вперед/назад
- **Yaw (Z)** -- напрямок (heading), найважливіший для локалізації

#### toRPYdegrees
```cpp
Vector3d toRPYdegrees() const;
```
Те саме, але в градусах.

#### toEigen
```cpp
Eigen::Quaterniond toEigen() const;
```
Конвертація в `Eigen::Quaterniond` для використання з матрицями обертання.

### Використання в алгоритмі

```cpp
// Отримання напрямку БПЛА з IMU
double direction = metadata.imuOrientation.toRPY().getZ(); // Yaw в радіанах
Particle::setDirection(direction);
```

Yaw з кватерніона IMU задає глобальний напрямок для всіх частинок -- це визначає орієнтацію порівнюваних ділянок карти.
