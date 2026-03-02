# Guía de Estilo de Código - Danger from the Deep

Este documento define las convenciones de estilo para el código del proyecto Danger from the Deep.

## Principios Generales

1. **Consistencia**: Seguir el estilo existente en el archivo que estás modificando
2. **Claridad**: El código debe ser autoexplicativo; evitar comentarios obvios
3. **Moderno C++**: Usar características de C++17 donde sea apropiado
4. **RAII**: Gestión automática de recursos siempre que sea posible

## Convenciones de Naming

### Clases y Estructuras
- **Estilo**: `snake_case` (minúsculas con guiones bajos)
- **Ejemplos**:
  ```cpp
  class user_interface { };
  class sea_object { };
  struct height_data { };
  ```
- **Excepción**: Bibliotecas externas (e.g., TinyXML usa `PascalCase`)

### Variables Miembro
- **Subsistemas complejos**: Prefijo `my`
  ```cpp
  std::unique_ptr<weather_renderer> myweather;
  std::unique_ptr<terrain_manager> myterrain;
  std::unique_ptr<scene_environment> myenvironment;
  ```
- **Datos simples**: Sin prefijo
  ```cpp
  angle bearing;
  angle elevation;
  bool daymode;
  unsigned current_display;
  ```
- **Evitar**: `m_`, `_` u otros prefijos (no son el estilo del proyecto)

### Funciones y Métodos
- **Estilo**: `snake_case`
- **Ejemplos**:
  ```cpp
  double get_throttle() const;
  void set_time(double tm);
  void draw_infopanel() const;
  bool is_enabled() const;
  ```
- **Getters**: `get_X()` donde X es el nombre del dato
- **Setters**: `set_X(...)` donde X es el nombre del dato
- **Predicados booleanos**: `is_X()` o `has_X()` para mayor claridad

### Constantes y Enumeraciones
- **Estilo**: `UPPER_CASE` con guiones bajos
- **Ejemplos**:
  ```cpp
  const int MAX_TORPEDOES = 24;
  const double CONVOY_MAX_SIZE = 10000.0;
  
  enum throttle_status {
      THROTTLE_STOP = 0,
      THROTTLE_SLOW = 1,
      THROTTLE_HALF = 2,
      THROTTLE_FULL = 3
  };
  ```

### Archivos
- **Headers**: `.h` (no `.hpp`)
- **Implementación**: `.cpp`
- **Nombre del archivo = nombre de la clase principal**:
  - `weather_renderer.h` / `weather_renderer.cpp`
  - `scene_environment.h` / `scene_environment.cpp`
  - `user_interface.h` / `user_interface.cpp`

## Formato de Código

### Indentación
- **4 espacios** (no tabs)
- Configurar tu editor para convertir tabs a espacios

### Llaves
```cpp
// Clases: llave de apertura en nueva línea
class my_class {
  public:
    my_class();
};

// Funciones cortas: llave en misma línea (estilo K&R)
void short_function() {
    do_something();
}

// Funciones largas: llave en nueva línea para mayor claridad
void long_complex_function(
    int param1,
    double param2,
    const std::string &param3)
{
    // múltiples líneas de código...
}

// Condicionales y bucles: llave en misma línea
if (condition) {
    do_something();
} else {
    do_something_else();
}

for (auto &item : items) {
    process(item);
}
```

### Longitud de Líneas
- **Preferir < 100 caracteres** cuando sea razonable
- Dividir líneas largas en argumentos de función, expresiones complejas
```cpp
// Bien
auto result = calculate_complex_thing(
    first_parameter,
    second_parameter,
    third_parameter);

// Evitar líneas excesivamente largas
```

### Espacios en Blanco
```cpp
// Operadores binarios: espacios alrededor
int result = a + b * c;
double ratio = numerator / denominator;

// Paréntesis de funciones: sin espacio
void function(int arg);
function(42);

// Comas: espacio después
foo(a, b, c);

// Punteros y referencias: pegados al tipo
int* ptr;           // Bien
int *ptr;           // También aceptable (menos preferido)
const std::string& ref;  // Bien
```

## Comentarios y Documentación

### Documentación de Headers
Usar sintaxis Doxygen (`///`) para documentar clases y métodos públicos:

```cpp
/// \brief Manages weather rendering effects
///
/// This subsystem handles procedural generation and display
/// of animated weather effects (rain, snow) using OpenGL.
class weather_renderer {
  public:
    /// Draw weather effects for current time
    /// @param current_time - current game time for animation
    void draw(double current_time) const;
};
```

### Comentarios en Código
- **Comentar el "por qué", no el "qué"**:
  ```cpp
  // Mal: obvio
  i++;  // Increment i
  
  // Bien: explica la razón
  // Skip first frame to avoid initial position jump
  if (frame_count > 0) {
      update_position();
  }
  ```

