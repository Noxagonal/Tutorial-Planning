#pragma once

#include "Platform.h"
#include "SceneObject.h"
#include "Mesh.h"

#include <memory>

class Renderer;
class Surface;
class GraphicsPipeline;
class Mesh;
class Texture;

struct SO_DescriptorSetInfo_DynamicObject
{
	VkBuffer				ubo						= VK_NULL_HANDLE;
	VkDeviceMemory			ubo_memory				= VK_NULL_HANDLE;
	VkDescriptorSet			descriptor_set			= VK_NULL_HANDLE;
};

class SceneObject_DynamicObject : public SceneObject
{
public:
	SceneObject_DynamicObject( Renderer * renderer, Surface * object_material, MESH_OBJECT_SHAPE default_shape = MESH_OBJECT_SHAPE::NONE );
	SceneObject_DynamicObject( Renderer * renderer, Surface * object_material, std::string path );
	~SceneObject_DynamicObject();

	void						UpdateLogic();
	void						CmdRender( VkCommandBuffer command_buffer );

	void						SetSurface( Surface * surface );

private:
	VkBuffer					_Get_ObjectUBO();
	void						_Update_ObjectUBO();

	void						_UpdateDescriptorSet_ObjectUBO();
	void						_CmdBindDescriptorSet_ObjectUBO( VkCommandBuffer command_buffer );

	void						_InitMeshBuffers();
	void						_DeInitMeshBuffers();

	void						_Allocate_ObjectUBO();
	void						_DeAllocate_ObjectUBO();

	Surface				*	_ref_material								= nullptr;

	VkBuffer					_vbo										= VK_NULL_HANDLE;
	VkBuffer					_ibo										= VK_NULL_HANDLE;

	VkDeviceMemory				_vbo_memory									= VK_NULL_HANDLE;
	VkDeviceMemory				_ibo_memory									= VK_NULL_HANDLE;

	SO_DescriptorSetInfo_DynamicObject	_descriptor_set_info;
public:
	std::unique_ptr<Mesh>		_mesh;
};
