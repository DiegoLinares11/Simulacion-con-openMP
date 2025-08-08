#  Simulación de Ecosistema con OpenMP

**Universidad del Valle de Guatemala**  
*Facultad de Ingeniería - Computación Paralela y Distribuida*  

---

## 🌱 Descripción
Simulación paralelizada con OpenMP de un ecosistema con tres especies:  
- **Plantas (P)**: Se reproducen en celdas adyacentes vacías.  
- **Herbívoros (H)**: Consumen plantas para sobrevivir.  
- **Carnívoros (C)**: Cazan herbívoros para alimentarse.  

---

## 🛠 Requisitos
| Componente       | Versión/Detalle          |
|------------------|--------------------------|
| Sistema Operativo| Linux (recomendado)      |
| Compilador       | GCC con soporte OpenMP   |
| Bibliotecas      | `stdio.h`, `stdlib.h`, `time.h`, `omp.h` |

---

## ⚙️ Compilación y Ejecución
```bash
# Compilar con OpenMP
gcc -fopenmp ecosistema.c -o ecosistema

# Ejecutar
./ecosistemas
```

### Paralelización con OpenMP
Estrategias de Paralelización Implementadas

Paralelización de Loops:
bash
pragma omp parallel for collapse(2)

Se paraleliza el procesamiento de la cuadrícula usando collapse(2) para manejar ambas dimensiones

Secciones Críticas:
bash
pragma omp critical

Protege las escrituras concurrentes al grid para evitar condiciones de carrera

Reductions:
bash
pragma omp parallel for collapse(2) reduction(+:p,h,c)

Para contar poblaciones de forma eficiente sin conflictos

### Manejo de Condiciones de Carrera

Doble Buffer: Se utiliza grid y next_grid para separar lectura y escritura
Procesamiento por Especies: Se procesan plantas, herbívoros y carnívoros en fases separadas
Secciones Críticas: Protegen las operaciones de movimiento y alimentación