- **Evitar comentarios redundantes**:
  ```cpp
  // NO hacer esto:
  void get_speed();    // Get the speed
  void set_speed();    // Set the speed
  int width;           // The width
  ```

### TODOs y FIXMEs
```cpp
// TODO: Implement collision detection optimization
// FIXME: Memory leak in texture loading
// NOTE: This assumes single-threaded context
```

## C++ Moderno (C++17)

### Smart Pointers
```cpp
// Preferir std::unique_ptr para ownership único
std::unique_ptr<model> mymodel;

// std::shared_ptr solo cuando realmente necesites ownership compartido
// (raro en este proyecto)

// Evitar raw pointers para ownership
Model* mymodel;  // NO (a menos que no sea owner)
```

### Const Correctness
```cpp
// Métodos que no modifican estado deben ser const
class ship {
  public:
    double get_speed() const;           // Bien
    throttle_status get_throttle() const;  // Bien
    
    void set_speed(double s);           // Bien (no const, modifica)
};

// Parámetros que no se modifican deben ser const ref
void process_data(const std::vector<int> &data);
void render(const vector3 &position) const;
```

### Auto
Usar `auto` para tipos complejos evidentes:
```cpp
// Bien: tipo obvio por contexto
auto mymodel = std::make_unique<model>("u_boat.obj");
auto it = my_map.find(key);

// Evitar: oculta el tipo de forma confusa
auto x = calculate();  // ¿Qué retorna calculate()?
```

### Range-Based For Loops
```cpp
// Preferir range-based loops cuando sea apropiado
for (const auto &ship : ships) {
    ship->update();
}

// En lugar de:
for (size_t i = 0; i < ships.size(); ++i) {
    ships[i]->update();
}
```

### Inicialización
```cpp
// Preferir inicialización uniforme con {}
int value{42};
std::vector<int> numbers{1, 2, 3, 4, 5};

// Pero usar () para constructores que podrían ser ambiguos
std::unique_ptr<model> mymodel(new model("file.obj"));
```

## Patrones de Diseño

### RAII (Resource Acquisition Is Initialization)
```cpp
// Bien: RAII automático
class texture_manager {
    std::unique_ptr<texture> mytex;
  public:
    texture_manager() : mytex(std::make_unique<texture>()) {}
    // Destructor automático libera mytex
};

// Evitar: gestión manual
class bad_manager {
    texture* mytex;
  public:
    bad_manager() : mytex(new texture()) {}
    ~bad_manager() { delete mytex; }  // Fácil olvidar, propenso a leaks
};
```

### Inyección de Dependencias
```cpp
// Bien: dependencias explícitas
class game {
    cfg &config;
    log &logger;
  public:
    game(cfg &c, log &l) : config(c), logger(l) {}
};

// Evitar: singletons ocultos (dificulta testing)
class bad_game {
  public:
    bad_game() {
        auto &config = cfg::instance();  // Acoplamiento oculto
    }
};
```

### Subsistemas
Al extraer responsabilidades, crear subsistemas autocontenidos:
```cpp
// Bien: subsistema con interfaz clara
class weather_renderer {
  public:
    weather_renderer();
    ~weather_renderer();
    
    void draw(double current_time) const;
    bool is_enabled() const;
    
  private:
    // Detalles de implementación ocultos
    ptrvector<texture> raintex;
    void init_rain();
};
```

## Errores Comunes a Evitar

1. **No usar const en getters**
   ```cpp
   // Mal
   double get_speed() { return speed; }
   
   // Bien
   double get_speed() const { return speed; }
   ```

2. **Gestión manual de memoria**
   ```cpp
   // Mal
   Model* m = new Model();
   // ... usar m ...
   delete m;
   
   // Bien
   auto m = std::make_unique<Model>();
   // Liberación automática
   ```

3. **Includes innecesarios en headers**
   ```cpp
   // Mal (en .h)
   #include "heavy_dependency.h"
   
   // Bien (en .h)
   class heavy_dependency;  // Forward declaration
   
   // Bien (en .cpp)
   #include "heavy_dependency.h"
   ```

4. **Comentarios obvios**
   ```cpp
   // Mal
   int width;   // The width
   int height;  // The height
   
   // Bien (autoexplicativo)
   int texture_width;
   int texture_height;
   ```

## Referencias

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [Modern C++ Features](https://github.com/AnthonyCalandra/modern-cpp-features)
- Proyecto Danger from the Deep: `docs/REFACTORING.md`

## Changelog

- 2026-03-02: Guía inicial basada en convenciones existentes del proyecto
