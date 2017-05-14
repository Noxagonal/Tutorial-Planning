/* -----------------------------------------------------
This source code is public domain ( CC0 )
The code is provided as-is without limitations, requirements and responsibilities.
Creators and contributors to this source code are provided as a token of appreciation
and no one associated with this source code can be held responsible for any possible
damages or losses of any kind.

Original file creator:  Niko Kauppi (Code maintenance)
Contributors:
----------------------------------------------------- */

#include "BUILD_OPTIONS.h"
#include "Platform.h"

#include "Renderer.h"
#include "Shared.h"
#include "Window.h"
#include "Pipeline.h"

#include <cstdlib>
#include <assert.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <memory>
#include <map>

Renderer::Renderer()
{
	InitPlatform();
	_SetupLayersAndExtensions();
	_SetupDebug();
	_InitInstance();
	_InitDebug();
	_InitDevice();
	_InitDescriptorSetLayouts();
	_InitCameraPipelineLayout();
}

Renderer::~Renderer()
{
	delete _window;

	_DeInitDescriptorPools();
	_DeInitCameraPipelineLayout();
	_DeInitDescriptorSetLayouts();
	_DeInitDevice();
	_DeInitDebug();
	_DeInitInstance();
	DeInitPlatform();
}

Window * Renderer::OpenWindow( uint32_t size_x, uint32_t size_y, std::string name )
{
	_window		= new Window( this, size_x, size_y, name );
	return		_window;
}

bool Renderer::Run()
{
	if( nullptr != _window ) {
		return _window->Update();
	}
	return true;
}

const VkInstance Renderer::GetVulkanInstance() const
{
	return _instance;
}

const VkPhysicalDevice Renderer::GetVulkanPhysicalDevice() const
{
	return _gpu;
}

const VkDevice Renderer::GetVulkanDevice() const
{
	return _device;
}

const VkQueue Renderer::GetVulkanQueue() const
{
	return _queue;
}

const uint32_t Renderer::GetVulkanGraphicsQueueFamilyIndex() const
{
	return _graphics_family_index;
}

const VkPhysicalDeviceFeatures & Renderer::GetVulkanPhysicalDeviceFeatures() const
{
	return _gpu_features;
}

const VkPhysicalDeviceProperties & Renderer::GetVulkanPhysicalDeviceProperties() const
{
	return _gpu_properties;
}

const VkPhysicalDeviceMemoryProperties & Renderer::GetVulkanPhysicalDeviceMemoryProperties() const
{
	return _gpu_memory_properties;
}

const VkPipelineLayout Renderer::GetVulkanCameraPipelineLayout() const
{
	return _camera_pipeline_layout;
}

const VkDescriptorSetLayout Renderer::GetVulkanCameraDescriptorSetLayout() const
{
	return _camera_descriptor_set_layout;
}

const VkDescriptorSetLayout Renderer::GetVulkanObjectDescriptorSetLayout() const
{
	return _object_descriptor_set_layout;
}

const VkDescriptorSetLayout Renderer::GetVulkanSurfacePlainDescriptorSetLayout() const
{
	return _material_plain_descriptor_set;
}

VkDescriptorSet Renderer::AllocateDescriptorSet( DESCRIPTOR_SET_TYPE descriptor_set_type )
{
	switch( descriptor_set_type ) {
	case DESCRIPTOR_SET_TYPE::CAMERA:
		return _AllocateDescriptorSet( _camera_descriptor_set_layout );
		break;
	case DESCRIPTOR_SET_TYPE::SCENE_OBJECT:
		return _AllocateDescriptorSet( _object_descriptor_set_layout );
		break;
	case DESCRIPTOR_SET_TYPE::MATERIAL_PLAIN:
		return _AllocateDescriptorSet( _material_plain_descriptor_set );
		break;
	default:
		assert( 0 && "Undefined descriptor set type." );
		break;
	}
	return VK_NULL_HANDLE;
}

void Renderer::FreeDescriptorSet( VkDescriptorSet set )
{
	_FreeDescriptorSet( set );
}

void Renderer::_SetupLayersAndExtensions()
{
	_instance_extensions.push_back( VK_KHR_SURFACE_EXTENSION_NAME );
	AddRequiredPlatformInstanceExtensions( &_instance_extensions );

	_device_extensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
}

