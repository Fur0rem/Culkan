#ifndef CULKAN_H
#define CULKAN_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define DEBUG 1

/**
 * Enum that holds the different error codes that can be returned by the library
 */
typedef enum {
	NO_ERROR,
	OUT_OF_BOUNDS_BINDING,
	FILE_NOT_FOUND,
	TOO_MANY_INVOCATIONS,
	NOT_ENOUGH_MEMORY,
} CulkanErrCodes;

/**
 * Struct that holds the result of a function that can return an error
*/
typedef struct {
	VkResult vkResult;
	CulkanErrCodes ckResult;
} CulkanResult;

#define CULKAN_SUCCESS(CulkanResult)                                                                                                                 \
	{ VK_SUCCESS, SUCCESS }
#define culkanCheckError(culkan)                                                                                                                     \
	if (DEBUG) __checkCulkanResult((culkan)->result, __FILE__, __LINE__);
#define culkanCheckErrorWithMessage(culkan, message)                                                                                                 \
	if (DEBUG) {                                                                                                                                     \
		if ((culkan)->result.ckResult != NO_ERROR) {                                                                                                 \
			fprintf(stderr, "ERROR : %s\n", message);                                                                                                \
		}                                                                                                                                            \
		__checkCulkanResult((culkan)->result, __FILE__, __LINE__);                                                                                   \
	}
#define vkCheckError(vkResult)                                                                                                                       \
	if (DEBUG) __checkCulkanResult((CulkanResult){vkResult, NO_ERROR}, __FILE__, __LINE__);
#define culkanCheckAllocation(variable)                                                                                                              \
	if (DEBUG)                                                                                                                                       \
		__checkCulkanResult((variable) == NULL ? (CulkanResult){VK_ERROR_OUT_OF_HOST_MEMORY, NO_ERROR} : (CulkanResult){VK_SUCCESS, NO_ERROR},       \
							__FILE__, __LINE__);
// Warning that type* should be enclosed in parentheses but if i do it C++ will cry
#define culkanMalloc(type, nbElems)                                                                                                                  \
	({                                                                                                                                               \
		type* ptr = (type*)malloc(sizeof(type) * (nbElems));                                                                                         \
		culkanCheckAllocation(ptr);                                                                                                                  \
		ptr;                                                                                                                                         \
	})

/**
 * Converts a CulkanErrCodes to a string
 * Allocates memory for the string, so it should be freed after use
 * \param code the CulkanErrCodes to convert
 * \return the string representation of the CulkanErrCodes
 */
const char* culkanErrCodeToString(CulkanErrCodes code);

/**
 * Prints to stderr the error message corresponding to either the Vulkan result or the CulkanErrCodes.
 * Exits the program if there are any errors
 * Should never be called directly, use culkanCheckError macros instead
 * \param result the result to check
 * \param file the file where the error occured
 * \param line the line where the error occured
 */
void __checkCulkanResult(CulkanResult result, const char* file, int line);

/// Vulkan objects
VkWriteDescriptorSet* createDescriptorSetWrite(VkDescriptorSet descriptorSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding);
VkDescriptorBufferInfo* createDescriptorBufferInfo(VkBuffer buffer, uint32_t size);
VkDescriptorSetLayoutBinding* createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags);
VkBufferCreateInfo* createBufferCreateInfo(uint32_t size, VkBufferUsageFlags usage, uint32_t family);
VkBuffer* createBuffer(VkDevice device, VkBufferCreateInfo* bufferCreateInfo);
VkDescriptorPoolSize* createDescriptorPoolSize(VkDescriptorType descriptorType, uint32_t descriptorCount);

typedef struct {
	VkBufferCreateInfo* bufferCreateInfoVar;
	VkBuffer* vkBufferVar;

	VkMemoryRequirements memoryRequirementsVar;
	VkMemoryAllocateInfo memoryAllocateInfoVar;

	// TODO : see if they are needed
	VkDevice deviceVar;
	VkDeviceMemory deviceMemoryVar;

	VkDescriptorSetLayoutBinding* layoutBindingVar;
	VkDescriptorPoolSize* poolSizeVar;
	VkDescriptorBufferInfo* bufferInfoVar;

	size_t sizeOfVar;
	void* dataVar;
} GPUVariable;

typedef enum {
	STORAGE_BUFFER = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	UNIFORM_BUFFER = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	OUTPUT_BUFFER = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
} CulkanBindingType;

typedef struct {
	size_t size;
	CulkanBindingType type;
} CulkanBinding;

/*
 * Struct that holds the layout of the shader to communicate with the GPU
*/
typedef struct {
	uint32_t bindingCount;
	CulkanBinding* bindings;
} CulkanLayout;

typedef struct {
	uint16_t x, y, z;
} CulkanInvocations;

