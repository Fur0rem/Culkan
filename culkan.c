#include "culkan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO : fix variable cases and make it compatible with g++

const char* culkanErrCodeToString(CulkanErrCodes code) {
	switch (code) {
		case NO_ERROR:
			return "No error";
		case OUT_OF_BOUNDS_BINDING:
			return "Out of bounds binding";
        case FILE_NOT_FOUND:
            return "File not found";
		case TOO_MANY_INVOCATIONS:
			return "Too many invocations";
		case NOT_ENOUGH_MEMORY:
			return "Not enough memory";
		default:
			return "Unknown error";
	}
}

const char* vkResultToString(VkResult result) {
	switch (result) {
		case VK_SUCCESS:
			return "Success";
		// TODO : Write more cases
		default:
			return "Unknown error";
	}
}

// TODO : Maybe make these two generic so you can call either one with a CulkanResult, VkResult or Culkan
void __checkCulkanResult(CulkanResult result, const char* file, int line) {
	int8_t should_exit = 0;
	if (result.vkResult != VK_SUCCESS) {
		printf("Vulkan error at %s:%d: %s (%d)\n", file, line, vkResultToString(result.vkResult), result.vkResult);
	}
	if (result.ckResult != NO_ERROR) {
		printf("Culkan error at %s:%d: %s\n", file, line, culkanErrCodeToString(result.ckResult));
		should_exit = 1;
	}
	if (should_exit) {
		exit(1);
	}
}

void __checkCulkanError(Culkan* culkan, const char* file, int line) {
	__checkCulkanResult(culkan->result, file, line);
}

VkWriteDescriptorSet* createDescriptorSetWrite(VkDescriptorSet descriptorSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding) {
	VkWriteDescriptorSet* descriptorWrite = culkanMalloc(sizeof(VkWriteDescriptorSet));
	*descriptorWrite = (VkWriteDescriptorSet){
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = NULL,
		.dstSet = descriptorSet,
		.dstBinding = binding,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pImageInfo = NULL,
		.pBufferInfo = bufferInfo,
		.pTexelBufferView = NULL,
	};
	return descriptorWrite;
}

VkDescriptorBufferInfo* createDescriptorBufferInfo(VkBuffer buffer, uint32_t size) {
	VkDescriptorBufferInfo* bufferInfo = culkanMalloc(sizeof(VkDescriptorBufferInfo));
	*bufferInfo = (VkDescriptorBufferInfo){
		.buffer = buffer,
		.offset = 0,
		.range = size,
	};
	return bufferInfo;
}

VkDescriptorSetLayoutBinding* createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags) {
	VkDescriptorSetLayoutBinding* layoutBinding = culkanMalloc(sizeof(VkDescriptorSetLayoutBinding));
	*layoutBinding = (VkDescriptorSetLayoutBinding){
		.binding = binding,
		.descriptorType = descriptorType,
		.descriptorCount = 1,
		.stageFlags = stageFlags,
		.pImmutableSamplers = NULL,
	};
	return layoutBinding;
}

VkBufferCreateInfo* createBufferCreateInfo(uint32_t size, VkBufferUsageFlags usage, uint32_t family) {
	VkBufferCreateInfo* bufferCreateInfo = culkanMalloc(sizeof(VkBufferCreateInfo));
	*bufferCreateInfo = (VkBufferCreateInfo){
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices = &family,
	};
	return bufferCreateInfo;
}

VkBuffer* createBuffer(VkDevice device, VkBufferCreateInfo* bufferCreateInfo) {
	VkBuffer* buffer = culkanMalloc(sizeof(VkBuffer));
	VkResult result = vkCreateBuffer(device, bufferCreateInfo, NULL, buffer);
	vkCheckError(result);
	return buffer;
}

VkDescriptorPoolSize* createDescriptorPoolSize(VkDescriptorType descriptorType, uint32_t descriptorCount) {
	VkDescriptorPoolSize* poolSize = culkanMalloc(sizeof(VkDescriptorPoolSize));
	*poolSize = (VkDescriptorPoolSize){
		.type = descriptorType,
		.descriptorCount = descriptorCount,
	};
	return poolSize;
}

