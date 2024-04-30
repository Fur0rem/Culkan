GLSLC = glslc
CC = gcc
CFLAGS = -g -Og -Wall -Wextra
SAMPLES_DIR = samples

# FIXME : wow this makefile is a mess

#make sample_vec_add && cd samples && ./vec_add && cd ../
sample_vec_add:
	cd $(SAMPLES_DIR) && $(GLSLC) vec_add.comp -o vec_add.spv 
	cd $(SAMPLES_DIR) && $(CC) $(CFLAGS) vec_add.c ../src/culkan.c -o vec_add -lvulkan

# make sample_matrix_sum && cd samples && ./matrix_sum && cd ../ 
sample_matrix_sum:
	cd $(SAMPLES_DIR) && $(GLSLC) matrix_sum.comp -o matrix_sum.spv 
	cd $(SAMPLES_DIR) && $(CC) $(CFLAGS) matrix_sum.c ../src/culkan.c -o matrix_sum -lvulkan

# Remove every file which is not a source file in samples directory
clean_samples:
	cd $(SAMPLES_DIR) && rm $(shell cd $(SAMPLES_DIR) && ls | grep -v -E "\.c")

doc:
	rm -rf doc && doxygen .doxygen