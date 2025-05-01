#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>

#define SIZE 9
#define SUBGRID_SIZE 3

// Structure to hold Sudoku data
typedef struct {
    int grid[SIZE][SIZE];  // 9x9 grid
    bool valid;            // Validity flag
    pthread_mutex_t mutex; // Mutex for thread-safe updates
} SudokuData;

// Check if a row contains numbers 1-9 exactly once
bool check_row(int grid[SIZE][SIZE], int row) {
    bool seen[SIZE] = {false};
    for (int col = 0; col < SIZE; col++) {
        int num = grid[row][col];
        if (num < 1 || num > SIZE || seen[num - 1]) {
            return false;
        }
        seen[num - 1] = true;
    }
    return true;
}

// Check if a column contains numbers 1-9 exactly once
bool check_col(int grid[SIZE][SIZE], int col) {
    bool seen[SIZE] = {false};
    for (int row = 0; row < SIZE; row++) {
        int num = grid[row][col];
        if (num < 1 || num > SIZE || seen[num - 1]) {
            return false;
        }
        seen[num - 1] = true;
    }
    return true;
}

// Check if a 3x3 subgrid contains numbers 1-9 exactly once
bool check_subgrid(int grid[SIZE][SIZE], int start_row, int start_col) {
    bool seen[SIZE] = {false};
    for (int i = 0; i < SUBGRID_SIZE; i++) {
        for (int j = 0; j < SUBGRID_SIZE; j++) {
            int num = grid[start_row + i][start_col + j];
            if (num < 1 || num > SIZE || seen[num - 1]) {
                return false;
            }
            seen[num - 1] = true;
        }
    }
    return true;
}

// Thread function: Validate all rows
void* validate_rows(void* arg) {
    SudokuData* data = (SudokuData*)arg;
    for (int row = 0; row < SIZE; row++) {
        if (!check_row(data->grid, row)) {
            pthread_mutex_lock(&data->mutex);
            data->valid = false;
            pthread_mutex_unlock(&data->mutex);
        }
        // Early exit if invalid
        pthread_mutex_lock(&data->mutex);
        bool current_valid = data->valid;
        pthread_mutex_unlock(&data->mutex);
        if (!current_valid) break;
    }
    return NULL;
}

// Thread function: Validate all columns
void* validate_cols(void* arg) {
    SudokuData* data = (SudokuData*)arg;
    for (int col = 0; col < SIZE; col++) {
        if (!check_col(data->grid, col)) {
            pthread_mutex_lock(&data->mutex);
            data->valid = false;
            pthread_mutex_unlock(&data->mutex);
        }
        // Early exit if invalid
        pthread_mutex_lock(&data->mutex);
        bool current_valid = data->valid;
        pthread_mutex_unlock(&data->mutex);
        if (!current_valid) break;
    }
    return NULL;
}

// Thread function: Validate all 3x3 subgrids
void* validate_subgrids(void* arg) {
    SudokuData* data = (SudokuData*)arg;
    for (int row = 0; row < SIZE; row += SUBGRID_SIZE) {
        for (int col = 0; col < SIZE; col += SUBGRID_SIZE) {
            if (!check_subgrid(data->grid, row, col)) {
                pthread_mutex_lock(&data->mutex);
                data->valid = false;
                pthread_mutex_unlock(&data->mutex);
            }
            // Early exit if invalid
            pthread_mutex_lock(&data->mutex);
            bool current_valid = data->valid;
            pthread_mutex_unlock(&data->mutex);
            if (!current_valid) return NULL;
        }
    }
    return NULL;
}

int main() {
    SudokuData data = {
        .valid = true,
        .mutex = PTHREAD_MUTEX_INITIALIZER
    };

    // Read 9x9 Sudoku grid input
    printf("Enter the 9x9 Sudoku grid (row-wise, numbers 1-9):\n");
    for (int row = 0; row < SIZE; row++) {
        printf("Row %d: ", row + 1);
        for (int col = 0; col < SIZE; col++) {
            scanf("%d", &data.grid[row][col]);
        }
    }

    // Create threads
    pthread_t row_thread, col_thread, subgrid_thread;
    pthread_create(&row_thread, NULL, validate_rows, &data);
    pthread_create(&col_thread, NULL, validate_cols, &data);
    pthread_create(&subgrid_thread, NULL, validate_subgrids, &data);

    // Wait for threads to finish
    pthread_join(row_thread, NULL);
    pthread_join(col_thread, NULL);
    pthread_join(subgrid_thread, NULL);

    // Output result
    printf("Sudoku is %s\n", data.valid ? "VALID!" : "INVALID!");
    return 0;
}