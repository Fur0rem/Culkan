GLSLC = glslc
CC = gcc
CFLAGS = -g -Og
EXAMPLE_DIR = examples

#make example_vec_add && cd examples && ./vec_add && cd ../
example_vec_add:
	cd $(EXAMPLE_DIR) && $(GLSLC) vec_add.comp -o vec_add.spv 
	cd $(EXAMPLE_DIR) && $(CC) $(CFLAGS) vec_add.c ../culkan.c -o vec_add -lvulkan

# make example_matrix_sum && cd examples && ./matrix_sum && cd ../ 
example_matrix_sum:
	cd $(EXAMPLE_DIR) && $(GLSLC) matrix_sum.comp -o matrix_sum.spv 
	cd $(EXAMPLE_DIR) && $(CC) $(CFLAGS) matrix_sum.c ../culkan.c -o matrix_sum -lvulkan

# Remove every file which is not a source file (doesnt end in .c or .h or .comp in examples)
clean_examples:
	cd $(EXAMPLE_DIR) && rm -f $(filter-out %.c %.h %.comp, $(wildcard *))