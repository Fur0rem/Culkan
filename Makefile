GLSLC = glslc
CC = gcc
DEBUG_FLAGS = -g -Og
RELEASE_FLAGS = -O3
CFLAGS = $(RELEASE_FLAGS) -Wall -Wextra
SAMPLES_DIR = samples

culkan: src/culkan.c src/culkan.h
	gcc -c src/culkan.c -o bin/culkan.o $(CFLAGS)

sample_vec_add: culkan
	cd $(SAMPLES_DIR) && $(GLSLC) vec_add.comp -o bin/vec_add.spv 
	cd $(SAMPLES_DIR) && $(CC) $(CFLAGS) vec_add.c ../bin/culkan.o -o bin/vec_add -lvulkan

sample_matrix_sum: culkan
	cd $(SAMPLES_DIR) && $(GLSLC) matrix_sum.comp -o matrix_sum.spv 
	cd $(SAMPLES_DIR) && $(CC) $(CFLAGS) matrix_sum.c ../src/culkan.c -o matrix_sum -lvulkan

# Remove every file which is not a source file in samples directory
clean_samples:
	cd $(SAMPLES_DIR) && rm $(shell cd $(SAMPLES_DIR) && ls | grep -v -E "\.c")

doc:
	rm -rf doc && doxygen .doxygen