typedef struct {
	const CulkanLayout* layout;
	const char* shaderPath;
	CulkanInvocations invocations;
	GPUVariable* variables;
	CulkanResult result;
	VkApplicationInfo appInfo;
	VkInstanceCreateInfo createInfo;
	VkInstance instance;
	uint32_t physicalDeviceCount;
	VkPhysicalDevice* physicalDevices;
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties deviceProperties;
	uint32_t queueFamilyCount;
	VkQueueFamilyProperties* queueFamilies;
	VkDeviceQueueCreateInfo queueCreateInfo;
	uint32_t family;
	float* queuePriorities;
	VkDeviceCreateInfo deviceCreateInfo;
	VkDevice device;
	VkPhysicalDeviceMemoryProperties memoryProperties;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPoolSize* poolSizeInfo;
	VkDescriptorPoolSize* pPoolSizes;
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
	VkDescriptorSet descriptorSet;
	VkWriteDescriptorSet** descriptorWritesVar;
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	VkPipelineLayout pipelineLayout;

	size_t fileSize;
	char* fileName;
	uint32_t* shaderBuffer;

	VkShaderModuleCreateInfo shaderModuleCreateInfo;
	VkShaderModule shaderModule;
	VkPipelineShaderStageCreateInfo stageCreateInfo;
	VkComputePipelineCreateInfo pipelineCreateInfo;
	VkPipeline pipeline;

	VkCommandPoolCreateInfo commandPoolCreateInfo;
	VkCommandPool commandPool;
	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	VkCommandBuffer commandBuffer;
	VkCommandBufferBeginInfo commandBufferBeginInfo;

	VkFence computeFence;
	VkFenceCreateInfo fenceCreateInfo;

	VkQueue queue;

	VkCommandBuffer* commandBuffers;
} Culkan;

/**
 * Opens a file and reads its content into a buffer
 * Allocates memory for the buffer, so it should be freed after use
 * \param filename the name of the file to open
 * \param fileSize a pointer to the size of the file that will be written
 * \param culkan the Culkan instance to use
 * \return the buffer containing the content of the file
 */
uint32_t* culkanOpenShader(const char* filename, size_t* fileSize, Culkan* culkan);


/**
 * Creates a variable that lives on the GPU
 * Allocates memory for the variable, so it should be freed after use
 * Should not be called directly, the Culkan instance allocates the memory for the variables itself based on the layout
 * \param sizeOfVar the size of the variable to create
 * \param usage the usage of the variable
 * \param binding the binding of the variable
 * \param family the family of the variable
 * \param device the device to use
 * \param memoryProperties the memory properties of the device
 * \param result the result of the operation
 * \return the created variable
 */
GPUVariable* createGPUVariable(size_t sizeOfVar, VkBufferUsageFlags usage, uint32_t binding, uint32_t family, VkDevice device,
							   VkPhysicalDeviceMemoryProperties* memoryProperties, CulkanResult* result);
/**
 * Frees the data of a GPUVariable
 * Should not be called directly, the Culkan instance frees the memory for the variables itself
 * It only frees the data, not the variable itself, in case you have an array of variables for example
 * \param variable the variable to free
 */
void freeGPUVariableData(GPUVariable* variable);
/**
 * Frees a GPUVariable
 * Should not be called directly, the Culkan instance frees the memory for the variables itself
 * \param variable the variable to free
 */
void freeGPUVariable(GPUVariable* variable);

/**
 * Gets a GPUVariable from a Culkan instance
 * \param culkan the Culkan instance to get the variable from
 * \param binding the binding of the variable to get
 * \return the variable
 */
GPUVariable* culkanGetBinding(Culkan* culkan, uint32_t binding);

/**
 * Writes data to a GPUVariable
 * Can be preferred over culkanWriteBinding, if you want explicit variable names
 * \param variable the variable to write to
 * \param src the data to write
 * \param result the result of the operation
 */
void culkanWriteGPUVariable(GPUVariable* variable, const void* src, CulkanResult* result);

/**
 * Writes data to a binding of a Culkan instance
 * Can be preferred over culkanWriteGPUVariable, if you want to write to a binding directly
 * \param culkan the Culkan instance to write to
 * \param binding the binding to write to
 * \param src the data to write
 */
void culkanWriteBinding(Culkan* culkan, uint32_t binding, const void* src);

/**
 * Reads data from a GPUVariable
 * Can be preferred over culkanReadBinding, if you want explicit variable names
 * \param variable the variable to read from
 * \param dst the destination to write the data to
 * \param result the result of the operation
 */
void culkanReadGPUVariable(GPUVariable* variable, void* dst, CulkanResult* result);

/**
 * Reads data from a binding of a Culkan instance
 * Can be preferred over culkanReadGPUVariable, if you want to read from a binding directly
 * \param culkan the Culkan instance to read from
 * \param binding the binding to read from
 * \param dst the destination to write the data to
 */
void culkanReadBinding(Culkan* culkan, uint32_t binding, void* dst);

/**
 * Initializes a Culkan instance
 * Allocates memory for the instance, so it should be freed after use by calling culkanDestroy
 * \param layout the layout of the shader to use
 * \param shaderPath the path to the shader to use
 * \param workGroups the number of invocations to use
 * \return the created Culkan instance
 */
Culkan* culkanInit(const CulkanLayout* layout, const char* shaderPath, CulkanInvocations workGroups);

/**
 * Sets up the Culkan instance
 * Should be called after writing to the bindings and before running the shader
 * \param culkan the Culkan instance to set up
 */
void culkanSetup(Culkan* culkan);

/**
 * Runs the shader of a Culkan instance
 * Should be called after setting up the instance
 * \param culkan the Culkan instance to run
 */
void culkanRun(Culkan* culkan);

/**
 * Allocates memory for the GPUVariables of a Culkan instance
 * Should not be called directly, culkanSetup does it for you
 * \param culkan the Culkan instance to allocate memory for
 */
void culkanGPUAlloc(Culkan* culkan);

/**
 * Frees the memory used by the Culkan instance
 * \param culkan the Culkan instance to free
 */
void culkanDestroy(Culkan* culkan);

#endif // CULKAN_H