uint32_t* culkanOpenShader(const char* filename, long* fileSize, Culkan* culkan) {
	FILE* file = fopen(filename, "rb");
	if (!file) {
		culkan->result.ckResult = FILE_NOT_FOUND;
		culkanCheckErrorWithMessage(culkan, filename);
	}
	fseek(file, 0, SEEK_END);
	*fileSize = ftell(file);
	rewind(file);
	uint32_t* buffer = culkanMalloc(*fileSize);
	fread(buffer, *fileSize, 1, file);
	fclose(file);
	culkan->result.ckResult = NO_ERROR;
	return buffer;
}

void freeGPUVariableData(GPUVariable* variable) {
	free(variable->bufferCreateInfoVar);
	free(variable->vkBufferVar);
	free(variable->layoutBindingVar);
	free(variable->poolSizeVar);
	free(variable->bufferInfoVar);
}

void freeGPUVariable(GPUVariable *variable) {
	freeGPUVariableData(variable);
	free(variable);
}

GPUVariable* createGPUVariable(size_t sizeOfVar, VkBufferUsageFlags usage, uint32_t binding, uint32_t family, VkDevice device,
								VkPhysicalDeviceMemoryProperties* memoryProperties, CulkanResult* result) {
	GPUVariable* variable = culkanMalloc(sizeof(GPUVariable));
	variable->bufferCreateInfoVar = createBufferCreateInfo(sizeOfVar, usage, family);
	variable->vkBufferVar = createBuffer(device, variable->bufferCreateInfoVar);
	variable->sizeOfVar = sizeOfVar;
	variable->deviceVar = device;
	vkGetBufferMemoryRequirements(device, *variable->vkBufferVar, &variable->memoryRequirementsVar);
	variable->memoryAllocateInfoVar = (VkMemoryAllocateInfo){
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = NULL,
		.allocationSize = variable->memoryRequirementsVar.size,
		.memoryTypeIndex = 0,
	};
	for (uint32_t i = 0; i < memoryProperties->memoryTypeCount; i++) {
		if (variable->memoryRequirementsVar.memoryTypeBits & (1 << i) &&
			memoryProperties->memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			variable->memoryAllocateInfoVar.memoryTypeIndex = i;
			break;
		}
	}
	result->vkResult = vkAllocateMemory(device, &variable->memoryAllocateInfoVar, NULL, &variable->deviceMemoryVar);
	vkCheckError(result->vkResult);
	result->vkResult = vkBindBufferMemory(device, *variable->vkBufferVar, variable->deviceMemoryVar, 0);
	vkCheckError(result->vkResult);

	variable->layoutBindingVar = createDescriptorSetLayoutBinding(binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT);
	variable->poolSizeVar = createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
	variable->bufferInfoVar = createDescriptorBufferInfo(*variable->vkBufferVar, variable->sizeOfVar);

	return variable;
}

void writeGPUVariable(GPUVariable* variable, const void* src, CulkanResult* result) {
	result->vkResult = vkMapMemory(variable->deviceVar, variable->deviceMemoryVar, 0, VK_WHOLE_SIZE, 0, &variable->dataVar);
	vkCheckError(result->vkResult);
	memcpy(variable->dataVar, src, variable->sizeOfVar);
	vkUnmapMemory(variable->deviceVar, variable->deviceMemoryVar);
}

void culkanWriteBinding(Culkan* culkan, uint32_t binding, const void* src) {
	if (binding >= culkan->layout->bindingCount) {
		culkan->result = (CulkanResult){VK_ERROR_UNKNOWN, OUT_OF_BOUNDS_BINDING};
		return;
	}
	writeGPUVariable(&culkan->variables[binding], src, &culkan->result);
}

void readGPUVariable(GPUVariable* variable, void* dst, CulkanResult* result) {
	result->vkResult = vkMapMemory(variable->deviceVar, variable->deviceMemoryVar, 0, VK_WHOLE_SIZE, 0, &variable->dataVar);
	vkCheckError(result->vkResult);
	memcpy(dst, variable->dataVar, variable->sizeOfVar);
	vkUnmapMemory(variable->deviceVar, variable->deviceMemoryVar);
}

void culkanReadBinding(Culkan* culkan, uint32_t binding, void* dst) {
	if (binding >= culkan->layout->bindingCount) {
		culkan->result = (CulkanResult){VK_ERROR_UNKNOWN, OUT_OF_BOUNDS_BINDING};
		culkanCheckError(culkan);
	}
	readGPUVariable(&culkan->variables[binding], dst, &culkan->result);
}

