#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define SIZE 20
#define TICKS 20
//Parametros del pdf
#define INITIAL_PLANTS 150
#define INITIAL_HERBIVORES 60
#define INITIAL_CARNIVORES 15
#define PLANT_REPRODUCTION_RATE 20 // Porcentaje de probabilidad de reproducción de plantas.
#define ANIMAL_REPRODUCTION_RATE 100 //SI pueden reproducirse, se reproduce siempre que tenga energía suficiente.
#define HERB_ENERGY_GAIN 1
#define CARN_ENERGY_GAIN 2
#define MAX_NO_FOOD_TICKS 3 // Ticks sin comida antes de morir
#define REPRODUCTION_ENERGY_THRESHOLD 2 // Energía mínima para reproducirse
#define MOVEMENT_PROBABILITY 90 // Porcentaje de probabilidad de movimiento aleatorio para que no se queden estancados si no encuentra comida.
#define PLANT_MAX_AGE 15  

typedef enum { EMPTY, PLANT, HERBIVORE, CARNIVORE } Species;

typedef struct {
    Species type;
    int energy;
    int age;
    int ticks_no_food;
} Cell;

Cell grid[SIZE][SIZE];
Cell next_grid[SIZE][SIZE];

void initialize();
void simulate();
void update_plant(int i, int j);
void update_herbivore(int i, int j);
void update_carnivore(int i, int j);
void print_state(int tick);
void move_animal(int i, int j, int ni, int nj, Cell animal);
void reproduce_animal(int i, int j, Species species);
int count_empty_adjacent(int i, int j);

int main() {
    srand(time(NULL));
    initialize();
    print_state(0);
    simulate();
    return 0;
}

void initialize() {
    // Inicializar grid vacío
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = (Cell){EMPTY, 0, 0, 0};
        }
    }

    // Posiciones aleatorias sin repetición
    int total = SIZE * SIZE;
    int* indices = malloc(total * sizeof(int));
    for (int i = 0; i < total; i++) indices[i] = i;
    
    // Barajar indices
    for (int i = total - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    // Asignar plantas
    #pragma omp parallel for
    for (int p = 0; p < INITIAL_PLANTS; p++) {
        int idx = indices[p];
        int i = idx / SIZE;
        int j = idx % SIZE;
        grid[i][j] = (Cell){PLANT, 1, 0, 0};
    }

    // Asignar herbívoros
    #pragma omp parallel for
    for (int h = 0; h < INITIAL_HERBIVORES; h++) {
        int idx = indices[INITIAL_PLANTS + h];
        int i = idx / SIZE;
        int j = idx % SIZE;
        grid[i][j] = (Cell){HERBIVORE, 2, 0, 0};
    }

    // Asignar carnívoros
    #pragma omp parallel for
    for (int c = 0; c < INITIAL_CARNIVORES; c++) {
        int idx = indices[INITIAL_PLANTS + INITIAL_HERBIVORES + c];
        int i = idx / SIZE;
        int j = idx % SIZE;
        grid[i][j] = (Cell){CARNIVORE, 2, 0, 0};
    }

    free(indices);
}

void simulate() {
    for (int t = 1; t <= TICKS; t++) {
        // Preparar next_grid
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                next_grid[i][j] = grid[i][j];
            }
        }

        //  Carnívoros primero
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (grid[i][j].type == CARNIVORE) {
                    update_carnivore(i, j);
                }
            }
        }

        #pragma omp parallel for collapse(2)
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (grid[i][j].type == HERBIVORE) {
                    update_herbivore(i, j);
                }
            }
        }

        #pragma omp parallel for collapse(2)
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (grid[i][j].type == PLANT) {
                    update_plant(i, j);
                }
            }
        }

        // Copiar next_grid a grid
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                grid[i][j] = next_grid[i][j];
            }
        }

        print_state(t);
    }
}

void update_plant(int i, int j) {
    // Verificar si la planta aún existe
    if (next_grid[i][j].type != PLANT) return;
    
    // 1. Envejecimiento
    next_grid[i][j].age++;
    
    // 2. Muerte por vejez
    if (next_grid[i][j].age > PLANT_MAX_AGE) {
        next_grid[i][j] = (Cell){EMPTY, 0, 0, 0};
        return;
    }
    
    // 3. Muerte por falta de espacio (rodeada completamente)
    if (count_empty_adjacent(i, j) == 0) {
        next_grid[i][j] = (Cell){EMPTY, 0, 0, 0};
        return;
    }
    
    // 4. Reproducción
    if (rand() % 100 < PLANT_REPRODUCTION_RATE) {
        int dirs[8][2] = {{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1},{1,1}};
        int reproduced = 0;
        
        for (int d = 0; d < 8 && !reproduced; d++) {
            int ni = i + dirs[d][0], nj = j + dirs[d][1];
            if (ni >= 0 && ni < SIZE && nj >= 0 && nj < SIZE) {
                #pragma omp critical
                if (next_grid[ni][nj].type == EMPTY) {
                    next_grid[ni][nj] = (Cell){PLANT, 1, 0, 0};
                    reproduced = 1;
                }
            }
        }
    }
}

int count_empty_adjacent(int i, int j) {
    int count = 0;
    int dirs[8][2] = {{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1},{1,1}};
    
    for (int d = 0; d < 8; d++) {
        int ni = i + dirs[d][0], nj = j + dirs[d][1];
        if (ni >= 0 && ni < SIZE && nj >= 0 && nj < SIZE) {
            if (next_grid[ni][nj].type == EMPTY) {
                count++;
            }
        }
    }
    return count;
}

