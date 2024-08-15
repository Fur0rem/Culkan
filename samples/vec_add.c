#include "../src/culkan.h"
#include <stdio.h>
#include <stdlib.h>

int main() {

	const int array_size = 10240;

	float* arrayA = malloc(array_size * sizeof(float));
	float* arrayB = malloc(array_size * sizeof(float));

	for (int i = 0; i < array_size; i++) {
		arrayA[i] = (float)i;
		arrayB[i] = (float)i;
	}

	CulkanLayout layout = {.bindingCount = 4,
						   .bindings	 = (CulkanBinding[]){
							   {
									   .size = array_size * sizeof(float),
									   .type = STORAGE_BUFFER,
							   },
							   {
									   .size = array_size * sizeof(float),
									   .type = STORAGE_BUFFER,
							   },
							   {
									   .size = array_size * sizeof(float),
									   .type = OUTPUT_BUFFER,
							   },
							   {
									   .size = sizeof(int),
									   .type = UNIFORM_BUFFER,
							   },
						   }};

	Culkan* culkan = culkanInit(&layout, "samples/bin/vec_add.spv", (CulkanInvocations){1024, 1, 1});

	GPUVariable* GPUArrayA	  = culkanGetBinding(culkan, 0);
	GPUVariable* GPUArrayB	  = culkanGetBinding(culkan, 1);
	GPUVariable* GPUArraySize = culkanGetBinding(culkan, 3);

	culkanWriteGPUVariable(GPUArrayA, arrayA, &culkan->result);
	culkanWriteGPUVariable(GPUArrayB, arrayB, &culkan->result);
	culkanWriteGPUVariable(GPUArraySize, &array_size, &culkan->result);

	culkanSetup(culkan);
	culkanRun(culkan);

	float* result = malloc(array_size * sizeof(float));
	culkanReadBinding(culkan, 2, result);

	printf("Result: \n");
	for (int i = 0; i < array_size; i++) {
		printf("%-8.2f ", result[i]);
	}
	printf("\n");

	culkanDestroy(culkan);

	free(arrayA);
	free(arrayB);
	free(result);

	return 0;
}