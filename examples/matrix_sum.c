#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include "../culkan.h"


int main() {

    const int matrix_rows = 4;
    const int matrix_cols = 7;
    int* matrix = malloc(matrix_rows * matrix_cols * sizeof(int));
    for (int i = 0; i < matrix_rows * matrix_cols; i++) {
        matrix[i] = i;
    }
    const int matrix_size = matrix_rows * matrix_cols;

    CulkanLayout layout = {
        3,
        (CulkanBinding[]) {
            {   
                .size = matrix_size * sizeof(int),
                .type = STORAGE_BUFFER,
            },
            {
                .size = sizeof(int),
                .type = UNIFORM_BUFFER,
            },
            {
                .size = sizeof(int),
                .type = STORAGE_BUFFER, 
            },
        }
    };

    Culkan* culkan = culkanInit(&layout, "matrix_sum.spv", (CulkanInvocations) {1024, 1, 1});

    culkanWriteBinding(culkan, 0, matrix);
    culkanWriteBinding(culkan, 1, &matrix_size);

    culkanSetup(culkan);

    culkanRun(culkan);

    int result;
    culkanReadBinding(culkan, 2, &result);

    printf("GPU Sum: %d\n", result);
    int cpu_sum = 0;
    for (int i = 0; i < matrix_size; i++) {
        cpu_sum += matrix[i];
    }
    printf("CPU sum: %d\n", cpu_sum);
    
    culkanDestroy(culkan);

    free(matrix);

    return 0;

}