# SAGE Engine - Математический API

**Версия:** Alpha  
**Namespace:** SAGE::Math  
**Header:** `<SAGE/Math/Math.h>`

---

## Содержание

- [Vector2](#vector2) - 2D векторы
- [Vector3](#vector3) - 3D векторы
- [Vector4](#vector4) - 4D векторы
- [Matrix4](#matrix4) - 4x4 матрицы
- [Random](#random) - Генератор случайных чисел
- [Color](#color-api) - Работа с цветом
- [Time](#time-api) - Управление временем
- [Utilities](#utility-api) - Вспомогательные функции

---

## Math API (детально)

### Vector2

Двумерный вектор для позиций, скоростей, направлений.

#### Конструкторы

```cpp
Vector2();                          // (0, 0)
Vector2(float x, float y);          // (x, y)
Vector2(float value);               // (value, value)
explicit Vector2(const Float2& f);  // Из Float2
```

#### Статические методы

```cpp
static Vector2 Zero();              // (0, 0)
static Vector2 One();               // (1, 1)
static Vector2 UnitX();             // (1, 0)
static Vector2 UnitY();             // (0, 1)
static Vector2 Up();                // (0, -1) в экранных координатах
static Vector2 Down();              // (0, 1)
static Vector2 Left();              // (-1, 0)
static Vector2 Right();             // (1, 0)
```

#### Методы

```cpp
// Длина и нормализация
float Length() const;                    // Длина вектора
float LengthSquared() const;             // Длина² (быстрее)
Vector2 Normalized() const;              // Нормализованная копия
void Normalize();                        // Нормализовать на месте
bool IsNormalized() const;               // Проверка (length ≈ 1)

// Операции с векторами
float Dot(const Vector2& other) const;   // Скалярное произведение
float Cross(const Vector2& other) const; // "Z" компонента векторного произв.
Vector2 Project(const Vector2& onto) const;   // Проекция на вектор
Vector2 Reflect(const Vector2& normal) const; // Отражение от нормали

// Расстояние и углы
float Distance(const Vector2& other) const;      // Расстояние до точки
float DistanceSquared(const Vector2& other) const; // Квадрат расстояния
float Angle(const Vector2& other) const;         // Угол между векторами (рад)
float AngleDegrees(const Vector2& other) const;  // Угол в градусах

// Вращение и трансформация
Vector2 Rotate(float angleDegrees) const;     // Вращение вокруг (0,0)
Vector2 RotateAround(const Vector2& center,   // Вращение вокруг точки
                     float angleDegrees) const;
Vector2 Perpendicular() const;                // Перпендикулярный вектор
Vector2 PerpendicularCW() const;              // По часовой
Vector2 PerpendicularCCW() const;             // Против часовой

// Интерполяция
static Vector2 Lerp(const Vector2& a, const Vector2& b, float t);
static Vector2 SmoothStep(const Vector2& a, const Vector2& b, float t);
static Vector2 Slerp(const Vector2& a, const Vector2& b, float t);

// Ограничения
Vector2 Clamp(const Vector2& min, const Vector2& max) const;
Vector2 ClampLength(float maxLength) const;
Vector2 ClampLength(float minLength, float maxLength) const;

// Утилиты
Vector2 Abs() const;                    // Абсолютные значения
Vector2 Floor() const;                  // Округление вниз
Vector2 Ceil() const;                   // Округление вверх
Vector2 Round() const;                  // Округление к ближайшему
bool IsZero() const;                    // Проверка на ноль
bool Equals(const Vector2& other, float epsilon = 0.0001f) const;
```

#### Операторы

```cpp
Vector2 operator+(const Vector2& other) const;
Vector2 operator-(const Vector2& other) const;
Vector2 operator*(float scalar) const;
Vector2 operator/(float scalar) const;
Vector2 operator*(const Vector2& other) const;  // Покомпонентное умножение
Vector2 operator/(const Vector2& other) const;  // Покомпонентное деление
Vector2 operator-() const;                      // Унарный минус

Vector2& operator+=(const Vector2& other);
Vector2& operator-=(const Vector2& other);
Vector2& operator*=(float scalar);
Vector2& operator/=(float scalar);

bool operator==(const Vector2& other) const;
bool operator!=(const Vector2& other) const;

friend Vector2 operator*(float scalar, const Vector2& vec);
```

#### Примеры использования

```cpp
// Нормализация направления
Vector2 direction = (target - position).Normalized();

// Расстояние
float dist = playerPos.Distance(enemyPos);
if (dist < attackRange) {
    Attack();
}

// Отражение (например, мяч от стены)
Vector2 velocity(100, -50);
Vector2 wallNormal(0, 1);  // Стена сверху
velocity = velocity.Reflect(wallNormal);

// Интерполяция позиции
Vector2 newPos = Vector2::Lerp(startPos, endPos, t);

// Вращение вектора
Vector2 forward(1, 0);
Vector2 rotated = forward.Rotate(45.0f);  // 45 градусов

// Проекция (например, для скользящего движения)
Vector2 moveDir(1, 1);
Vector2 surfaceNormal(0, 1);
Vector2 slideDir = moveDir.Project(surfaceNormal.Perpendicular());

// Ограничение скорости
Vector2 velocity(150, 200);
float maxSpeed = 100.0f;
velocity = velocity.ClampLength(maxSpeed);

// Dot product для проверки направления
Vector2 toEnemy = (enemy - player).Normalized();
Vector2 playerForward(1, 0);
float dot = playerForward.Dot(toEnemy);
if (dot > 0.9f) {
    // Враг прямо впереди
}
```

---

### Vector3

Трёхмерный вектор (для 3D звука, частиц).

```cpp
struct Vector3 {
    float x, y, z;
    
    Vector3();
    Vector3(float x, float y, float z);
    Vector3(const Vector2& xy, float z);
    
    static Vector3 Zero();
    static Vector3 One();
    static Vector3 UnitX();
    static Vector3 UnitY();
    static Vector3 UnitZ();
    static Vector3 Up();
    static Vector3 Forward();
    static Vector3 Right();
    
    float Length() const;
    float LengthSquared() const;
    Vector3 Normalized() const;
    void Normalize();
    
    float Dot(const Vector3& other) const;
    Vector3 Cross(const Vector3& other) const;
    
    float Distance(const Vector3& other) const;
    float Angle(const Vector3& other) const;
    
    // Операторы аналогичны Vector2
};
```

---

### Vector4

Четырёхмерный вектор (RGBA цвета, однородные координаты).

```cpp
struct Vector4 {
    float x, y, z, w;
    
    Vector4();
    Vector4(float x, float y, float z, float w);
    Vector4(const Vector3& xyz, float w);
    Vector4(const Color& color);
    
    // Методы аналогичны Vector2/Vector3
    
    Color ToColor() const;
};
```

---

### Matrix4

4x4 матрица для трансформаций (column-major для OpenGL).

#### Конструкторы

```cpp
Matrix4();                              // Identity матрица
Matrix4(float diagonal);                // Диагональная матрица
Matrix4(const float* data);             // Из массива [16]
Matrix4(const std::array<float, 16>& data);
```

#### Статические методы создания

```cpp
// Единичная матрица
static Matrix4 Identity();

// Трансляция
static Matrix4 Translate(const Vector2& translation);
static Matrix4 Translate(float x, float y);
static Matrix4 Translate(const Vector3& translation);

// Вращение
static Matrix4 Rotate(float angleDegrees);              // Вокруг Z оси
static Matrix4 RotateX(float angleDegrees);
static Matrix4 RotateY(float angleDegrees);
static Matrix4 RotateZ(float angleDegrees);
static Matrix4 RotateAxis(const Vector3& axis, float angleDegrees);

// Масштабирование
static Matrix4 Scale(const Vector2& scale);
static Matrix4 Scale(float x, float y);
static Matrix4 Scale(float uniformScale);
static Matrix4 Scale(const Vector3& scale);

// Проекции
static Matrix4 Ortho(float left, float right, float bottom, float top);
static Matrix4 Ortho(float left, float right, float bottom, float top,
                     float near, float far);
static Matrix4 Perspective(float fov, float aspect, float near, float far);

// View матрицы
static Matrix4 LookAt(const Vector3& eye, const Vector3& center,
                     const Vector3& up);
static Matrix4 LookAt2D(const Vector2& position, float rotation, float zoom);
```

#### Методы

```cpp
// Операции с матрицами
Matrix4 Multiply(const Matrix4& other) const;
Matrix4 operator*(const Matrix4& other) const;
Matrix4& operator*=(const Matrix4& other);

// Трансформация векторов
Vector2 Apply(const Vector2& point) const;       // 2D точка
Vector3 Apply(const Vector3& point) const;       // 3D точка
Vector4 Apply(const Vector4& point) const;       // 4D вектор
Vector3 ApplyDirection(const Vector3& dir) const; // Только вращение/масштаб

// Обратная и транспонированная
Matrix4 Inverse() const;
Matrix4 Transpose() const;
bool TryInverse(Matrix4& out) const;             // Безопасная версия

// Извлечение компонентов
Vector3 GetTranslation() const;
Vector3 GetScale() const;
float GetRotation2D() const;                     // Для 2D (угол в градусах)

// Разложение
void Decompose(Vector3& translation, Vector3& rotation, Vector3& scale) const;

// Доступ к данным
const float* Data() const;                       // Для OpenGL
float* Data();
float& operator[](size_t index);
const float& operator[](size_t index) const;
float& At(size_t row, size_t col);
float At(size_t row, size_t col) const;

// Утилиты
float Determinant() const;
bool IsIdentity() const;
bool Equals(const Matrix4& other, float epsilon = 0.0001f) const;
```

#### Примеры использования

```cpp
// Трансформация спрайта
Matrix4 transform = Matrix4::Translate(position);
transform *= Matrix4::Rotate(rotation);
transform *= Matrix4::Scale(scale);

// Camera view matrix
Camera2D camera;
Matrix4 view = Matrix4::LookAt2D(camera.GetPosition(), 
                                 camera.GetRotation(),
                                 camera.GetZoom());

// Projection matrix
Matrix4 projection = Matrix4::Ortho(0, width, height, 0);

// View-Projection
Matrix4 vp = projection * view;

// Трансформация точки
Vector2 worldPos(100, 200);
Vector2 screenPos = vp.Apply(worldPos);

// Обратная трансформация
Matrix4 invVP = vp.Inverse();
Vector2 worldPosBack = invVP.Apply(screenPos);

// Комбинированная трансформация
Matrix4 model = Matrix4::Identity();
model = Matrix4::Translate(100, 200) * 
        Matrix4::Rotate(45) * 
        Matrix4::Scale(2.0f);

// Извлечение компонентов
Vector3 translation = model.GetTranslation();
Vector3 scale = model.GetScale();
float rotation = model.GetRotation2D();
```

---

### Random

Генератор псевдослучайных чисел (PCG32).

#### Доступ

```cpp
namespace Math {
    class Random {
    public:
        static Random& Global();        // Глобальный генератор
        
        Random();                       // Новый генератор
        explicit Random(uint32_t seed); // С заданным seed
    };
}
```

#### Методы

```cpp
// Seed
void SetSeed(uint32_t seed);
uint32_t GetSeed() const;

// Целые числа
int NextInt();                              // Любое int
int NextInt(int max);                       // [0, max)
int NextInt(int min, int max);              // [min, max]
uint32_t NextUInt();                        // Любое uint32_t
uint32_t NextUInt(uint32_t max);            // [0, max)

// Вещественные числа
float NextFloat();                          // [0, 1)
float NextRange(float min, float max);      // [min, max)
double NextDouble();                        // [0, 1)
double NextDouble(double min, double max);  // [min, max)

// Булевы значения
bool NextBool();                            // 50/50
bool NextBool(float probability);           // true с вероятностью probability

// Векторы
Vector2 NextUnitVector();                   // Единичный вектор
Vector2 NextInCircle(float radius);         // Точка в круге
Vector2 NextOnCircle(float radius);         // Точка на окружности
Vector2 NextInRect(const Vector2& min, const Vector2& max);

Vector3 NextUnitVector3();
Vector3 NextInSphere(float radius);
Vector3 NextOnSphere(float radius);

// Цвета
Color NextColor();                          // Случайный RGB (alpha = 255)
Color NextColor(uint8_t alpha);             // С заданным alpha
Color NextColorHSV(float hMin, float hMax,  // HSV диапазон
                  float sMin, float sMax,
                  float vMin, float vMax);

// Выбор из коллекции
template<typename T>
const T& Choose(const std::vector<T>& items);

template<typename T>
T& Choose(std::vector<T>& items);

template<typename T, size_t N>
const T& Choose(const std::array<T, N>& items);

// Перемешивание
template<typename T>
void Shuffle(std::vector<T>& items);

// Распределения
float NextGaussian(float mean = 0.0f, float stddev = 1.0f);  // Нормальное
float NextExponential(float lambda = 1.0f);                   // Экспоненциальное
```

#### Примеры использования

```cpp
auto& rng = Math::Random::Global();

// Установка seed для воспроизводимости
rng.SetSeed(12345);

// Случайное целое
int dice = rng.NextInt(1, 6);           // 1-6 (кубик)
int randomIndex = rng.NextInt(items.size());

// Случайное вещественное
float damage = rng.NextRange(10.0f, 20.0f);
float critChance = rng.NextFloat();     // 0-1

// Вероятность
if (rng.NextBool(0.1f)) {
    // 10% шанс критического удара
    damage *= 2.0f;
}

// Случайное направление
Vector2 direction = rng.NextUnitVector();
velocity = direction * speed;

// Спавн в круге
Vector2 spawnPos = playerPos + rng.NextInCircle(100.0f);

// Случайный цвет
Color randomColor = rng.NextColor();

// Выбор случайного элемента
std::vector<std::string> loot = {"sword", "shield", "potion"};
std::string drop = rng.Choose(loot);

// Перемешивание
std::vector<int> deck = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
rng.Shuffle(deck);

// Нормальное распределение (для естественной вариации)
float height = rng.NextGaussian(170.0f, 10.0f);  // Среднее 170, отклонение 10
```

---

## Color API

### Структура Color

```cpp
struct Color {
    uint8_t r, g, b, a;
    
    // Конструкторы
    Color();                                    // Чёрный (0, 0, 0, 255)
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    explicit Color(uint32_t hex);               // 0xRRGGBBAA
    explicit Color(const Vector3& rgb);         // RGB [0-1]
    explicit Color(const Vector4& rgba);        // RGBA [0-1]
    
    // Предустановленные цвета
    static Color White();           // (255, 255, 255, 255)
    static Color Black();           // (0, 0, 0, 255)
    static Color Red();             // (255, 0, 0, 255)
    static Color Green();           // (0, 255, 0, 255)
    static Color Blue();            // (0, 0, 255, 255)
    static Color Yellow();          // (255, 255, 0, 255)
    static Color Cyan();            // (0, 255, 255, 255)
    static Color Magenta();         // (255, 0, 255, 255)
    static Color Transparent();     // (0, 0, 0, 0)
    static Color Gray(uint8_t value = 128);
    
    // Конверсия
    static Color FromHex(uint32_t hex);         // 0xRRGGBBAA
    static Color FromHex(const std::string& hex); // "#RRGGBB" или "#RRGGBBAA"
    static Color FromHSV(float h, float s, float v, float a = 1.0f);
    static Color FromHSL(float h, float s, float l, float a = 1.0f);
    
    uint32_t ToHex() const;                     // 0xRRGGBBAA
    std::string ToHexString() const;            // "#RRGGBBAA"
    void ToHSV(float& h, float& s, float& v) const;
    void ToHSL(float& h, float& s, float& l) const;
    
    Vector3 ToFloat3() const;                   // RGB [0-1]
    Vector4 ToFloat4() const;                   // RGBA [0-1]
    
    // Операции
    Color WithAlpha(uint8_t alpha) const;
    Color WithRed(uint8_t red) const;
    Color WithGreen(uint8_t green) const;
    Color WithBlue(uint8_t blue) const;
    
    Color Lerp(const Color& other, float t) const;
    Color Multiply(const Color& other) const;   // Покомпонентное
    Color Add(const Color& other) const;
    
    float GetLuminance() const;                 // Яркость [0-1]
    Color Invert() const;                       // Инверсия RGB
    Color Grayscale() const;
    
    // Операторы
    bool operator==(const Color& other) const;
    bool operator!=(const Color& other) const;
    Color operator*(float scalar) const;
    Color operator*(const Color& other) const;
};
```

#### Примеры

```cpp
// Создание цветов
Color red = Color::Red();
Color customColor(128, 64, 32, 255);
Color fromHex = Color::FromHex(0xFF5733FF);
Color fromString = Color::FromHex("#FF5733");

// HSV
Color rainbow = Color::FromHSV(hue, 1.0f, 1.0f);  // hue 0-360

// Операции
Color tinted = sprite.tint.Multiply(Color::Red());  // Красный тинт
Color faded = color.WithAlpha(128);                 // Полупрозрачный

// Интерполяция
Color lerped = Color::Lerp(Color::Red(), Color::Blue(), 0.5f);  // Фиолетовый

// Яркость
float brightness = color.GetLuminance();
if (brightness < 0.5f) {
    // Тёмный цвет, использовать белый текст
}

// Инверсия
Color inverted = color.Invert();

// Серый
Color gray = color.Grayscale();
```

---

## Time API

### Time класс

```cpp
namespace SAGE {
    class Time {
    public:
        static float GetDeltaTime();          // Время с прошлого кадра (сек)
        static float GetTime();               // Время с начала игры (сек)
        static float GetUnscaledDeltaTime();  // Без учёта timeScale
        static float GetUnscaledTime();
        
        static uint64_t GetFrameCount();      // Номер кадра
        static float GetFPS();                // FPS
        static float GetAverageFPS();         // Средний FPS
        
        static void SetTimeScale(float scale); // Множитель времени
        static float GetTimeScale();
        
        static float GetFixedDeltaTime();     // Фиксированный шаг физики
        static void SetFixedDeltaTime(float dt);
        
        static double GetRealTime();          // Системное время (секунды)
    };
}
```

#### Примеры

```cpp
// В Update
void Update() {
    float dt = Time::GetDeltaTime();
    position += velocity * dt;
    
    // Slow motion
    if (input.IsKeyPressed(GLFW_KEY_T)) {
        Time::SetTimeScale(0.5f);  // 2x медленнее
    }
}

// FPS counter
static float fpsTimer = 0;
fpsTimer += Time::GetDeltaTime();
if (fpsTimer >= 1.0f) {
    float fps = Time::GetFPS();
    SAGE_INFO("FPS", "{}", fps);
    fpsTimer = 0;
}

// Таймер
static float timer = 5.0f;
timer -= Time::GetDeltaTime();
if (timer <= 0) {
    // Событие
    timer = 5.0f;  // Перезарядка
}
```

---

## Utility API

### Filesystem

```cpp
namespace SAGE::Utils {
    class Filesystem {
    public:
        static bool FileExists(const std::string& path);
        static bool DirectoryExists(const std::string& path);
        
        static std::string ReadFile(const std::string& path);
        static std::vector<uint8_t> ReadBinaryFile(const std::string& path);
        static bool WriteFile(const std::string& path, const std::string& content);
        static bool WriteBinaryFile(const std::string& path, 
                                   const std::vector<uint8_t>& data);
        
        static std::vector<std::string> ListFiles(const std::string& directory,
                                                  const std::string& extension = "");
        static std::vector<std::string> ListDirectories(const std::string& directory);
        
        static bool CreateDirectory(const std::string& path);
        static bool DeleteFile(const std::string& path);
        static bool DeleteDirectory(const std::string& path);
        
        static std::string GetExtension(const std::string& path);
        static std::string GetFilename(const std::string& path);
        static std::string GetDirectory(const std::string& path);
        static std::string GetAbsolutePath(const std::string& path);
        
        static std::string JoinPath(const std::string& a, const std::string& b);
        static std::string NormalizePath(const std::string& path);
    };
}
```

### String utilities

```cpp
namespace SAGE::Utils {
    class String {
    public:
        static std::string ToLower(const std::string& str);
        static std::string ToUpper(const std::string& str);
        static std::string Trim(const std::string& str);
        static std::string TrimLeft(const std::string& str);
        static std::string TrimRight(const std::string& str);
        
        static std::vector<std::string> Split(const std::string& str, char delimiter);
        static std::string Join(const std::vector<std::string>& parts, 
                               const std::string& separator);
        
        static bool StartsWith(const std::string& str, const std::string& prefix);
        static bool EndsWith(const std::string& str, const std::string& suffix);
        static bool Contains(const std::string& str, const std::string& substr);
        
        static std::string Replace(const std::string& str, 
                                  const std::string& from,
                                  const std::string& to);
        static std::string ReplaceAll(const std::string& str,
                                     const std::string& from,
                                     const std::string& to);
        
        static std::string Format(const char* format, ...);
        
        // UTF-8
        static size_t UTF8Length(const std::string& str);
        static std::string UTF8Substring(const std::string& str, 
                                        size_t start, size_t length);
    };
}
```

---

Продолжение документации охватывает все математические типы, утилиты и вспомогательные API. Для полного справочника см. также SYSTEM_REFERENCE.md и COMPONENT_REFERENCE.md.
