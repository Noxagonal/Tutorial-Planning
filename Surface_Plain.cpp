#include "Surface_Plain.h"

#include "Renderer.h"
#include "Pipeline.h"
#include "Texture.h"

Surface_Plain::Surface_Plain( Renderer * renderer, GraphicsPipeline * pipeline, Texture * texture )
	: Surface( renderer, pipeline )
{
	_ref_texture							= texture;

	_shader_data_info.descriptor_set		= _ref_renderer->AllocateDescriptorSet( DESCRIPTOR_SET_TYPE::MATERIAL_PLAIN );
}


Surface_Plain::~Surface_Plain()
{
	assert( VK_NULL_HANDLE != _shader_data_info.descriptor_set );
	_ref_renderer->FreeDescriptorSet( _shader_data_info.descriptor_set );
}

void Surface_Plain::UpdateDescriptorSets()
{
	assert( VK_NULL_HANDLE != _ref_vk_device );
	assert( VK_NULL_HANDLE != _ref_texture );
	assert( VK_NULL_HANDLE != _shader_data_info.descriptor_set );

	std::vector<VkWriteDescriptorSet> write_sets( 1 );

	VkDescriptorImageInfo image_buffer_info {};
	image_buffer_info.sampler			= _ref_texture->GetVulkanSampler();
	image_buffer_info.imageView			= _ref_texture->GetVulkanImageView();
	image_buffer_info.imageLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	write_sets[ 0 ].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_sets[ 0 ].dstSet				= _shader_data_info.descriptor_set;
	write_sets[ 0 ].dstBinding			= 0;
	write_sets[ 0 ].dstArrayElement		= 0;
	write_sets[ 0 ].descriptorCount		= 1;
	write_sets[ 0 ].descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write_sets[ 0 ].pImageInfo			= &image_buffer_info;
	write_sets[ 0 ].pBufferInfo			= nullptr;
	write_sets[ 0 ].pTexelBufferView	= nullptr;

	vkUpdateDescriptorSets( _ref_vk_device, uint32_t( write_sets.size() ), write_sets.data(), 0, nullptr );
}

void Surface_Plain::CmdBindDescriptorSets( VkCommandBuffer command_buffer )
{
	vkCmdBindDescriptorSets( command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		_ref_pipeline->GetVulkanPipelineLayout(),
		2,
		1, &_shader_data_info.descriptor_set,
		0, nullptr );
}
