#include "SceneObject_Camera.h"

#include "Platform.h"
#include "Shared.h"
#include "Renderer.h"
#include "Pipeline.h"
#include "Surface.h"

SceneObject_Camera::SceneObject_Camera( Renderer * renderer )
	: SceneObject( renderer )
{
	_descriptor_set			= _ref_renderer->AllocateDescriptorSet( DESCRIPTOR_SET_TYPE::CAMERA );

	_InitCameraShaderDataBuffer();
}


SceneObject_Camera::~SceneObject_Camera()
{
	_DeInitCameraShaderDataBuffer();
	_ref_renderer->FreeDescriptorSet( _descriptor_set );
}

void SceneObject_Camera::UpdateLogic()
{
}

void SceneObject_Camera::CmdRender( VkCommandBuffer command_buffer )
{
}

glm::mat4 SceneObject_Camera::CalculateViewMatrix()
{
	return glm::inverse( CalculateTransformationMatrix() );
}

glm::mat4 SceneObject_Camera::CalculateProjectionMatrix( float fov_angle, VkExtent2D viewport_size, float near_plane, float far_plane )
{
	auto ret = glm::perspective( glm::radians( fov_angle ), float( viewport_size.width ) / float( viewport_size.height ), near_plane, far_plane );
	return ret;
}

void SceneObject_Camera::CmdUpdateUBOAndBindDescriptorSetsForPipeline(
	VkCommandBuffer command_buffer,
	float fov_angle,
	VkExtent2D viewport_size,
	float near_plane,
	float far_plane )
{
	_Update_CameraUBO( fov_angle, viewport_size, near_plane, far_plane );
	_UpdateDescriptorSet_CameraUBO();
	_CmdBindDescriptorSet_CameraUBO( command_buffer );
}

void SceneObject_Camera::_Update_CameraUBO( float fov_angle, VkExtent2D viewport_size, float near_plane, float far_plane )
{
	UBOData_Camera * data	= nullptr;
	vkMapMemory( _ref_vk_device, _camera_shader_data_buffer_memory, 0, sizeof( UBOData_Camera ), 0, (void**)&data );
	data->Projection_Matrix		= CalculateProjectionMatrix( fov_angle, viewport_size, near_plane, far_plane );
	data->View_Matrix			= CalculateViewMatrix();
	vkUnmapMemory( _ref_vk_device, _camera_shader_data_buffer_memory );
}

VkBuffer SceneObject_Camera::_Get_CameraUBO()
{
	return _camera_shader_data_buffer;
}

void SceneObject_Camera::_UpdateDescriptorSet_CameraUBO()
{
	std::vector<VkWriteDescriptorSet> write_sets( 1 );

	VkDescriptorBufferInfo write_buffer_info {};
	write_buffer_info.buffer			= _camera_shader_data_buffer;
	write_buffer_info.offset			= 0;
	write_buffer_info.range				= sizeof( UBOData_Camera );

	write_sets[ 0 ].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_sets[ 0 ].dstSet				= _descriptor_set;
	write_sets[ 0 ].dstBinding			= 0;
	write_sets[ 0 ].dstArrayElement		= 0;
	write_sets[ 0 ].descriptorCount		= 1;
	write_sets[ 0 ].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write_sets[ 0 ].pImageInfo			= nullptr;
	write_sets[ 0 ].pBufferInfo			= &write_buffer_info;
	write_sets[ 0 ].pTexelBufferView	= nullptr;

	vkUpdateDescriptorSets( _ref_vk_device, uint32_t( write_sets.size() ), write_sets.data(), 0, nullptr );
}

void SceneObject_Camera::_CmdBindDescriptorSet_CameraUBO( VkCommandBuffer command_buffer )
{
	vkCmdBindDescriptorSets( command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		_ref_renderer->GetVulkanCameraPipelineLayout(),
		0,
		1, &_descriptor_set,
		0, nullptr );
}

void SceneObject_Camera::_InitCameraShaderDataBuffer()
{
	VkBufferCreateInfo buffer_create_info {};
	buffer_create_info.sType			= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.flags			= 0;
	buffer_create_info.size				= sizeof( UBOData_Camera );
	buffer_create_info.usage			= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buffer_create_info.sharingMode		= VK_SHARING_MODE_EXCLUSIVE;
	vkCreateBuffer( _ref_vk_device, &buffer_create_info, nullptr, &_camera_shader_data_buffer );

	VkMemoryRequirements memory_requirements {};
	vkGetBufferMemoryRequirements( _ref_vk_device, _camera_shader_data_buffer, &memory_requirements );
	auto memory_index = FindMemoryTypeIndex( &_ref_renderer->GetVulkanPhysicalDeviceMemoryProperties(), &memory_requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

	VkMemoryAllocateInfo memory_allocate_info {};
	memory_allocate_info.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize		= memory_requirements.size;
	memory_allocate_info.memoryTypeIndex	= memory_index;
	vkAllocateMemory( _ref_vk_device, &memory_allocate_info, nullptr, &_camera_shader_data_buffer_memory );
	vkBindBufferMemory( _ref_vk_device, _camera_shader_data_buffer, _camera_shader_data_buffer_memory, 0 );
}

void SceneObject_Camera::_DeInitCameraShaderDataBuffer()
{
	vkDestroyBuffer( _ref_vk_device, _camera_shader_data_buffer, nullptr );
	vkFreeMemory( _ref_vk_device, _camera_shader_data_buffer_memory, nullptr );
}
