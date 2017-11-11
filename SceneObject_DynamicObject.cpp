
#include "SceneObject_DynamicObject.h"

#include "Shared.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Platform.h"
#include "Pipeline.h"
#include "Surface.h"
#include "Texture.h"

SceneObject_DynamicObject::SceneObject_DynamicObject( Renderer * renderer, Surface * object_material, MESH_OBJECT_SHAPE default_shape )
	: SceneObject( renderer )
{
	assert( nullptr != renderer );
	assert( nullptr != object_material );
	_ref_material	= object_material;

	_mesh			= std::unique_ptr<Mesh>( new Mesh );
	_mesh->GenerateShape( default_shape );

	_InitMeshBuffers();
	_Allocate_ObjectUBO();
	_descriptor_set_info.descriptor_set			= _ref_renderer->AllocateDescriptorSet( DESCRIPTOR_SET_TYPE::SCENE_OBJECT );
	_UpdateDescriptorSet_ObjectUBO();
}

SceneObject_DynamicObject::SceneObject_DynamicObject( Renderer * renderer, Surface * object_material, std::string path )
	: SceneObject( renderer )
{
	assert( nullptr != renderer );
	assert( nullptr != object_material );
	_ref_material	= object_material;

	_mesh			= std::unique_ptr<Mesh>( new Mesh );
	_mesh->Load( path );

	_InitMeshBuffers();
	_Allocate_ObjectUBO();
	_descriptor_set_info.descriptor_set			= _ref_renderer->AllocateDescriptorSet( DESCRIPTOR_SET_TYPE::SCENE_OBJECT );
	_UpdateDescriptorSet_ObjectUBO();
}

SceneObject_DynamicObject::~SceneObject_DynamicObject()
{
	_ref_renderer->FreeDescriptorSet( _descriptor_set_info.descriptor_set );
	_DeAllocate_ObjectUBO();
	_DeInitMeshBuffers();
}

void SceneObject_DynamicObject::UpdateLogic()
{
}

void SceneObject_DynamicObject::CmdRender( VkCommandBuffer command_buffer )
{
	// update and bind object shader data
	_Update_ObjectUBO();
//	_UpdateDescriptorSet_ObjectUBO();	// Only needed to do once in our case so this call is moved to a constructor
	_CmdBindDescriptorSet_ObjectUBO( command_buffer );

	// update and bind material shader data
	_ref_material->UpdateDescriptorSets();
	_ref_material->CmdBindDescriptorSets( command_buffer );

	// update vertex buffer
	Vertex * mapped = nullptr;
	ErrorCheck( vkMapMemory( _ref_renderer->GetVulkanDevice(), _vbo_memory, 0, _mesh->GetVerticesByteSize(), 0, (void**)&mapped ) );
	std::memcpy( mapped, _mesh->vertices.data(), _mesh->GetVerticesByteSize() );
	vkUnmapMemory( _ref_renderer->GetVulkanDevice(), _vbo_memory );

	// bind vertex and index buffers, draw
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers( command_buffer, 0, 1, &_vbo, &offset );
	vkCmdBindIndexBuffer( command_buffer, _ibo, 0, VK_INDEX_TYPE_UINT32 );
	vkCmdDrawIndexed( command_buffer, 3 * _mesh->triangles.size(), 1, 0, 0, 0 );
}

void SceneObject_DynamicObject::SetSurface( Surface * material )
{
	_ref_material			= material;
}

VkBuffer SceneObject_DynamicObject::_Get_ObjectUBO()
{
	return _descriptor_set_info.ubo;
}

void SceneObject_DynamicObject::_Update_ObjectUBO()
{
	UBOData_Object * data	= nullptr;
	vkMapMemory( _ref_vk_device, _descriptor_set_info.ubo_memory, 0, sizeof( UBOData_Object ), 0, (void**)&data );
	data->Model_Matrix			= CalculateTransformationMatrix();
	vkUnmapMemory( _ref_vk_device, _descriptor_set_info.ubo_memory );
}

void SceneObject_DynamicObject::_UpdateDescriptorSet_ObjectUBO()
{
	assert( VK_NULL_HANDLE != _descriptor_set_info.ubo );
	assert( VK_NULL_HANDLE != _descriptor_set_info.descriptor_set );

	std::vector<VkWriteDescriptorSet> write_sets( 1 );

	VkDescriptorBufferInfo write_buffer_info {};
	write_buffer_info.buffer			= _descriptor_set_info.ubo;
	write_buffer_info.offset			= 0;
	write_buffer_info.range				= sizeof( UBOData_Object );

	write_sets[ 0 ].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_sets[ 0 ].dstSet				= _descriptor_set_info.descriptor_set;
	write_sets[ 0 ].dstBinding			= 0;
	write_sets[ 0 ].dstArrayElement		= 0;
	write_sets[ 0 ].descriptorCount		= 1;
	write_sets[ 0 ].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write_sets[ 0 ].pImageInfo			= nullptr;
	write_sets[ 0 ].pBufferInfo			= &write_buffer_info;
	write_sets[ 0 ].pTexelBufferView	= nullptr;

	vkUpdateDescriptorSets( _ref_vk_device, uint32_t( write_sets.size() ), write_sets.data(), 0, nullptr );
}

void SceneObject_DynamicObject::_CmdBindDescriptorSet_ObjectUBO( VkCommandBuffer command_buffer )
{
	vkCmdBindDescriptorSets( command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		_ref_material->GetPipeline()->GetVulkanPipelineLayout(),
		1,
		1, &_descriptor_set_info.descriptor_set,
		0, nullptr );
}

