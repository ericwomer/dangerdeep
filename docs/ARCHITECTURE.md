# Arquitectura del Proyecto: Danger from the Deep

## Índice

1. [Visión General](#visión-general)
2. [Arquitectura Actual](#arquitectura-actual)
3. [Subsistemas Extraídos](#subsistemas-extraídos)
4. [Clase Game](#clase-game)
5. [Patrones de Diseño](#patrones-de-diseño)
6. [Flujo de Ejecución](#flujo-de-ejecución)
7. [Gestión de Memoria](#gestión-de-memoria)
8. [Dependencias](#dependencias)

---

## Visión General

**Danger from the Deep** es un simulador de submarinos de la Segunda Guerra Mundial escrito en C++20. El proyecto ha evolucionado desde una arquitectura monolítica hacia un diseño más modular basado en subsistemas.

### Estadísticas del Proyecto

- **Lenguaje**: C++20
- **Build System**: CMake 3.20+
- **Líneas de código**: ~150,000 LOC
- **Tests**: 101 tests unitarios (100% passing)
- **Cobertura**: En crecimiento (50+ módulos con tests)

### Objetivos Arquitecturales

1. **Modularidad**: Separar responsabilidades en subsistemas cohesivos
2. **Testabilidad**: Facilitar tests unitarios mediante inyección de dependencias
3. **Mantenibilidad**: Código autodocumentado y bien estructurado
4. **Performance**: Mantener 60+ FPS en hardware moderno

---

## Arquitectura Actual

### Visión de Alto Nivel

```
┌─────────────────────────────────────────────────────┐
│                   Aplicación                        │
│                    (main.cpp)                       │
└──────────────────────┬──────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────┐
│                  Clase Game                         │
│           (Coordinador Central)                     │
│  • Inicialización                                   │
│  • Game Loop Principal                              │
│  • Gestión de Estado                                │
└───────┬─────────────────────────────────────────────┘
        │
        │  Delega a ▼
        │
        ├─────────────────────────────────────────────┐
        │                                             │
        ▼                                             ▼
┌──────────────────┐                     ┌──────────────────┐
│   Subsistemas    │                     │   Subsistemas    │
│   Extraídos      │                     │   Legacy         │
│   (9 módulos)    │                     │   (en game.h)    │
└──────────────────┘                     └──────────────────┘
        │                                             │
        │                                             │
        ▼                                             ▼
┌──────────────────────────────────────────────────────────┐
│              Capa de Bajo Nivel                          │
│  • OpenGL/SDL2 (Rendering)                               │
│  • TinyXML (Serialización)                               │
│  • Sistema de Archivos                                   │
└──────────────────────────────────────────────────────────┘
```

### Estructura de Directorios

```
dangerdeep/
├── src/                    # Código fuente principal
│   ├── game.cpp/h          # Clase Game (coordinador central)
│   ├── *_manager.cpp/h     # Subsistemas extraídos
│   ├── *_system.cpp/h      # Sistemas de juego
│   ├── oglext/             # Extensiones OpenGL
│   ├── test/               # Tests unitarios (101 tests)
│   └── tools/              # Herramientas auxiliares
├── data/                   # Assets del juego
├── docs/                   # Documentación
│   ├── ARCHITECTURE.md     # Este archivo
│   ├── REFACTORING.md      # Historial de refactoring
│   └── SUBSYSTEMS.md       # Documentación de subsistemas
├── build/                  # Directorio de compilación (generado)
└── CMakeLists.txt          # Configuración de build
```

---

## Subsistemas Extraídos

El proyecto ha migrado hacia una arquitectura basada en subsistemas independientes. **9 subsistemas** han sido extraídos de la clase monolítica `game`:

### 1. **event_manager** - Gestión de Eventos
- **Responsabilidad**: Cola de eventos del juego (input, acciones, señales)
- **Estado**: ✅ Extraído y testeado
- **Archivo**: `event_manager.h/cpp`
- **Tests**: `event_manager_test.cpp` (13 casos)

### 2. **physics_system** - Sistema de Física
- **Responsabilidad**: Detección de colisiones, respuesta física
- **Estado**: ✅ Extraído y testeado
- **Archivo**: `physics_system.h/cpp`
- **Tests**: `physics_system_test.cpp` (7 casos)

### 3. **time_freezer** - Control de Tiempo
- **Responsabilidad**: Pausas del juego, control de tiempo
- **Estado**: ✅ Extraído y testeado
- **Archivo**: `time_freezer.h/cpp`
- **Tests**: `time_freezer_test.cpp` (10 casos)

### 4. **trail_manager** - Gestión de Estelas
- **Responsabilidad**: Estelas de barcos/torpedos, timing
- **Estado**: ✅ Extraído y testeado
- **Archivo**: `trail_manager.h/cpp`
- **Tests**: `trail_manager_test.cpp` (8 casos)

### 5. **visibility_manager** - Sistema de Visibilidad
- **Responsabilidad**: Cálculo de distancia de visibilidad (clima, hora)
- **Estado**: ✅ Extraído y testeado
- **Archivo**: `visibility_manager.h/cpp`
- **Tests**: `visibility_manager_test.cpp` (6 casos)

### 6. **scoring_manager** - Sistema de Puntuación
- **Responsabilidad**: Registro de barcos hundidos, tonelaje
- **Estado**: ✅ Extraído y testeado
- **Archivo**: `scoring_manager.h/cpp`
- **Tests**: `scoring_manager_test.cpp` (8 casos)

### 7. **ping_manager** - Gestión de Pings de Sonar
- **Responsabilidad**: Pings activos de sonar, expiración
- **Estado**: ✅ Extraído y testeado
- **Archivo**: `ping_manager.h/cpp`
- **Tests**: `ping_manager_test.cpp` (7 casos)

### 8. **logbook** - Bitácora del Capitán
- **Responsabilidad**: Registro de eventos de la patrulla
- **Estado**: ✅ Extraído y testeado
- **Archivo**: `logbook.h/cpp`
- **Tests**: `logbook_test.cpp` (9 casos)

### 9. **network_manager** - Red y Multijugador
- **Responsabilidad**: Comunicación cliente-servidor
- **Estado**: ✅ Extraído (sin tests por complejidad de red)
- **Archivo**: `network_manager.h/cpp`

### Patrón de Extracción

Todos los subsistemas siguen el mismo patrón:

```cpp
class subsystem_manager {
public:
    // Constructor simple
    subsystem_manager();
    
    // API pública clara
    void operation();
    
    // Getters para testing
    StateType get_state() const;
    
    // Serialización (si aplica)
    void load(const xml_elem& parent);
    void save(xml_elem& parent) const;
    
private:
    // Estado encapsulado
    InternalState state_;
};
```

**Beneficios**:
- ✅ Responsabilidad única
- ✅ Fácil de testear (aislado)
- ✅ Bajo acoplamiento
- ✅ API clara y documentada

---

## Clase Game

La clase `game` es el **coordinador central** del juego. Después de las extracciones, su rol se ha simplificado:

### Responsabilidades Actuales

1. **Inicialización**: Setup de subsistemas y recursos
2. **Game Loop**: Ciclo principal de actualización y rendering
3. **Gestión de Estado**: Transiciones entre estados del juego
4. **Coordinación**: Comunicación entre subsistemas

### Estado Actual

- **Líneas**: ~1936 LOC (antes: ~3500 LOC)
- **Reducción**: ~45% gracias a las extracciones
- **Subsistemas**: Instancias de los 9 managers extraídos

### Estructura Típica

```cpp
class game {
public:
    // Subsistemas extraídos
    std::unique_ptr<event_manager> events;
    std::unique_ptr<physics_system> physics;
    std::unique_ptr<time_freezer> time_control;
    // ... otros subsistemas
    
    // Game loop
    void display(double delta_time);
    void process_input(const SDL_Event& event);
    
private:
    // Estado del juego
    GameState current_state;
    // ... otras variables
};
```

### Candidatos para Futuras Extracciones

1. **input_manager**: Gestión de teclado/ratón
2. **save_manager**: Guardado/carga de partidas
3. **ui_manager**: Sistema de UI del juego
4. **sound_manager**: Audio y música
5. **camera_manager**: Control de cámara

---

## Patrones de Diseño

### 1. Singleton (Limitado)

**Uso**: Solo para recursos globales genuinos
```cpp
system& sys(); // Sistema operativo
```

**Nota**: Se evita en nuevos subsistemas (preferimos inyección de dependencias)

### 2. Manager/System

**Uso**: Subsistemas extraídos
- Encapsulan estado y comportamiento
- API pública clara
- Responsabilidad única

### 3. RAII (Resource Acquisition Is Initialization)

**Uso**: Gestión de recursos
```cpp
std::unique_ptr<T> resource;  // Auto-destrucción
std::vector<T> data;           // Gestión automática
```

### 4. Factory (Implícito)

**Uso**: Creación de objetos del juego
```cpp
std::unique_ptr<ship> ship::create(xml_elem& spec);
```

### 5. Observer (Event System)

**Uso**: Sistema de eventos
```cpp
event_manager.add_event(std::make_unique<my_event>());
```

---

## Flujo de Ejecución

### Inicialización

```
main()
  ├─> Inicializar SDL2/OpenGL
  ├─> Cargar configuración
  ├─> Crear instancia de game
  ├─> game.init()
  │     ├─> Inicializar subsistemas
  │     ├─> Cargar assets
  │     └─> Setup inicial
  └─> Entrar al game loop
```

### Game Loop Principal

```
while (running) {
    ├─> Procesar eventos (SDL_PollEvent)
    │     └─> game.process_input(event)
    ├─> Actualizar lógica
    │     └─> game.display(delta_time)
    │           ├─> time_freezer.process()
    │           ├─> physics_system.update()
    │           ├─> event_manager.evaluate_events()
    │           └─> ... otros subsistemas
    ├─> Renderizar frame
    │     └─> OpenGL draw calls
    └─> SDL_GL_SwapWindow()
}
```

### Shutdown

```
game destructor
  ├─> Guardar estado (si necesario)
  ├─> Destruir subsistemas (orden inverso)
  ├─> Liberar recursos OpenGL
  └─> Cerrar SDL2
```

---

## Gestión de Memoria

### Estrategia General

1. **Smart Pointers**: `std::unique_ptr` / `std::shared_ptr`
2. **RAII**: Contenedores STL (`std::vector`, `std::list`)
3. **Stack allocation**: Para objetos temporales
4. **Evitar**: Raw pointers para ownership

### Ejemplos

```cpp
// ✅ Bueno: Ownership claro
std::unique_ptr<ship> my_ship = std::make_unique<ship>();

// ✅ Bueno: Colección gestionada automáticamente
std::vector<torpedo> torpedoes;

// ❌ Evitar: Raw pointer sin ownership claro
ship* ptr = new ship();  // ¿Quién lo libera?

// ✅ Bueno: Referencias para no-ownership
void process(const ship& s);  // No toma ownership
```

### Tests y Stubs

Para testing, usamos stubs mínimos que evitan dependencias pesadas:
- `physics_system_stub.cpp`: Métodos vacíos
- `event_stub.h`: Eventos mock
- `time_freezer_stub.cpp`: Constructor + no-ops

---

## Dependencias

### Externas

| Librería | Versión | Uso |
|----------|---------|-----|
| **SDL2** | 2.0+ | Ventanas, input, OpenGL context |
| **OpenGL** | 2.1+ | Rendering 3D |
| **TinyXML** | - | Serialización XML |
| **Catch2** | 3.5.2 | Framework de testing |

### Internas

```
game (coordinador)
  ├─> event_manager
  ├─> physics_system
  ├─> time_freezer
  ├─> trail_manager
  ├─> visibility_manager
  ├─> scoring_manager
  ├─> ping_manager
  ├─> logbook
  └─> network_manager
```

**Principio**: Dependencias unidireccionales (game → subsistemas, nunca al revés)

---

## Próximos Pasos

### Arquitectura

1. Extraer más subsistemas de `game` (input, save, ui, sound)
2. Implementar Event Bus para comunicación entre subsistemas
3. Separar rendering en su propio subsistema

### Testing

1. Aumentar cobertura de tests (actualmente: 101 tests)
2. Tests de integración entre subsistemas
3. Performance benchmarks

### Documentación

1. Diagramas UML de subsistemas
2. API docs con Doxygen
3. Tutoriales para nuevos desarrolladores

---

## Referencias

- [REFACTORING.md](REFACTORING.md): Historial detallado de refactorings
- [SUBSYSTEMS.md](SUBSYSTEMS.md): Documentación detallada de cada subsistema
- [CONTRIBUTING.md](../CONTRIBUTING.md): Guía de contribución
- [README.md](../README.md): Información general del proyecto

---

**Última actualización**: 2026-03-03
**Mantenedor**: Equipo de desarrollo de Danger from the Deep
