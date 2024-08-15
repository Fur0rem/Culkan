#ifndef CULKAN_H
#define CULKAN_H

/**
 * @file culkan.h
 * @brief The main header file for the Culkan library, and the API you should use.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

/**
 * @brief The debug flag. If defined, the library will print errors and other debug information
 */
#define DEBUG 1

/**
 * @brief Enum that holds the different error codes that can be returned by the library
 */
typedef enum {
	NO_ERROR,
	OUT_OF_BOUNDS_BINDING,
	FILE_NOT_FOUND,
	TOO_MANY_INVOCATIONS,
	NOT_ENOUGH_MEMORY,
} CulkanErrCodes;

/**
 * @brief Struct that holds the result of a function that can return an error
 */
typedef struct {
	VkResult	   vkResult;
	CulkanErrCodes ckResult;
} CulkanResult;

/**
 * @ Macro for a successful CulkanResult
 */
#define CULKAN_SUCCESS                                                                                                                               \
	(CulkanResult) {                                                                                                                                 \
		VK_SUCCESS, SUCCESS                                                                                                                          \
	}

/**
 * @brief Check for any errors after a Culkan function call
 *
 * It reads the result field of a Culkan instance.
 * If there are errors, it prints the error name and code, line and file where the error occurred, and exits the program.
 * @param culkan the Culkan instance to check the result of
 */
#define culkanCheckError(culkan)                                                                                                                     \
	if (DEBUG) __checkCulkanResult((culkan)->result, __FILE__, __LINE__);

/**
 * @brief Like culkanCheckError(), but with a custom message
 * @param culkan the Culkan instance to check the result of
 * @param message the message to print if there is an error
 */
#define culkanCheckErrorWithMessage(culkan, message)                                                                                                 \
	if (DEBUG) {                                                                                                                                     \
		if ((culkan)->result.ckResult != NO_ERROR) {                                                                                                 \
			fprintf(stderr, "ERROR : %s\n", message);                                                                                                \
		}                                                                                                                                            \
		__checkCulkanResult((culkan)->result, __FILE__, __LINE__);                                                                                   \
	}

/**
 * @brief Like culkanCheckError(), but for vulkan functions
 * @param vkResult the result of the vulkan function
 */
#define vkCheckError(vkResult)                                                                                                                       \
	if (DEBUG) __checkCulkanResult((CulkanResult){vkResult, NO_ERROR}, __FILE__, __LINE__);

/**
 * @brief Checks for any allocation errors after a malloc call
 *
 * If the allocation failed, it prints where the error occurred and exits the program
 * @param variable the variable to check the allocation of
 */
#define culkanCheckAllocation(variable)                                                                                                              \
	if (DEBUG)                                                                                                                                       \
		__checkCulkanResult((variable) == NULL ? (CulkanResult){VK_ERROR_OUT_OF_HOST_MEMORY, NO_ERROR} : (CulkanResult){VK_SUCCESS, NO_ERROR},       \
							__FILE__, __LINE__);

/**
 * @brief Custom malloc with checks for allocation errors
 * @param type the type of the variable to allocate
 * @param nbElems the number of elements to allocate
 * @return the allocated variable
 */
// Warning that type* should be enclosed in parentheses but if i do it C++ will cry
#define culkanMalloc(type, nbElems)                                                                                                                  \
	({                                                                                                                                               \
		type* ptr = (type*)malloc(sizeof(type) * (nbElems));                                                                                         \
		culkanCheckAllocation(ptr);                                                                                                                  \
		ptr;                                                                                                                                         \
	})

typedef struct {
	VkBufferCreateInfo* bufferCreateInfoVar;
	VkBuffer*			vkBufferVar;

	VkMemoryRequirements memoryRequirementsVar;
	VkMemoryAllocateInfo memoryAllocateInfoVar;

	// TODO : see if they are needed
	VkDevice	   deviceVar;
	VkDeviceMemory deviceMemoryVar;

	VkDescriptorSetLayoutBinding* layoutBindingVar;
	VkDescriptorPoolSize*		  poolSizeVar;
	VkDescriptorBufferInfo*		  bufferInfoVar;

	size_t sizeOfVar;
	void*  dataVar;
} GPUVariable;

typedef enum {
	STORAGE_BUFFER = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	UNIFORM_BUFFER = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	OUTPUT_BUFFER  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
} CulkanBindingType;

typedef struct {
	size_t			  size;
	CulkanBindingType type;
} CulkanBinding;

/*
 * Struct that holds the layout of the shader to communicate with the GPU
 */
typedef struct {
	uint32_t	   bindingCount;
	CulkanBinding* bindings;
} CulkanLayout;

typedef struct {
	uint16_t x, y, z;
} CulkanInvocations;

