#pragma once

#include "Platform.h"
#include "SceneObject.h"

class GraphicsPipeline;
class Surface;

class SceneObject_Camera : public SceneObject
{
public:
	SceneObject_Camera( Renderer * renderer );
	~SceneObject_Camera();

	void						UpdateLogic();
	void						CmdRender( VkCommandBuffer command_buffer );

	glm::mat4					CalculateViewMatrix();
	glm::mat4					CalculateProjectionMatrix( float fov_angle, VkExtent2D viewport_size, float near_plane, float far_plane );

	void						CmdUpdateUBOAndBindDescriptorSetsForPipeline(
		VkCommandBuffer command_buffer,
		float fov_angle,
		VkExtent2D viewport_size,
		float near_plane,
		float far_plane );

private:
	void						_Update_CameraUBO( float fov_angle, VkExtent2D viewport_size, float near_plane, float far_plane );
	VkBuffer					_Get_CameraUBO();

	void						_UpdateDescriptorSet_CameraUBO();
	void						_CmdBindDescriptorSet_CameraUBO( VkCommandBuffer command_buffer );

	void						_InitCameraShaderDataBuffer();
	void						_DeInitCameraShaderDataBuffer();

	VkDescriptorSet				_descriptor_set;

	VkBuffer					_camera_shader_data_buffer			= VK_NULL_HANDLE;
	VkDeviceMemory				_camera_shader_data_buffer_memory	= VK_NULL_HANDLE;
};