void Renderer::_InitInstance()
{
	VkApplicationInfo application_info {};
	application_info.sType							= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.apiVersion						= VK_MAKE_VERSION( 1, 0, 2 );			// 1.0.2 should work on all vulkan enabled drivers.
	application_info.applicationVersion				= VK_MAKE_VERSION( 0, 1, 0 );
	application_info.pApplicationName				= "Vulkan API Tutorial Series";

	VkInstanceCreateInfo instance_create_info {};
	instance_create_info.sType						= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pApplicationInfo			= &application_info;
	instance_create_info.enabledLayerCount			= _instance_layers.size();
	instance_create_info.ppEnabledLayerNames		= _instance_layers.data();
	instance_create_info.enabledExtensionCount		= _instance_extensions.size();
	instance_create_info.ppEnabledExtensionNames	= _instance_extensions.data();
	instance_create_info.pNext						= &_debug_callback_create_info;

	ErrorCheck( vkCreateInstance( &instance_create_info, nullptr, &_instance ) );
}

void Renderer::_DeInitInstance()
{
	vkDestroyInstance( _instance, nullptr );
	_instance = nullptr;
}

void Renderer::_InitDevice()
{
	{
		uint32_t gpu_count = 0;
		vkEnumeratePhysicalDevices( _instance, &gpu_count, nullptr );
		std::vector<VkPhysicalDevice> gpu_list( gpu_count );
		vkEnumeratePhysicalDevices( _instance, &gpu_count, gpu_list.data() );
		_gpu = gpu_list[ 0 ];
		vkGetPhysicalDeviceFeatures( _gpu, &_gpu_features );
		vkGetPhysicalDeviceProperties( _gpu, &_gpu_properties );
		vkGetPhysicalDeviceMemoryProperties( _gpu, &_gpu_memory_properties );
	}
	{
		uint32_t family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( _gpu, &family_count, nullptr );
		std::vector<VkQueueFamilyProperties> family_property_list( family_count );
		vkGetPhysicalDeviceQueueFamilyProperties( _gpu, &family_count, family_property_list.data() );

		bool found = false;
		for( uint32_t i=0; i < family_count; ++i ) {
			if( family_property_list[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
				found = true;
				_graphics_family_index = i;
				break;
			}
		}
		if( !found ) {
			assert( 0 && "Vulkan ERROR: Queue family supporting graphics not found." );
			std::exit( -1 );
		}
	}

	float queue_priorities[] { 1.0f };
	VkDeviceQueueCreateInfo device_queue_create_info {};
	device_queue_create_info.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	device_queue_create_info.queueFamilyIndex	= _graphics_family_index;
	device_queue_create_info.queueCount			= 1;
	device_queue_create_info.pQueuePriorities	= queue_priorities;

	VkPhysicalDeviceFeatures enabled_features {};
	enabled_features.fillModeNonSolid			= VK_TRUE;
	enabled_features.samplerAnisotropy			= VK_TRUE;

	VkDeviceCreateInfo device_create_info {};
	device_create_info.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount		= 1;
	device_create_info.pQueueCreateInfos		= &device_queue_create_info;
//	device_create_info.enabledLayerCount		= _device_layers.dimensions_size();			// depricated
//	device_create_info.ppEnabledLayerNames		= _device_layers.data();					// depricated
	device_create_info.enabledExtensionCount	= _device_extensions.size();
	device_create_info.ppEnabledExtensionNames	= _device_extensions.data();
	device_create_info.pEnabledFeatures			= &enabled_features;

	ErrorCheck( vkCreateDevice( _gpu, &device_create_info, nullptr, &_device ) );

	vkGetDeviceQueue( _device, _graphics_family_index, 0, &_queue );
}

void Renderer::_DeInitDevice()
{
	vkDestroyDevice( _device, nullptr );
	_device = nullptr;
}

void Renderer::_InitCameraPipelineLayout()
{
	VkPipelineLayoutCreateInfo create_info {};
	create_info.sType			= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	create_info.pNext			= nullptr;
	create_info.flags			= 0;
	create_info.setLayoutCount	= 1;
	create_info.pSetLayouts		= &_camera_descriptor_set_layout;
	create_info.pushConstantRangeCount	= 0;
	create_info.pPushConstantRanges		= nullptr;
	ErrorCheck( vkCreatePipelineLayout( _device, &create_info, nullptr, &_camera_pipeline_layout ) );
}

void Renderer::_DeInitCameraPipelineLayout()
{
	vkDestroyPipelineLayout( _device, _camera_pipeline_layout, nullptr );
}

void Renderer::_InitDescriptorSetLayouts()
{
	// Camera Descriptor Set
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings( 1 );
		bindings[ 0 ].binding				= 0;
		bindings[ 0 ].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[ 0 ].descriptorCount		= 1;
		bindings[ 0 ].stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
		descriptor_set_layout_create_info.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.flags				= 0;
		descriptor_set_layout_create_info.bindingCount		= uint32_t( bindings.size() );
		descriptor_set_layout_create_info.pBindings			= bindings.data();
		vkCreateDescriptorSetLayout( _device, &descriptor_set_layout_create_info, nullptr, &_camera_descriptor_set_layout );
	}
	// Object Descriptor Set
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings( 1 );
		bindings[ 0 ].binding				= 0;
		bindings[ 0 ].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[ 0 ].descriptorCount		= 1;
		bindings[ 0 ].stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
		descriptor_set_layout_create_info.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.flags				= 0;
		descriptor_set_layout_create_info.bindingCount		= uint32_t( bindings.size() );
		descriptor_set_layout_create_info.pBindings			= bindings.data();
		vkCreateDescriptorSetLayout( _device, &descriptor_set_layout_create_info, nullptr, &_object_descriptor_set_layout );
	}
	// Material Descriptor Sets: Plain
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings( 1 );

		bindings[ 0 ].binding				= 0;
		bindings[ 0 ].descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[ 0 ].descriptorCount		= 1;
		bindings[ 0 ].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
		descriptor_set_layout_create_info.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_set_layout_create_info.flags				= 0;
		descriptor_set_layout_create_info.bindingCount		= uint32_t( bindings.size() );
		descriptor_set_layout_create_info.pBindings			= bindings.data();
		vkCreateDescriptorSetLayout( _device, &descriptor_set_layout_create_info, nullptr, &_material_plain_descriptor_set );
	}
}