Culkan* culkanInit(const CulkanLayout* layout, const char* shaderPath, CulkanInvocations invocations) {
	Culkan* culkan = culkanMalloc(sizeof(Culkan));
	culkan->layout = layout;
	culkan->shaderPath = shaderPath;
	culkan->invocations = invocations;

	culkan->appInfo = (VkApplicationInfo){
		// TODO : Maybe let the user choose the API version?
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = NULL,
		.pApplicationName = "CulkanApp",
		.applicationVersion = 0,
		.pEngineName = NULL,
		.engineVersion = 0,
		.apiVersion = VK_API_VERSION_1_3,
	};

	culkan->createInfo = (VkInstanceCreateInfo){
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.pApplicationInfo = &culkan->appInfo,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = NULL,
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = NULL,
	};

	culkan->result.vkResult = vkCreateInstance(&culkan->createInfo, NULL, &culkan->instance);
	culkanCheckError(culkan);

	culkan->result.vkResult = vkEnumeratePhysicalDevices(culkan->instance, &culkan->physicalDeviceCount, NULL);
	culkanCheckError(culkan);

	culkan->physicalDevices = culkanMalloc(culkan->physicalDeviceCount * sizeof(VkPhysicalDevice));

	culkan->result.vkResult = vkEnumeratePhysicalDevices(culkan->instance, &culkan->physicalDeviceCount, culkan->physicalDevices);
	culkanCheckError(culkan);

	culkan->physicalDevice = culkan->physicalDevices[0];

	vkGetPhysicalDeviceProperties(culkan->physicalDevice, &culkan->deviceProperties);

	// Check for memory
	if (culkan->deviceProperties.limits.maxComputeWorkGroupInvocations < culkan->invocations.x * culkan->invocations.y * culkan->invocations.z) {
		culkan->result.ckResult = TOO_MANY_INVOCATIONS;
		char message[100];
		sprintf(message, "Max invocations: %d, requested invocations: %d", culkan->deviceProperties.limits.maxComputeWorkGroupInvocations,
				culkan->invocations.x * culkan->invocations.y * culkan->invocations.z);
		culkanCheckErrorWithMessage(culkan, message);
	}

	vkGetPhysicalDeviceQueueFamilyProperties(culkan->physicalDevice, &culkan->queueFamilyCount, NULL);
	culkan->queueFamilies = culkanMalloc(culkan->queueFamilyCount * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(culkan->physicalDevice, &culkan->queueFamilyCount, culkan->queueFamilies);

	uint32_t family;
	for (family = 0; !(culkan->queueFamilies[family].queueFlags & VK_QUEUE_COMPUTE_BIT) && family < culkan->queueFamilyCount; family++) {}
	if (family == culkan->queueFamilyCount) {
		printf("No compute queue family found\n");
		exit(1);
	}

	culkan->queuePriorities = (float[]){1.0f};
	const VkDeviceQueueCreateInfo queueCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.queueFamilyIndex = family,
		.queueCount = 1,
		.pQueuePriorities = culkan->queuePriorities,
	};

	culkan->deviceCreateInfo = (VkDeviceCreateInfo){
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCreateInfo,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = NULL,
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = NULL,
		.pEnabledFeatures = NULL,
	};

	culkan->result.vkResult = vkCreateDevice(culkan->physicalDevice, &culkan->deviceCreateInfo, NULL, &culkan->device);
	culkanCheckError(culkan);

	vkGetPhysicalDeviceMemoryProperties(culkan->physicalDevice, &culkan->memoryProperties);
	culkan->variables = culkanMalloc(culkan->layout->bindingCount * sizeof(GPUVariable));
	for (int binding_idx = 0; binding_idx < culkan->layout->bindingCount; binding_idx++) {
		culkan->variables[binding_idx] =
			*createGPUVariable(culkan->layout->bindings[binding_idx].size, culkan->layout->bindings[binding_idx].type, binding_idx, family,
							   culkan->device, &culkan->memoryProperties, &culkan->result);
	}
	culkanCheckError(culkan);

	culkan->descriptorWritesVar = culkanMalloc(culkan->layout->bindingCount * sizeof(VkWriteDescriptorSet*));
	// Unitialised pointers

	// Check for memory (heap size requested vs heap size available)
	// TODO : i think it doesn't work lol
	uint32_t heap_idx;
	for (heap_idx = 0; heap_idx < culkan->memoryProperties.memoryHeapCount; heap_idx++) {
		int is_heap_big_enough = 1;
		for (int binding_idx = 0; binding_idx < culkan->layout->bindingCount; binding_idx++) {
			if (culkan->memoryProperties.memoryHeaps[heap_idx].size < culkan->variables[binding_idx].memoryRequirementsVar.size) {
				is_heap_big_enough = 0;
				break;
			}
		}
		if (!is_heap_big_enough) {
			culkan->result.ckResult = NOT_ENOUGH_MEMORY;
			char message[256];
			sprintf(message, "Heap %d is not big enough : Requested %lu, available %lu", heap_idx,
					culkan->variables[heap_idx].memoryRequirementsVar.size, culkan->memoryProperties.memoryHeaps[heap_idx].size);
		}
	}

	return culkan;
}