void update_herbivore(int i, int j) {
    if (next_grid[i][j].type != HERBIVORE) return;
    
    Cell current = grid[i][j];
    int moved = 0;
    int dirs[8][2] = {{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1},{1,1}};

    // 1. Alimentación
    for (int d = 0; d < 8 && !moved; d++) {
        int ni = i + dirs[d][0], nj = j + dirs[d][1];
        if (ni >= 0 && ni < SIZE && nj >= 0 && nj < SIZE) {
            if (grid[ni][nj].type == PLANT) {
                #pragma omp critical
                {
                    if (next_grid[ni][nj].type == PLANT) {
                        move_animal(i, j, ni, nj, current);
                        next_grid[ni][nj].energy += HERB_ENERGY_GAIN;
                        moved = 1;
                    }
                }
            }
        }
    }

    // 2. Movimiento aleatorio SI NO COMIO
    if (!moved && (rand() % 100 < MOVEMENT_PROBABILITY)) {
        for (int d = 0; d < 8 && !moved; d++) {
            int ni = i + dirs[d][0], nj = j + dirs[d][1];
            if (ni >= 0 && ni < SIZE && nj >= 0 && nj < SIZE) {
                #pragma omp critical
                if (next_grid[ni][nj].type == EMPTY) {
                    move_animal(i, j, ni, nj, current);
                    moved = 1;
                }
            }
        }
    }

    // 3. Actualizar estado si no se movió
    if (!moved) {
        next_grid[i][j].ticks_no_food++;
        next_grid[i][j].age++;
        next_grid[i][j].energy--;

        if (next_grid[i][j].ticks_no_food >= MAX_NO_FOOD_TICKS || next_grid[i][j].energy <= 0) {
            next_grid[i][j] = (Cell){EMPTY, 0, 0, 0};
        } 
        else if (next_grid[i][j].energy >= REPRODUCTION_ENERGY_THRESHOLD) {
            reproduce_animal(i, j, HERBIVORE);
        }
    }
}

void update_carnivore(int i, int j) {
    if (next_grid[i][j].type != CARNIVORE) return;
    
    Cell current = grid[i][j];
    int moved = 0;
    int dirs[8][2] = {{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1},{1,1}};

    // 1. Caza
    for (int d = 0; d < 8 && !moved; d++) {
        int ni = i + dirs[d][0], nj = j + dirs[d][1];
        if (ni >= 0 && ni < SIZE && nj >= 0 && nj < SIZE) {
            if (grid[ni][nj].type == HERBIVORE) {
                #pragma omp critical
                {
                    if (next_grid[ni][nj].type == HERBIVORE) {
                        move_animal(i, j, ni, nj, current);
                        next_grid[ni][nj].energy += CARN_ENERGY_GAIN;
                        moved = 1;
                    }
                }
            }
        }
    }

    // 2. Movimiento aleatorio
    if (!moved && (rand() % 100 < MOVEMENT_PROBABILITY)) {
        for (int d = 0; d < 8 && !moved; d++) {
            int ni = i + dirs[d][0], nj = j + dirs[d][1];
            if (ni >= 0 && ni < SIZE && nj >= 0 && nj < SIZE) {
                #pragma omp critical
                if (next_grid[ni][nj].type == EMPTY) {
                    move_animal(i, j, ni, nj, current);
                    moved = 1;
                }
            }
        }
    }

    // 3. Actualizar estado si no se movió
    if (!moved) {
        next_grid[i][j].ticks_no_food++;
        next_grid[i][j].age++;
        next_grid[i][j].energy--;

        if (next_grid[i][j].ticks_no_food >= MAX_NO_FOOD_TICKS || next_grid[i][j].energy <= 0) {
            next_grid[i][j] = (Cell){EMPTY, 0, 0, 0};
        } 
        else if (next_grid[i][j].energy >= REPRODUCTION_ENERGY_THRESHOLD) {
            reproduce_animal(i, j, CARNIVORE);
        }
    }
}

void move_animal(int i, int j, int ni, int nj, Cell animal) {
    animal.ticks_no_food = 0;
    next_grid[ni][nj] = animal;
    next_grid[i][j] = (Cell){EMPTY, 0, 0, 0};
}

void reproduce_animal(int i, int j, Species species) {
    if (rand() % 100 < ANIMAL_REPRODUCTION_RATE) {
        int dirs[8][2] = {{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1},{1,1}};
        int reproduced = 0;
        
        for (int d = 0; d < 8 && !reproduced; d++) {
            int ni = i + dirs[d][0], nj = j + dirs[d][1];
            if (ni >= 0 && ni < SIZE && nj >= 0 && nj < SIZE) {
                #pragma omp critical
                {
                    if (next_grid[ni][nj].type == EMPTY && !reproduced) {
                        next_grid[ni][nj] = (Cell){species, 1, 0, 0};
                        next_grid[i][j].energy -= 2;
                        reproduced = 1;
                    }
                }
            }
        }
    }
}

void print_state(int tick) {
    int p = 0, h = 0, c = 0;
    
    #pragma omp parallel for collapse(2) reduction(+:p, h, c)
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            switch (grid[i][j].type) {
                case PLANT: p++; break;
                case HERBIVORE: h++; break;
                case CARNIVORE: c++; break;
                default: break;
            }
        }
    }
    
    printf("\nTick %d\n", tick);
    printf("Plantas: %d, Herbivoros: %d, Carnivoros: %d\n", p, h, c);
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            switch (grid[i][j].type) {
                case PLANT: printf("P "); break;
                case HERBIVORE: printf("H "); break;
                case CARNIVORE: printf("C "); break;
                default: printf(". "); break;
            }
        }
        printf("\n");
    }
}