void Renderer::_DeInitDescriptorSetLayouts()
{
	vkDestroyDescriptorSetLayout( _device, _camera_descriptor_set_layout, nullptr );
	vkDestroyDescriptorSetLayout( _device, _object_descriptor_set_layout, nullptr );
	vkDestroyDescriptorSetLayout( _device, _material_plain_descriptor_set, nullptr );
}

void Renderer::_DeInitDescriptorPools()
{
	for( auto pool : _descriptor_pools ) {
		vkDestroyDescriptorPool( _device, pool, nullptr );
	}
	_descriptor_pools.clear();
}

std::map<VkDescriptorSet, VkDescriptorPool>		set_pool_map;
std::map<VkDescriptorPool, uint32_t>			pool_set_count_map;
constexpr uint32_t								DESCRIPTOR_POOL_MAX_SETS		= 50;

VkDescriptorSet Renderer::_AllocateDescriptorSet( VkDescriptorSetLayout descriptor_set_layout )
{
	for( auto pool : _descriptor_pools ) {
		if( pool_set_count_map[ pool ] < DESCRIPTOR_POOL_MAX_SETS ) {
			VkDescriptorSet set = VK_NULL_HANDLE;
			VkDescriptorSetAllocateInfo allocate_info {};
			allocate_info.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocate_info.pNext					= nullptr;
			allocate_info.descriptorPool		= pool;
			allocate_info.descriptorSetCount	= 1;
			allocate_info.pSetLayouts			= &descriptor_set_layout;
			auto result = vkAllocateDescriptorSets( _device, &allocate_info, &set );
			if( result == VK_SUCCESS ) {
				pool_set_count_map[ pool ]++;
				set_pool_map.insert( std::pair<VkDescriptorSet, VkDescriptorPool>( set, pool ) );
				return set;
			}
		}
	}
	// not found, create a new pool
	{
		std::vector<VkDescriptorPoolSize> pool_sizes;
		pool_sizes.push_back( { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 } );
		pool_sizes.push_back( { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 } );
		pool_sizes.push_back( { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 } );

		VkDescriptorPool pool = VK_NULL_HANDLE;
		VkDescriptorPoolCreateInfo descriptor_pool_create_info {};
		descriptor_pool_create_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_create_info.flags			= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		descriptor_pool_create_info.maxSets			= DESCRIPTOR_POOL_MAX_SETS;
		descriptor_pool_create_info.poolSizeCount	= uint32_t( pool_sizes.size() );
		descriptor_pool_create_info.pPoolSizes		= pool_sizes.data();
		ErrorCheck( vkCreateDescriptorPool( _device, &descriptor_pool_create_info, nullptr, &pool ) );
		_descriptor_pools.push_back( pool );

		VkDescriptorSet set = VK_NULL_HANDLE;
		VkDescriptorSetAllocateInfo allocate_info {};
		allocate_info.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocate_info.pNext					= nullptr;
		allocate_info.descriptorPool		= pool;
		allocate_info.descriptorSetCount	= 1;
		allocate_info.pSetLayouts			= &descriptor_set_layout;
		ErrorCheck( vkAllocateDescriptorSets( _device, &allocate_info, &set ) );
		pool_set_count_map.insert( std::pair<VkDescriptorPool, uint32_t>( pool, 1 ) );
		set_pool_map.insert( std::pair<VkDescriptorSet, VkDescriptorPool>( set, pool ) );
		return set;
	}
}

