#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define SIZE 20
#define TICKS 20
#define INITIAL_PLANTS 150
#define INITIAL_HERBIVORES 40
#define INITIAL_CARNIVORES 15
#define PLANT_REPRODUCTION_RATE 30 // porcentaje
#define HERB_ENERGY_GAIN 1
#define CARN_ENERGY_GAIN 2
#define MAX_NO_FOOD_TICKS 3

typedef enum { EMPTY, PLANT, HERBIVORE, CARNIVORE } Species;

typedef struct {
    Species type;
    int energy;
    int age;
    int ticks_no_food;
} Cell;

Cell grid[SIZE][SIZE];
Cell next_grid[SIZE][SIZE]; // para evitar condiciones de carrera



// Prototipos
void initialize();
void simulate();
void update_cell(int i, int j);
void print_state(int tick);
void copy_grid();

// Inicializa el ecosistema con especies aleatorias
void initialize() {
}

// Simula por ticks
void simulate() {
}

void update_cell(int i, int j) {
    // Actualiza el estado de la celda (i, j) según las reglas del ecosistema
}

void print_state(int tick) {
    // Imprime el estado del ecosistema en el tick actual

}
void copy_grid() {
    // Copia el estado de next_grid a grid
}