void culkanSetup(Culkan* culkan) {

	VkDescriptorSetLayoutBinding* layoutBindings = culkanMalloc(culkan->layout->bindingCount * sizeof(VkDescriptorSetLayoutBinding));

	for (int i = 0; i < culkan->layout->bindingCount; i++) {
		layoutBindings[i] = (VkDescriptorSetLayoutBinding){
			.binding = i,
			.descriptorType = culkan->layout->bindings[i].type,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
			.pImmutableSamplers = NULL,
		};
	}

	culkan->descriptorSetLayoutCreateInfo = (VkDescriptorSetLayoutCreateInfo){
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.bindingCount = culkan->layout->bindingCount,
		.pBindings = layoutBindings,
	};

	culkan->result.vkResult = vkCreateDescriptorSetLayout(culkan->device, &culkan->descriptorSetLayoutCreateInfo, NULL, &culkan->descriptorSetLayout);
	culkanCheckError(culkan);

	culkan->poolSizeInfo = createDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);

	culkan->pPoolSizes = culkanMalloc(culkan->layout->bindingCount * sizeof(VkDescriptorPoolSize));
	for (int i = 0; i < culkan->layout->bindingCount; i++) {
		culkan->pPoolSizes[i] = (VkDescriptorPoolSize){
			.type = culkan->layout->bindings[i].type,
			.descriptorCount = 1,
		};
	}

	culkan->descriptorPoolCreateInfo = (VkDescriptorPoolCreateInfo){
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.maxSets = 1,
		.poolSizeCount = culkan->layout->bindingCount,
		.pPoolSizes = culkan->pPoolSizes,
	};

	culkan->result.vkResult = vkCreateDescriptorPool(culkan->device, &culkan->descriptorPoolCreateInfo, NULL, &culkan->descriptorPool);
	culkanCheckError(culkan);

	culkan->descriptorSetAllocateInfo = (VkDescriptorSetAllocateInfo){
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = NULL,
		.descriptorPool = culkan->descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &culkan->descriptorSetLayout,
	};

	culkan->result.vkResult = vkAllocateDescriptorSets(culkan->device, &culkan->descriptorSetAllocateInfo, &culkan->descriptorSet);
	culkanCheckError(culkan);

	for (int i = 0; i < culkan->layout->bindingCount; i++) {
		culkan->descriptorWritesVar[i] = createDescriptorSetWrite(culkan->descriptorSet, culkan->variables[i].bufferInfoVar, i);
		vkUpdateDescriptorSets(culkan->device, 1, culkan->descriptorWritesVar[i], 0, NULL);
	}

	culkan->pipelineLayoutCreateInfo = (VkPipelineLayoutCreateInfo){
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.setLayoutCount = 1,
		.pSetLayouts = &culkan->descriptorSetLayout,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = NULL,
	};

	culkan->result.vkResult = vkCreatePipelineLayout(culkan->device, &culkan->pipelineLayoutCreateInfo, NULL, &culkan->pipelineLayout);
	culkanCheckError(culkan);

	culkan->shaderBuffer = culkanOpenShader(culkan->shaderPath, &culkan->fileSize, culkan);

	culkan->shaderModuleCreateInfo = (VkShaderModuleCreateInfo){
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.codeSize = culkan->fileSize,
		.pCode = culkan->shaderBuffer,
	};

	culkan->result.vkResult = vkCreateShaderModule(culkan->device, &culkan->shaderModuleCreateInfo, NULL, &culkan->shaderModule);
	culkanCheckError(culkan);

	culkan->stageCreateInfo = (VkPipelineShaderStageCreateInfo){
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stage = VK_SHADER_STAGE_COMPUTE_BIT,
		.module = culkan->shaderModule,
		.pName = "main",
		.pSpecializationInfo = NULL,
	};

	culkan->pipelineCreateInfo = (VkComputePipelineCreateInfo){
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stage = culkan->stageCreateInfo,
		.layout = culkan->pipelineLayout,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = 0,
	};

	culkan->result.vkResult = vkCreateComputePipelines(culkan->device, VK_NULL_HANDLE, 1, &culkan->pipelineCreateInfo, NULL, &culkan->pipeline);
	culkanCheckError(culkan);

	culkan->commandPoolCreateInfo = (VkCommandPoolCreateInfo){
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.queueFamilyIndex = culkan->family,
	};

	culkan->result.vkResult = vkCreateCommandPool(culkan->device, &culkan->commandPoolCreateInfo, NULL, &culkan->commandPool);
	culkanCheckError(culkan);

	culkan->commandBufferAllocateInfo = (VkCommandBufferAllocateInfo){
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = culkan->commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	culkan->result.vkResult = vkAllocateCommandBuffers(culkan->device, &culkan->commandBufferAllocateInfo, &culkan->commandBuffer);
	culkanCheckError(culkan);

	culkan->commandBufferBeginInfo = (VkCommandBufferBeginInfo){
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = 0,
		.pInheritanceInfo = NULL,
	};

	vkBeginCommandBuffer(culkan->commandBuffer, &culkan->commandBufferBeginInfo);
	vkCmdBindPipeline(culkan->commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, culkan->pipeline);
	vkCmdBindDescriptorSets(culkan->commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, culkan->pipelineLayout, 0, 1, &culkan->descriptorSet, 0, NULL);
	vkCmdDispatch(culkan->commandBuffer, 1, 1, 1);
	vkEndCommandBuffer(culkan->commandBuffer);

	culkan->fenceCreateInfo = (VkFenceCreateInfo){
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
	};

	culkan->result.vkResult = vkCreateFence(culkan->device, &culkan->fenceCreateInfo, NULL, &culkan->computeFence);
	culkanCheckError(culkan);

	vkGetDeviceQueue(culkan->device, culkan->family, 0, &culkan->queue);
}