typedef struct {
	const CulkanLayout*				 layout;
	const char*						 shaderPath;
	CulkanInvocations				 invocations;
	GPUVariable*					 variables;
	CulkanResult					 result;
	VkApplicationInfo				 appInfo;
	VkInstanceCreateInfo			 createInfo;
	VkInstance						 instance;
	uint32_t						 physicalDeviceCount;
	VkPhysicalDevice*				 physicalDevices;
	VkPhysicalDevice				 physicalDevice;
	VkPhysicalDeviceProperties		 deviceProperties;
	uint32_t						 queueFamilyCount;
	VkQueueFamilyProperties*		 queueFamilies;
	VkDeviceQueueCreateInfo			 queueCreateInfo;
	uint32_t						 family;
	float*							 queuePriorities;
	VkDeviceCreateInfo				 deviceCreateInfo;
	VkDevice						 device;
	VkPhysicalDeviceMemoryProperties memoryProperties;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
	VkDescriptorSetLayout			descriptorSetLayout;
	VkDescriptorPoolSize*			poolSizeInfo;
	VkDescriptorPoolSize*			pPoolSizes;
	VkDescriptorPoolCreateInfo		descriptorPoolCreateInfo;
	VkDescriptorPool				descriptorPool;
	VkDescriptorSetAllocateInfo		descriptorSetAllocateInfo;
	VkDescriptorSet					descriptorSet;
	VkWriteDescriptorSet**			descriptorWritesVar;
	VkPipelineLayoutCreateInfo		pipelineLayoutCreateInfo;
	VkPipelineLayout				pipelineLayout;

	size_t	  fileSize;
	char*	  fileName;
	uint32_t* shaderBuffer;

	VkShaderModuleCreateInfo		shaderModuleCreateInfo;
	VkShaderModule					shaderModule;
	VkPipelineShaderStageCreateInfo stageCreateInfo;
	VkComputePipelineCreateInfo		pipelineCreateInfo;
	VkPipeline						pipeline;

	VkCommandPoolCreateInfo		commandPoolCreateInfo;
	VkCommandPool				commandPool;
	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	VkCommandBuffer				commandBuffer;
	VkCommandBufferBeginInfo	commandBufferBeginInfo;

	VkFence			  computeFence;
	VkFenceCreateInfo fenceCreateInfo;

	VkQueue queue;

	VkCommandBuffer* commandBuffers;
} Culkan;

/**
 * @brief Gets a GPUVariable from a Culkan instance
 * @param culkan the Culkan instance to get the variable from
 * @param binding the binding of the variable to get
 * @return the variable
 */
GPUVariable* culkanGetBinding(Culkan* culkan, uint32_t binding);

/**
 * @brief Writes data to a GPUVariable. Can be preferred over culkanWriteBinding, if you want explicit variable names
 * @param variable the variable to write to
 * @param src the data to write
 * @param result the result of the operation
 */
void culkanWriteGPUVariable(GPUVariable* variable, const void* src, CulkanResult* result);

/**
 * @brief Writes data to a binding of a Culkan instance. Can be preferred over culkanWriteGPUVariable, if you want to write to a binding directly
 * @param culkan the Culkan instance to write to
 * @param binding the binding to write to
 * @param src the data to write
 */
void culkanWriteBinding(Culkan* culkan, uint32_t binding, const void* src);

/**
 * @brief Reads data from a GPUVariable. Can be preferred over culkanReadBinding, if you want explicit variable names
 * @param variable the variable to read from
 * @param dst the destination to write the data to
 * @param result the result of the operation
 */
void culkanReadGPUVariable(GPUVariable* variable, void* dst, CulkanResult* result);

/**
 * @brief Reads data from a binding of a Culkan instance. Can be preferred over culkanReadGPUVariable, if you want to read from a binding directly
 * @param culkan the Culkan instance to read from
 * @param binding the binding to read from
 * @param dst the destination to write the data to
 */
void culkanReadBinding(Culkan* culkan, uint32_t binding, void* dst);

/**
 * @brief Initializes a Culkan instance. It allocates memory for the instance, so it should be freed after use by calling culkanDestroy()
 * @param layout the layout of the shader to use
 * @param shaderPath the path to the shader to use
 * @param workGroups the number of invocations to use
 * @return the created Culkan instance
 */
Culkan* culkanInit(const CulkanLayout* layout, const char* shaderPath, CulkanInvocations invocations);

/**
 * @brief Sets up the Culkan instance.
 * Should be called after writing to the bindings and before running the shader
 * @param culkan the Culkan instance to set up
 */
void culkanSetup(Culkan* culkan);

/**
 * @brief Runs the shader of a Culkan instance. Should be called after setting up the instance.
 * @param culkan the Culkan instance to run
 */
void culkanRun(Culkan* culkan);

/**
 * @brief Frees the memory of a Culkan instance
 * @param culkan the Culkan instance to free the memory of
 */
void culkanDestroy(Culkan* culkan);

#endif // CULKAN_H