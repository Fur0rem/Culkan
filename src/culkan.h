#ifndef CULKAN_H
#define CULKAN_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define DEBUG 1

/// Error handling
typedef enum {
	NO_ERROR,
	OUT_OF_BOUNDS_BINDING,
	FILE_NOT_FOUND,
	TOO_MANY_INVOCATIONS,
	NOT_ENOUGH_MEMORY,
} CulkanErrCodes;

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
			fprintf(stderr, "ERROR : %s\n", message);                                                                                                        \
		}                                                                                                                                            \
		__checkCulkanResult((culkan)->result, __FILE__, __LINE__);                                                                                   \
	}
#define vkCheckError(vkResult)                                                                                                                       \
	if (DEBUG) __checkCulkanResult((CulkanResult){vkResult, NO_ERROR}, __FILE__, __LINE__);
#define culkanCheckAllocation(variable)                                                                                                              \
	if (DEBUG)                                                                                                                                       \
		__checkCulkanResult((variable) == NULL ? (CulkanResult){VK_ERROR_OUT_OF_HOST_MEMORY, NO_ERROR} : (CulkanResult){VK_SUCCESS, NO_ERROR},       \
							__FILE__, __LINE__);
// FIXME : WHAT ON PEUT FAIRE CA EN C???????
// Warning that type* should be enclosed in parentheses but if i do it C++ will cry
#define culkanMalloc(type, nbElems) 																											   \
	({\
		type* ptr = (type*)malloc(sizeof(type) * (nbElems));																					   \
		culkanCheckAllocation(ptr); 																											   \
		ptr; 																																	   \
	})

const char* culkanErrCodeToString(CulkanErrCodes code);
const char* vkResultToString(VkResult result);
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

typedef struct {
	uint32_t bindingCount;
	CulkanBinding* bindings;
} CulkanLayout;

typedef struct {
	uint16_t x, y, z;
} CulkanInvocations;

typedef struct {
	// TODO : holy shit that's a lot of fields, clean that up
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

uint32_t* culkanOpenShader(const char* filename, size_t* fileSize, Culkan* culkan);

/// GPU GPUVariables
void freeGPUVariableData(GPUVariable* variable);
void freeGPUVariable(GPUVariable* variable);
GPUVariable* createGPUVariable(size_t sizeOfVar, VkBufferUsageFlags usage, uint32_t binding, uint32_t family, VkDevice device,
							   VkPhysicalDeviceMemoryProperties* memoryProperties, CulkanResult* result);

void writeGPUVariable(GPUVariable* variable, const void* src, CulkanResult* result);
void culkanWriteBinding(Culkan* culkan, uint32_t binding, const void* src);

void readGPUVariable(GPUVariable* variable, void* dst, CulkanResult* result);
void culkanReadBinding(Culkan* culkan, uint32_t binding, void* dst);

Culkan* culkanInit(const CulkanLayout* layout, const char* shaderPath, CulkanInvocations workGroups);
void culkanSetup(Culkan* culkan);
void culkanRun(Culkan* culkan);

//. Memory
void culkanGPUAlloc(Culkan* culkan);
void culkanDestroy(Culkan* culkan);

#endif // CULKAN_H