void Renderer::_FreeDescriptorSet( VkDescriptorSet set )
{
	auto pool = set_pool_map[ set ];
	vkFreeDescriptorSets( _device, pool, 1, &set );
	set_pool_map.erase( set );

	pool_set_count_map[ pool ]--;
	if( pool_set_count_map[ pool ] == 0 ) {
		pool_set_count_map.erase( pool );
		vkDestroyDescriptorPool( _device, pool, nullptr );
		_descriptor_pools.remove( pool );
	}
}

#if BUILD_ENABLE_VULKAN_DEBUG

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(
	VkDebugReportFlagsEXT		flags,
	VkDebugReportObjectTypeEXT	obj_type,
	uint64_t					src_obj,
	size_t						location,
	int32_t						msg_code,
	const char *				layer_prefix,
	const char *				msg,
	void *						user_data
	)
{
	std::ostringstream stream;
	stream << "VKDBG: ";
	if( flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT ) {
		stream << "INFO: ";
	}
	if( flags & VK_DEBUG_REPORT_WARNING_BIT_EXT ) {
		stream << "WARNING: ";
	}
	if( flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT ) {
		stream << "PERFORMANCE: ";
	}
	if( flags & VK_DEBUG_REPORT_ERROR_BIT_EXT ) {
		stream << "ERROR: ";
	}
	if( flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT ) {
		stream << "DEBUG: ";
	}
	stream << "@[" << layer_prefix << "]: ";
	stream << msg << std::endl;
	std::cout << stream.str();

#if defined( _WIN32 )
	if( flags & VK_DEBUG_REPORT_ERROR_BIT_EXT ) {
		MessageBox( NULL, stream.str().c_str(), "Vulkan Error!", 0 );
	}
#endif

	return false;
}

void Renderer::_SetupDebug()
{
	_debug_callback_create_info.sType			= VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	_debug_callback_create_info.pfnCallback		= VulkanDebugCallback;
	_debug_callback_create_info.flags			=
//		VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
//		VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		0;

	_instance_layers.push_back( "VK_LAYER_LUNARG_standard_validation" );
	/*
//	_instance_layers.push_back( "VK_LAYER_LUNARG_threading" );
	_instance_layers.push_back( "VK_LAYER_GOOGLE_threading" );
	_instance_layers.push_back( "VK_LAYER_LUNARG_draw_state" );
	_instance_layers.push_back( "VK_LAYER_LUNARG_image" );
	_instance_layers.push_back( "VK_LAYER_LUNARG_mem_tracker" );
	_instance_layers.push_back( "VK_LAYER_LUNARG_object_tracker" );
	_instance_layers.push_back( "VK_LAYER_LUNARG_param_checker" );
	*/
	_instance_extensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );

//	_device_layers.push_back( "VK_LAYER_LUNARG_standard_validation" );			// depricated
	/*
//	_device_layers.push_back( "VK_LAYER_LUNARG_threading" );
	_device_layers.push_back( "VK_LAYER_GOOGLE_threading" );
	_device_layers.push_back( "VK_LAYER_LUNARG_draw_state" );
	_device_layers.push_back( "VK_LAYER_LUNARG_image" );
	_device_layers.push_back( "VK_LAYER_LUNARG_mem_tracker" );
	_device_layers.push_back( "VK_LAYER_LUNARG_object_tracker" );
	_device_layers.push_back( "VK_LAYER_LUNARG_param_checker" );
	*/
}

PFN_vkCreateDebugReportCallbackEXT		fvkCreateDebugReportCallbackEXT		= nullptr;
PFN_vkDestroyDebugReportCallbackEXT		fvkDestroyDebugReportCallbackEXT	= nullptr;

void Renderer::_InitDebug()
{
	fvkCreateDebugReportCallbackEXT		= (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr( _instance, "vkCreateDebugReportCallbackEXT" );
	fvkDestroyDebugReportCallbackEXT	= (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr( _instance, "vkDestroyDebugReportCallbackEXT" );
	if( nullptr == fvkCreateDebugReportCallbackEXT || nullptr == fvkDestroyDebugReportCallbackEXT ) {
		assert( 0 && "Vulkan ERROR: Can't fetch debug function pointers." );
		std::exit( -1 );
	}

	fvkCreateDebugReportCallbackEXT( _instance, &_debug_callback_create_info, nullptr, &_debug_report );

//	vkCreateDebugReportCallbackEXT( _instance, nullptr, nullptr, nullptr );
}

void Renderer::_DeInitDebug()
{
	fvkDestroyDebugReportCallbackEXT( _instance, _debug_report, nullptr );
	_debug_report = VK_NULL_HANDLE;
}

#else

void Renderer::_SetupDebug() {};
void Renderer::_InitDebug() {};
void Renderer::_DeInitDebug() {};

#endif // BUILD_ENABLE_VULKAN_DEBUG
