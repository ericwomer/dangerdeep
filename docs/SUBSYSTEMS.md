# Documentación de Subsistemas

## Índice

1. [event_manager](#event_manager)
2. [physics_system](#physics_system)
3. [time_freezer](#time_freezer)
4. [trail_manager](#trail_manager)
5. [visibility_manager](#visibility_manager)
6. [scoring_manager](#scoring_manager)
7. [ping_manager](#ping_manager)
8. [logbook](#logbook)
9. [network_manager](#network_manager)

---

## event_manager

### Propósito

Gestiona una cola de eventos del juego (input de usuario, acciones, señales del sistema). Desacopla la generación de eventos de su procesamiento.

### API

```cpp
class event_manager {
public:
    event_manager();
    
    // Agregar eventos
    void add_event(std::unique_ptr<event> e);
    void add_event(event* e);  // Legacy, toma ownership
    
    // Procesar eventos
    void evaluate_events(game& gm);
    
    // Gestión
    void clear_events();
    size_t event_count() const;
};
```

### Uso Típico

```cpp
event_manager events;

// Agregar evento
events.add_event(std::make_unique<my_event>(data));

// Procesar todos los eventos
events.evaluate_events(game_instance);

// Limpiar cola
events.clear_events();
```

### Testing

- **Archivo**: `src/test/event_manager_test.cpp`
- **Casos**: 13 test cases
- **Cobertura**: Constructor, agregar eventos, limpiar, iteradores

### Dependencias

- `event` (clase base abstracta)
- Mock events para testing (`event_stub.h`)

---

## physics_system

### Propósito

Sistema de física del juego. Detecta colisiones entre objetos marinos (barcos, submarinos, torpedos) y calcula respuestas físicas.

### API

```cpp
class physics_system {
public:
    physics_system();
    
    // Detección de colisiones
    void check_collisions(
        const std::vector<ship*>& ships,
        const std::vector<ship*>& contacts
    );
    
    // Respuesta física
    void collision_response(
        sea_object* obj1,
        sea_object* obj2,
        const vector3& collision_point
    );
};
```

### Uso Típico

```cpp
physics_system physics;

// En el game loop
physics.check_collisions(all_ships, detected_contacts);
```

### Algoritmo de Colisiones

1. **Broad Phase**: Bounding volumes (esferas/AABB)
2. **Narrow Phase**: BV-Tree traversal para geometría detallada
3. **Response**: Cálculo de impulsos y cambio de velocidad

### Testing

- **Archivo**: `src/test/physics_system_test.cpp`
- **Casos**: 7 test cases
- **Cobertura**: Construcción, nullptr safety, múltiples instancias
- **Stub**: `physics_system_stub.cpp` para aislar dependencias

### Dependencias

- `sea_object`, `ship` (objetos del juego)
- `bv_tree` (spatial partitioning)
- `vector3` (matemáticas)

---

## time_freezer

### Propósito

Controla el estado de pausa del juego. Permite "congelar" el tiempo del juego mientras la UI permanece responsiva.

### API

```cpp
class time_freezer {
public:
    time_freezer();
    
    // Control de pausa
    void freeze();
    void unfreeze();
    bool is_frozen() const;
    
    // Estado
    void get_state(unsigned& freezetime, unsigned& freezetime_start) const;
    
    // Procesamiento
    void process_freezetime(unsigned current_time);
    
    // Serialización
    void load(const xml_elem& parent);
    void save(xml_elem& parent) const;
};
```

### Uso Típico

```cpp
time_freezer time_control;

// Pausar juego
time_control.freeze();

// En game loop
time_control.process_freezetime(current_millis);

// Reanudar
time_control.unfreeze();

if (!time_control.is_frozen()) {
    // Actualizar simulación
}
```

### Estados

- **Normal**: `freeze_time == 0`
- **Congelado**: `freeze_time > 0`

### Testing

- **Archivo**: `src/test/time_freezer_test.cpp`
- **Casos**: 10 test cases
- **Cobertura**: Constructor, load, get_state, process, edge cases
- **Stub**: `time_freezer_stub.cpp` (evita dependencia de `system.cpp`)

---

## trail_manager

### Propósito

Gestiona las estelas visuales de barcos y torpedos. Controla el timing y frecuencia de registro de puntos de estela.

### API

```cpp
class trail_manager {
public:
    trail_manager();
    
    // Control de estelas
    bool should_record(unsigned current_time) const;
    void record_trail(unsigned current_time);
    
    // Configuración
    void set_last_trail_time(unsigned t);
    unsigned get_last_trail_time() const;
    unsigned get_trail_interval() const;
    
    // Serialización
    void load(const xml_elem& parent);
    void save(xml_elem& parent) const;
};
```

### Uso Típico

```cpp
trail_manager trails;

// En game loop
if (trails.should_record(current_time)) {
    create_trail_point(ship_position);
    trails.record_trail(current_time);
}
```

### Configuración

- **Intervalo por defecto**: 5000ms (5 segundos entre puntos)
- **Configurable** vía XML

### Testing

- **Archivo**: `src/test/trail_manager_test.cpp`
- **Casos**: 8 test cases
- **Cobertura**: Timing, should_record, intervalos, secuencias realistas

---

## visibility_manager

### Propósito

Calcula la distancia máxima de visibilidad basándose en condiciones atmosféricas y tiempo del día.

### API

```cpp
class visibility_manager {
public:
    visibility_manager();
    
    // Cálculo de visibilidad
    double compute(double brightness) const;
    
    // Configuración
    void set_max_distance(double d);
    double get_max_distance() const;
    
    // Serialización
    void load(const xml_elem& parent);
    void save(xml_elem& parent) const;
};
```

### Uso Típico

```cpp
visibility_manager visibility;

// Calcular visibilidad actual
double brightness = calculate_brightness();  // 0.0 - 1.0
double vis_range = visibility.compute(brightness);

// Usar en culling
if (distance_to_object < vis_range) {
    render(object);
}
```

### Fórmula

```
visibility_distance = max_distance * sqrt(brightness)
```

- **brightness = 0.0**: No se ve nada
- **brightness = 0.25**: 50% distancia
- **brightness = 1.0**: 100% distancia

### Testing

- **Archivo**: `src/test/visibility_manager_test.cpp`
- **Casos**: 6 test cases
- **Cobertura**: Diferentes brightness, max distance, edge cases, ciclo día/noche

---

## scoring_manager

### Propósito

Registra barcos hundidos durante la patrulla y calcula tonelaje total. Usado para scoring y estadísticas de misión.

### API

```cpp
class scoring_manager {
public:
    scoring_manager();
    
    // Registro de hundimientos
    void record_sunk_ship(const std::string& name, double tonnage);
    
    // Consultas
    size_t sunk_count() const;
    bool has_records() const;
    double total_tonnage() const;
    
    // Acceso a registros
    const std::list<ship_sunk_record>& get_records() const;
    
    // Gestión
    void clear();
    
    // Serialización
    void load(const xml_elem& parent);
    void save(xml_elem& parent) const;
};
```

### ship_sunk_record

```cpp
struct ship_sunk_record {
    std::string name;     // Nombre del barco
    double tonnage;       // Tonelaje
    unsigned timestamp;   // Tiempo del juego
};
```

### Uso Típico

```cpp
scoring_manager scoring;

// Registrar hundimiento
scoring.record_sunk_ship("SS Liberty", 7176.0);

// Consultar estadísticas
std::cout << "Barcos hundidos: " << scoring.sunk_count() << "\n";
std::cout << "Tonelaje total: " << scoring.total_tonnage() << "t\n";

// Limpiar al final de patrulla
scoring.clear();
```

### Testing

- **Archivo**: `src/test/scoring_manager_test.cpp`
- **Casos**: 8 test cases
- **Cobertura**: Registro, conteo, tonelaje, orden, clear, secuencias realistas

---

## ping_manager

### Propósito

Gestiona pings de sonar activo. Cada ping tiene posición, ángulo, y tiempo de expiración. Usado para detección acústica activa.

### API

```cpp
class ping_manager {
public:
    ping_manager();
    
    // Gestión de pings
    void add_ping(vector2 pos, angle bearing, unsigned duration_ms);
    size_t ping_count() const;
    bool has_pings() const;
    
    // Actualización
    void update(unsigned current_time);
    
    // Gestión
    void clear();
    
    // Acceso a pings
    const std::list<sonar_ping>& get_pings() const;
    
    // Serialización
    void load(const xml_elem& parent);
    void save(xml_elem& parent) const;
};
```

### sonar_ping

```cpp
struct sonar_ping {
    vector2 position;       // Posición en mapa
    angle bearing;          // Dirección
    unsigned expire_time;   // Tiempo de expiración
};
```

### Uso Típico

```cpp
ping_manager pings;

// Emitir ping
pings.add_ping(sub_position, sub_heading, 5000);  // 5 segundos

// En game loop
pings.update(current_time);  // Expira pings antiguos

// Renderizar pings activos
for (const auto& ping : pings.get_pings()) {
    draw_sonar_ping(ping.position, ping.bearing);
}
```

### Testing

- **Archivo**: `src/test/ping_manager_test.cpp`
- **Casos**: 7 test cases
- **Cobertura**: Agregar, expiración, orden, clear, secuencias realistas

---

## logbook

### Propósito

Bitácora del capitán. Registra eventos importantes de la patrulla (avistamientos, hundimientos, daños, etc).

### API

```cpp
class logbook {
public:
    logbook();
    
    // Agregar entradas
    void add_entry(const std::string& text);
    
    // Consultar entradas
    const std::string& get_entry(size_t index) const;
    size_t entry_count() const;
    
    // Iteradores
    using iterator = std::vector<std::string>::iterator;
    using const_iterator = std::vector<std::string>::const_iterator;
    
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    
    // Serialización
    void load(const xml_elem& parent);
    void save(xml_elem& parent) const;
};
```

### Uso Típico

```cpp
logbook log;

// Agregar entradas
log.add_entry("0830: Convoy avistado, rumbo 045, distancia 8000m");
log.add_entry("0845: Disparados 2 torpedos");
log.add_entry("0847: Impactos confirmados. Carguero hundido.");

// Mostrar en UI
for (size_t i = 0; i < log.entry_count(); ++i) {
    display_line(log.get_entry(i));
}

// Iterar
for (const auto& entry : log) {
    process(entry);
}
```

### Formato de Entradas

Las entradas típicamente incluyen:
- **Timestamp**: Hora del juego
- **Evento**: Descripción
- **Detalles**: Datos relevantes (distancias, rumbos, etc)

### Testing

- **Archivo**: `src/test/logbook_test.cpp`
- **Casos**: 9 test cases
- **Cobertura**: Agregar, get_entry, iteradores, caracteres especiales, muchas entradas

---

## network_manager

### Propósito

Gestiona comunicación cliente-servidor para modo multijugador. Maneja conexiones, serialización de estado del juego, y sincronización.

### API

```cpp
class network_manager {
public:
    network_manager();
    
    // Configuración
    void set_server_mode(bool is_server);
    bool is_server() const;
    
    // Conexión
    void connect(const std::string& host, int port);
    void disconnect();
    bool is_connected() const;
    
    // Comunicación
    void send_message(const network_message& msg);
    void receive_messages(std::vector<network_message>& out);
    
    // Sincronización
    void sync_game_state(const game& g);
    void apply_game_state(game& g);
};
```

### Uso Típico

**Servidor**:
```cpp
network_manager net;
net.set_server_mode(true);
net.listen(9876);

// Game loop
net.sync_game_state(game_instance);
net.receive_messages(client_inputs);
```

**Cliente**:
```cpp
network_manager net;
net.set_server_mode(false);
net.connect("192.168.1.100", 9876);

// Game loop
net.send_message(player_input);
net.apply_game_state(game_instance);
```

### Protocolo

- **TCP**: Mensajes confiables (chat, comandos)
- **UDP**: Estado del juego (posiciones, actualizaciones frecuentes)

### Testing

- **Estado**: Sin tests unitarios (complejidad de red)
- **Testing**: Manual, modo multijugador

### Dependencias

- Sockets BSD / Winsock
- Serialización binaria/XML

---

## Patrones Comunes

Todos los subsistemas siguen patrones consistentes:

### 1. Construcción Simple

```cpp
subsystem_manager();  // Constructor por defecto
```

### 2. API Clara

Métodos públicos bien nombrados que expresan intención.

### 3. Serialización

```cpp
void load(const xml_elem& parent);
void save(xml_elem& parent) const;
```

### 4. Estado Encapsulado

Variables privadas, acceso controlado via getters.

### 5. Testabilidad

Cada subsistema tiene su propio test file con casos comprensivos.

---

## Guía de Uso

### Integración en `game`

```cpp
class game {
private:
    // Declarar como miembros
    event_manager events_;
    physics_system physics_;
    time_freezer time_control_;
    // ... otros subsistemas
    
public:
    // Usar en game loop
    void display(double delta_time) {
        if (!time_control_.is_frozen()) {
            physics_.check_collisions(ships, contacts);
            events_.evaluate_events(*this);
            // ...
        }
    }
};
```

### Testing de Subsistemas

```cpp
TEST_CASE("subsystem - basic operation") {
    subsystem_manager mgr;
    
    // Setup
    mgr.configure(...);
    
    // Acción
    mgr.operation();
    
    // Verificación
    REQUIRE(mgr.get_state() == expected);
}
```

---

## Futuras Mejoras

### Candidatos para Extracción

1. **input_manager**: Unificar gestión de input
2. **save_manager**: Centralizar guardado/carga
3. **ui_manager**: Sistema de UI del juego
4. **sound_manager**: Audio y música

### Mejoras Arquitecturales

1. **Event Bus**: Comunicación desacoplada entre subsistemas
2. **Dependency Injection**: Constructor injection para testing
3. **Interfaces**: Abstracciones para facilitar mocking

---

## Referencias

- [ARCHITECTURE.md](ARCHITECTURE.md): Arquitectura general
- [REFACTORING.md](REFACTORING.md): Historia de refactorings
- Tests: `src/test/*_manager_test.cpp`

---

**Última actualización**: 2026-03-03
**Mantenedor**: Equipo de desarrollo de Danger from the Deep
