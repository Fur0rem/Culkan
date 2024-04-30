GLSLC = glslc
CC = gcc
CFLAGS = -g -Og -Wall -Wextra
SAMPLES_DIR = samples

# FIXME : wow this makefile is a mess

#make example_vec_add && cd samples && ./vec_add && cd ../
example_vec_add:
	cd $(SAMPLES_DIR) && $(GLSLC) vec_add.comp -o vec_add.spv 
	cd $(SAMPLES_DIR) && $(CC) $(CFLAGS) vec_add.c ../src/culkan.c -o vec_add -lvulkan

# make example_matrix_sum && cd samples && ./matrix_sum && cd ../ 
example_matrix_sum:
	cd $(SAMPLES_DIR) && $(GLSLC) matrix_sum.comp -o matrix_sum.spv 
	cd $(SAMPLES_DIR) && $(CC) $(CFLAGS) matrix_sum.c ../src/culkan.c -o matrix_sum -lvulkan

# Remove every file which is not a source file (doesnt end in .c or .h or .comp in samples)
clean_samples:
	cd $(SAMPLES_DIR) && rm -f $(shell ls | grep -v -E '\.c|\.h|\.comp')