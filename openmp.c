typedef enum { EMPTY, PLANT, HERBIVORE, CARNIVORE } Species;

typedef struct {
    Species type;
    int energy;
    int age;
    int ticks_without_food;
} Cell;