// TODO : Be able to choose the number of workgroups
void culkanRun(Culkan* culkan) {
	culkan->commandBuffers = culkanMalloc(sizeof(VkCommandBuffer));
	culkan->commandBuffers[0] = culkan->commandBuffer;

	vkQueueSubmit(culkan->queue, 1,
				  &(VkSubmitInfo){
					  .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
					  .pNext = NULL,
					  .waitSemaphoreCount = 0,
					  .pWaitSemaphores = NULL,
					  .pWaitDstStageMask = NULL,
					  .commandBufferCount = 1,
					  .pCommandBuffers = culkan->commandBuffers,
					  .signalSemaphoreCount = 0,
					  .pSignalSemaphores = NULL,
				  },
				  culkan->computeFence);
	vkWaitForFences(culkan->device, 1, &culkan->computeFence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(culkan->device, culkan->computeFence, NULL);
}

// Allocate all the memory needed for the variables and bindings
void culkanAlloc(Culkan* culkan) {
	culkan->variables = malloc(culkan->layout->bindingCount * sizeof(GPUVariable));
	for (int i = 0; i < culkan->layout->bindingCount; i++) {
		culkan->variables[i] = *createGPUVariable(culkan->layout->bindings[i].size, culkan->layout->bindings[i].type, i, culkan->family,
												  culkan->device, &culkan->memoryProperties, &culkan->result);
	}
	culkanCheckError(culkan);
}

void culkanDestroy(Culkan* culkan) {
	free(culkan->shaderBuffer);
	vkDestroyShaderModule(culkan->device, culkan->shaderModule, NULL);
	vkDestroyPipeline(culkan->device, culkan->pipeline, NULL);
	vkDestroyPipelineLayout(culkan->device, culkan->pipelineLayout, NULL);
	vkDestroyDescriptorPool(culkan->device, culkan->descriptorPool, NULL);
	vkDestroyDescriptorSetLayout(culkan->device, culkan->descriptorSetLayout, NULL);
	vkDestroyDevice(culkan->device, NULL);
	vkDestroyInstance(culkan->instance, NULL);
	free(culkan->physicalDevices);
	free(culkan->queueFamilies);

	for (int i = 0; i < culkan->layout->bindingCount; i++) {
		freeGPUVariableData(&culkan->variables[i]);
	}

	free(culkan->variables);
	free(culkan);
}