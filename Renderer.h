/* -----------------------------------------------------
This source code is public domain ( CC0 )
The code is provided as-is without limitations, requirements and responsibilities.
Creators and contributors to this source code are provided as a token of appreciation
and no one associated with this source code can be held responsible for any possible
damages or losses of any kind.

Original file creator:  Niko Kauppi (Code maintenance)
Contributors:
----------------------------------------------------- */

#pragma once

#include "Platform.h"

#include <list>
#include <vector>
#include <string>
#include <memory>

class Window;
class GraphicsPipeline;

struct MemoryInfo
{
	VkDeviceMemory			memory;
	VkDeviceSize			memory_offset;
};

enum class DESCRIPTOR_SET_TYPE : uint32_t
{
	CAMERA,						// Descriptor Set for Camera UBO

	SCENE_OBJECT,				// Descriptor Set for Object UBO

	MATERIAL_PLAIN,				// Descriptor Set for Plain Material and Shaders UBOs
};

class Renderer
{
public:
	Renderer();
	~Renderer();

	Window									*	OpenWindow( uint32_t size_x, uint32_t size_y, std::string name );

	bool										Run();

	const VkInstance							GetVulkanInstance()	const;
	const VkPhysicalDevice						GetVulkanPhysicalDevice() const;
	const VkDevice								GetVulkanDevice() const;
	const VkQueue								GetVulkanQueue() const;
	const uint32_t								GetVulkanGraphicsQueueFamilyIndex() const;
	const VkPhysicalDeviceFeatures			&	GetVulkanPhysicalDeviceFeatures() const;
	const VkPhysicalDeviceProperties		&	GetVulkanPhysicalDeviceProperties() const;
	const VkPhysicalDeviceMemoryProperties	&	GetVulkanPhysicalDeviceMemoryProperties() const;

	const VkPipelineLayout						GetVulkanCameraPipelineLayout() const;
	const VkDescriptorSetLayout					GetVulkanCameraDescriptorSetLayout() const;
	const VkDescriptorSetLayout					GetVulkanObjectDescriptorSetLayout() const;
	const VkDescriptorSetLayout					GetVulkanSurfacePlainDescriptorSetLayout() const;

	MemoryInfo									AllocateBufferMemory( VkBufferUsageFlags usage, VkDeviceSize byte_size );

	VkDescriptorSet								AllocateDescriptorSet( DESCRIPTOR_SET_TYPE descriptor_set_type );
	void										FreeDescriptorSet( VkDescriptorSet set );

private:
	void										_SetupLayersAndExtensions();

	void										_InitInstance();
	void										_DeInitInstance();

	void										_InitDevice();
	void										_DeInitDevice();

	void										_InitCameraPipelineLayout();
	void										_DeInitCameraPipelineLayout();

	void										_InitDescriptorSetLayouts();
	void										_DeInitDescriptorSetLayouts();

	void										_DeInitDescriptorPools();

	VkDescriptorSet								_AllocateDescriptorSet( VkDescriptorSetLayout descriptor_set_layout );
	void										_FreeDescriptorSet( VkDescriptorSet set );

	void										_SetupDebug();
	void										_InitDebug();
	void										_DeInitDebug();

	VkInstance									_instance						= VK_NULL_HANDLE;
	VkPhysicalDevice							_gpu							= VK_NULL_HANDLE;
	VkDevice									_device							= VK_NULL_HANDLE;
	VkQueue										_queue							= VK_NULL_HANDLE;
	VkPhysicalDeviceFeatures					_gpu_features					= {};
	VkPhysicalDeviceProperties					_gpu_properties					= {};
	VkPhysicalDeviceMemoryProperties			_gpu_memory_properties			= {};

	uint32_t									_graphics_family_index			= 0;

	Window									*	_window							= nullptr;

	std::vector<const char*>					_instance_layers;
	std::vector<const char*>					_instance_extensions;
//	std::vector<const char*>					_device_layers;					// depricated
	std::vector<const char*>					_device_extensions;

	VkPipelineLayout							_camera_pipeline_layout			= VK_NULL_HANDLE;

	VkDescriptorSetLayout						_camera_descriptor_set_layout	= VK_NULL_HANDLE;
	VkDescriptorSetLayout						_object_descriptor_set_layout	= VK_NULL_HANDLE;
	VkDescriptorSetLayout						_material_plain_descriptor_set	= VK_NULL_HANDLE;

	std::list<VkDescriptorPool>					_descriptor_pools;

	VkDebugReportCallbackEXT					_debug_report					= VK_NULL_HANDLE;
	VkDebugReportCallbackCreateInfoEXT			_debug_callback_create_info		= {};
};
