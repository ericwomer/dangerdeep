# Generación de tiles de terreno (batimetría)

El juego usa un mapa de elevación/batimetría global dividido en tiles comprimidos (`.bz2`) para renderizar el fondo marino. Estos tiles **no están incluidos** en el repositorio por su tamaño.

## Comportamiento cuando faltan tiles

Si un tile no existe en `data/maps/terrain/`, el juego usa profundidad `-200` m (agua profunda) para esa región. Funciona correctamente, pero en el log aparecerán avisos:

```
Cannot open file: .../data/maps/terrain/5376_10496.bz2
```

Son advertencias esperables, no errores fatales.

## Generar tiles con map_precompute

La herramienta `map_precompute` convierte un archivo binario de elevaciones (formato ETOPO1) en tiles `.bz2`.

### Requisitos

- **Datos ETOPO1**: modelo global de relieve (batimetría + topografía), 1 minuto de arco.
- **Formato esperado**: binario, valores float (4 bytes), orden fila-mayor, sin cabecera.
- **Resolución por defecto**: 21600 columnas × 10800 filas.

### Obtener ETOPO1

- NOAA NGDC: <https://www.ngdc.noaa.gov/mgg/global/relief/ETOPO1/>
- Versiones: Ice Surface (superficie de hielo) o Bedrock (roca base).
- Binario f4: ej. `etopo1_ice_g_f4.zip` o `etopo1_bed_g_f4.zip`.

Nota: el grid oficial es 21601×10801; puede hace falta ajustar o recortar para 21600×10800 según lo que espere la herramienta.

### Compilar map_precompute

La herramienta está en `src/tools/map_precompute.cpp`. Actualmente se compila con SCons. Si usás CMake, hay que añadir un target para este ejecutable.

### Uso básico

```bash
# Generar todos los tiles
./map_precompute --infile /ruta/a/etopo1.bin --outdir data/maps/terrain/

# Parámetros que coinciden con terrain.xml:
./map_precompute --infile etopo1.bin --outdir data/maps/terrain/ \
    --mapsize 21600*10800 --tile_size 256
```

### Opciones

| Opción | Descripción | Por defecto |
|--------|-------------|-------------|
| `--infile <file>` | Archivo binario de entrada | — |
| `--outdir <dir>` | Directorio de salida para los `.bz2` | — |
| `--mapsize X*Y` | Dimensiones del mapa | 21600*10800 |
| `--tile_size N` | Tamaño de cada tile (potencia de 2) | 512 |
| `--clip X1,Y1 X2,Y2` | Solo generar una región recortada | — |

El archivo `data/maps/terrain/terrain.xml` define `tile_size` 256; usá `--tile_size 256` para que coincida.

### Región recortada

Para probar sin generar todo el mapa:

```bash
# Solo tiles de una zona (coords deben ser múltiplos de tile_size)
./map_precompute --infile etopo1.bin --outdir data/maps/terrain/ \
    --tile_size 256 --clip 5120,9728 6144,11520
```

---

*Pendiente: integrar `map_precompute` en el sistema de build CMake y validar el formato exacto de ETOPO1 con la herramienta.*
