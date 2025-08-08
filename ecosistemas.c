#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define SIZE 20
#define TICKS 20
//ACa estamos probando con los parametos del pdf. 
#define INITIAL_PLANTS 150
#define INITIAL_HERBIVORES 40
#define INITIAL_CARNIVORES 15
#define PLANT_REPRODUCTION_RATE 30
#define ANIMAL_REPRODUCTION_RATE 20
#define HERB_ENERGY_GAIN 1
#define CARN_ENERGY_GAIN 2
#define MAX_NO_FOOD_TICKS 3
#define REPRODUCTION_ENERGY_THRESHOLD 3

typedef enum { EMPTY, PLANT, HERBIVORE, CARNIVORE } Species;

typedef struct {
    Species type;
    int energy;
    int age;
    int ticks_no_food;
    int moved_this_tick; // evitar múltiples movimientos
} Cell;

Cell grid[SIZE][SIZE];
Cell next_grid[SIZE][SIZE];

// Prototipos
void initialize();
void simulate();
void update_plants(int i, int j);
void update_herbivores(int i, int j);
void update_carnivores(int i, int j);
void print_state(int tick);
void copy_grid();
int count_adjacent_empty(int i, int j);
void get_random_empty_adjacent(int i, int j, int* ni, int* nj);

int main() {
    srand(time(NULL));
    initialize();
    print_state(0); // Estado inicial
    simulate();
    return 0;
}

void initialize() {
    // Inicializar todo como vacío
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = (Cell){EMPTY, 0, 0, 0, 0};
        }
    }

    // Colocar plantas
    int placed = 0;
    while (placed < INITIAL_PLANTS) {
        int x = rand() % SIZE, y = rand() % SIZE;
        if (grid[x][y].type == EMPTY) {
            grid[x][y] = (Cell){PLANT, 1, 0, 0, 0};
            placed++;
        }
    }

    // Colocar herbívoros
    placed = 0;
    while (placed < INITIAL_HERBIVORES) {
        int x = rand() % SIZE, y = rand() % SIZE;
        if (grid[x][y].type == EMPTY) {
            grid[x][y] = (Cell){HERBIVORE, 2, 0, 0, 0};
            placed++;
        }
    }

    // Colocar carnívoros
    placed = 0;
    while (placed < INITIAL_CARNIVORES) {
        int x = rand() % SIZE, y = rand() % SIZE;
        if (grid[x][y].type == EMPTY) {
            grid[x][y] = (Cell){CARNIVORE, 2, 0, 0, 0};
            placed++;
        }
    }
}

void simulate() {
    for (int t = 1; t <= TICKS; t++) {
        // Copiar grid actual a next_grid
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                next_grid[i][j] = grid[i][j];
                next_grid[i][j].moved_this_tick = 0;
            }
        }

        // Procesar cada tipo de especie por separado para evitar conflictos
        #pragma omp parallel for collapse(2)
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (grid[i][j].type == PLANT) {
                    update_plants(i, j);
                }
            }
        }

        #pragma omp parallel for collapse(2)
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (grid[i][j].type == HERBIVORE && !grid[i][j].moved_this_tick) {
                    update_herbivores(i, j);
                }
            }
        }

        #pragma omp parallel for collapse(2)
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (grid[i][j].type == CARNIVORE && !grid[i][j].moved_this_tick) {
                    update_carnivores(i, j);
                }
            }
        }

        copy_grid();
        print_state(t);
    }
}

void update_plants(int i, int j) {
    Cell current = grid[i][j];
    int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    
    // Envejecer planta
    next_grid[i][j].age++;
    
    // Intentar reproducirse
    for (int d = 0; d < 4; d++) {
        int ni = i + dirs[d][0], nj = j + dirs[d][1];
        if (ni >= 0 && ni < SIZE && nj >= 0 && nj < SIZE) {
            if (grid[ni][nj].type == EMPTY && (rand() % 100) < PLANT_REPRODUCTION_RATE) {
                #pragma omp critical
                {
                    if (next_grid[ni][nj].type == EMPTY) {
                        next_grid[ni][nj] = (Cell){PLANT, 1, 0, 0, 0};
                    }
                }
            }
        }
    }
}

void update_herbivores(int i, int j) {

}

void update_carnivores(int i, int j) {

}

void get_random_empty_adjacent(int i, int j, int* ni, int* nj) {

}

void copy_grid() {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = next_grid[i][j];
        }
    }
}

void print_state(int tick) {

}