void SceneObject_DynamicObject::_InitMeshBuffers()
{
	{
		VkBufferCreateInfo buffer_create_info {};
		buffer_create_info.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.flags					= 0;
		buffer_create_info.size						= _mesh->GetVerticesByteSize();
		buffer_create_info.usage					= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		buffer_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
		vkCreateBuffer( _ref_renderer->GetVulkanDevice(), &buffer_create_info, nullptr, &_vbo );

		VkMemoryRequirements memory_requirements {};
		vkGetBufferMemoryRequirements( _ref_renderer->GetVulkanDevice(), _vbo, &memory_requirements );

		auto id = FindMemoryTypeIndex( &_ref_renderer->GetVulkanPhysicalDeviceMemoryProperties(), &memory_requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

		VkMemoryAllocateInfo memory_allocate_info {};
		memory_allocate_info.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.allocationSize		= memory_requirements.size;
		memory_allocate_info.memoryTypeIndex	= id;
		vkAllocateMemory( _ref_renderer->GetVulkanDevice(), &memory_allocate_info, nullptr, &_vbo_memory );

		Vertex * mapped = nullptr;
		ErrorCheck( vkMapMemory( _ref_renderer->GetVulkanDevice(), _vbo_memory, 0, _mesh->GetVerticesByteSize(), 0, (void**)&mapped ) );
		std::memcpy( mapped, _mesh->vertices.data(), _mesh->GetVerticesByteSize() );
		vkUnmapMemory( _ref_renderer->GetVulkanDevice(), _vbo_memory );

		vkBindBufferMemory( _ref_renderer->GetVulkanDevice(), _vbo, _vbo_memory, 0 );
	}
	{
		VkBufferCreateInfo buffer_create_info {};
		buffer_create_info.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.flags					= 0;
		buffer_create_info.size						= _mesh->GetIndicesByteSize();
		buffer_create_info.usage					= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		buffer_create_info.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
		vkCreateBuffer( _ref_renderer->GetVulkanDevice(), &buffer_create_info, nullptr, &_ibo );

		VkMemoryRequirements memory_requirements {};
		vkGetBufferMemoryRequirements( _ref_renderer->GetVulkanDevice(), _ibo, &memory_requirements );

		auto id = FindMemoryTypeIndex( &_ref_renderer->GetVulkanPhysicalDeviceMemoryProperties(), &memory_requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

		VkMemoryAllocateInfo memory_allocate_info {};
		memory_allocate_info.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.allocationSize		= memory_requirements.size;
		memory_allocate_info.memoryTypeIndex	= id;
		vkAllocateMemory( _ref_renderer->GetVulkanDevice(), &memory_allocate_info, nullptr, &_ibo_memory );

		Triangle * mapped = nullptr;
		ErrorCheck( vkMapMemory( _ref_renderer->GetVulkanDevice(), _ibo_memory, 0, _mesh->GetIndicesByteSize(), 0, (void**)&mapped ) );
		std::memcpy( mapped, _mesh->triangles.data(), _mesh->GetIndicesByteSize() );
		vkUnmapMemory( _ref_renderer->GetVulkanDevice(), _ibo_memory );

		vkBindBufferMemory( _ref_renderer->GetVulkanDevice(), _ibo, _ibo_memory, 0 );
	}
}

void SceneObject_DynamicObject::_DeInitMeshBuffers()
{
	vkDestroyBuffer( _ref_renderer->GetVulkanDevice(), _ibo, nullptr );
	vkDestroyBuffer( _ref_renderer->GetVulkanDevice(), _vbo, nullptr );
	vkFreeMemory( _ref_renderer->GetVulkanDevice(), _ibo_memory, nullptr );
	vkFreeMemory( _ref_renderer->GetVulkanDevice(), _vbo_memory, nullptr );
}

void SceneObject_DynamicObject::_Allocate_ObjectUBO()
{
	VkBufferCreateInfo buffer_create_info {};
	buffer_create_info.sType			= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.flags			= 0;
	buffer_create_info.size				= sizeof( UBOData_Object );
	buffer_create_info.usage			= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buffer_create_info.sharingMode		= VK_SHARING_MODE_EXCLUSIVE;
	vkCreateBuffer( _ref_vk_device, &buffer_create_info, nullptr, &_descriptor_set_info.ubo );

	VkMemoryRequirements memory_requirements {};
	vkGetBufferMemoryRequirements( _ref_vk_device, _descriptor_set_info.ubo, &memory_requirements );
	auto memory_index = FindMemoryTypeIndex( &_ref_renderer->GetVulkanPhysicalDeviceMemoryProperties(), &memory_requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

	VkMemoryAllocateInfo memory_allocate_info {};
	memory_allocate_info.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize		= memory_requirements.size;
	memory_allocate_info.memoryTypeIndex	= memory_index;
	vkAllocateMemory( _ref_vk_device, &memory_allocate_info, nullptr, &_descriptor_set_info.ubo_memory );
	vkBindBufferMemory( _ref_vk_device, _descriptor_set_info.ubo, _descriptor_set_info.ubo_memory, 0 );
}

void SceneObject_DynamicObject::_DeAllocate_ObjectUBO()
{
	vkDestroyBuffer( _ref_vk_device, _descriptor_set_info.ubo, nullptr );
	vkFreeMemory( _ref_vk_device, _descriptor_set_info.ubo_memory, nullptr );
}
