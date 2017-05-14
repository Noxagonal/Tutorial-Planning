#pragma once

#include "Platform.h"

class Renderer;
class Window;

class GraphicsPipeline
{
public:
	GraphicsPipeline( Renderer * renderer, Window * window, std::vector<VkDescriptorSetLayout> used_descriptor_set_layouts );
	~GraphicsPipeline();

	VkPipeline				GetVulkanPipeline();
	VkPipelineLayout		GetVulkanPipelineLayout();

private:
	void					_InitPipeline();
	void					_DeInitPipeline();

	void					_InitPipelineLayout();
	void					_DeInitPipelineLayout();

	Renderer			*	_ref_renderer				= nullptr;
	Window				*	_ref_window					= nullptr;
	VkDevice				_ref_vk_device				= VK_NULL_HANDLE;

	VkPipeline				_pipeline					= VK_NULL_HANDLE;
	VkPipelineLayout		_pipeline_layout			= VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayout>					_descriptor_set_layouts;

	VkShaderModule			_vertex_shader_module		= VK_NULL_HANDLE;
	VkShaderModule			_fragment_shader_module		= VK_NULL_HANDLE;
};
