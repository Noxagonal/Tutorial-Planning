
#include "Platform.h"
#include "Pipeline.h"

#include "Shared.h"
#include "Renderer.h"
#include "Window.h"
#include "Mesh.h"

#include <assert.h>
#include <array>
#include <fstream>

GraphicsPipeline::GraphicsPipeline( Renderer * renderer, Window * window, std::vector<VkDescriptorSetLayout> used_descriptor_set_layouts )
{
	assert( nullptr != renderer );
	assert( nullptr != window );

	// collect references
	_ref_renderer				= renderer;
	_ref_window					= window;
	_ref_vk_device				= _ref_renderer->GetVulkanDevice();

	_descriptor_set_layouts		= used_descriptor_set_layouts;

	_InitPipelineLayout();
	_InitPipeline();
}

GraphicsPipeline::~GraphicsPipeline()
{
	_DeInitPipeline();
	_DeInitPipelineLayout();
}

VkPipeline GraphicsPipeline::GetVulkanPipeline()
{
	return _pipeline;
}

VkPipelineLayout GraphicsPipeline::GetVulkanPipelineLayout()
{
	return _pipeline_layout;
}

void GraphicsPipeline::_InitPipeline()
{
	{
		std::ifstream file( "shaders/default.vert.spv", std::ifstream::binary | std::ifstream::ate );
		assert( file.is_open() );

		size_t file_size = file.tellg();
		std::vector<char> file_data( file_size );
		file.seekg( 0 );
		file.read( file_data.data(), file_size );

		VkShaderModuleCreateInfo shader_module_create_info {};
		shader_module_create_info.sType			= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_create_info.codeSize		= file_data.size();
		shader_module_create_info.pCode			= reinterpret_cast<uint32_t*>( file_data.data() );

		ErrorCheck( vkCreateShaderModule( _ref_renderer->GetVulkanDevice(), &shader_module_create_info, nullptr, &_vertex_shader_module ) );
	}
	{
		std::ifstream file( "shaders/default.frag.spv", std::ifstream::binary | std::ifstream::ate );
		assert( file.is_open() );

		size_t file_size = file.tellg();
		std::vector<char> file_data( file_size );
		file.seekg( 0 );
		file.read( file_data.data(), file_size );

		VkShaderModuleCreateInfo shader_module_create_info {};
		shader_module_create_info.sType			= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_create_info.codeSize		= file_data.size();
		shader_module_create_info.pCode			= reinterpret_cast<uint32_t*>( file_data.data() );

		ErrorCheck( vkCreateShaderModule( _ref_renderer->GetVulkanDevice(), &shader_module_create_info, nullptr, &_fragment_shader_module ) );
	}
	std::array<VkPipelineShaderStageCreateInfo, 2> shader_stage_create_infos {};
	shader_stage_create_infos[ 0 ].sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_create_infos[ 0 ].stage		= VK_SHADER_STAGE_VERTEX_BIT;
	shader_stage_create_infos[ 0 ].module		= _vertex_shader_module;
	shader_stage_create_infos[ 0 ].pName		= "main";

	shader_stage_create_infos[ 1 ].sType		= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_create_infos[ 1 ].stage		= VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stage_create_infos[ 1 ].module		= _fragment_shader_module;
	shader_stage_create_infos[ 1 ].pName		= "main";


	std::vector<VkVertexInputBindingDescription> vertex_input_binding_descriptions( 1 );
	vertex_input_binding_descriptions[ 0 ].binding		= 0;
	vertex_input_binding_descriptions[ 0 ].stride		= sizeof( Vertex );
	vertex_input_binding_descriptions[ 0 ].inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions( 3 );
	vertex_input_attribute_descriptions[ 0 ].location		= 0;
	vertex_input_attribute_descriptions[ 0 ].binding		= 0;
	vertex_input_attribute_descriptions[ 0 ].format			= VK_FORMAT_R32G32B32_SFLOAT;
	vertex_input_attribute_descriptions[ 0 ].offset			= offsetof( Vertex, position );

	vertex_input_attribute_descriptions[ 1 ].location		= 1;
	vertex_input_attribute_descriptions[ 1 ].binding		= 0;
	vertex_input_attribute_descriptions[ 1 ].format			= VK_FORMAT_R32G32B32_SFLOAT;
	vertex_input_attribute_descriptions[ 1 ].offset			= offsetof( Vertex, color );

	vertex_input_attribute_descriptions[ 2 ].location		= 2;
	vertex_input_attribute_descriptions[ 2 ].binding		= 0;
	vertex_input_attribute_descriptions[ 2 ].format			= VK_FORMAT_R32G32_SFLOAT;
	vertex_input_attribute_descriptions[ 2 ].offset			= offsetof( Vertex, uv );

	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info {};
	vertex_input_state_create_info.sType		= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_state_create_info.vertexBindingDescriptionCount	= vertex_input_binding_descriptions.size();
	vertex_input_state_create_info.pVertexBindingDescriptions		= vertex_input_binding_descriptions.data();
	vertex_input_state_create_info.vertexAttributeDescriptionCount	= vertex_input_attribute_descriptions.size();
	vertex_input_state_create_info.pVertexAttributeDescriptions		= vertex_input_attribute_descriptions.data();


	VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info {};
	input_assembly_state_create_info.sType						= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_state_create_info.topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_state_create_info.primitiveRestartEnable		= VK_FALSE;


	std::array<VkViewport, 1> viewports {};
	viewports[ 0 ].x			= 0;
	viewports[ 0 ].y			= 0;
	viewports[ 0 ].width		= float( _ref_window->GetVulkanSurfaceSize().width );
	viewports[ 0 ].height		= float( _ref_window->GetVulkanSurfaceSize().height );
	viewports[ 0 ].minDepth		= 0.0f;
	viewports[ 0 ].maxDepth		= 1.0f;

	std::array<VkRect2D, viewports.size()> scissors {};
	scissors[ 0 ].offset.x		= 0;
	scissors[ 0 ].offset.y		= 0;
	scissors[ 0 ].extent		= _ref_window->GetVulkanSurfaceSize();

	VkPipelineViewportStateCreateInfo viewport_state_create_info {};
	viewport_state_create_info.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_create_info.viewportCount	= viewports.size();
	viewport_state_create_info.pViewports		= viewports.data();
	viewport_state_create_info.scissorCount		= scissors.size();
	viewport_state_create_info.pScissors		= scissors.data();


	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info {};
	rasterization_state_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state_create_info.depthClampEnable		= VK_FALSE;
	rasterization_state_create_info.rasterizerDiscardEnable	= VK_FALSE;
	rasterization_state_create_info.polygonMode				= VK_POLYGON_MODE_FILL;
	rasterization_state_create_info.cullMode				= VK_CULL_MODE_BACK_BIT;
	rasterization_state_create_info.frontFace				= VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization_state_create_info.depthBiasEnable			= VK_FALSE;
	rasterization_state_create_info.depthBiasConstantFactor	= 0.0f;
	rasterization_state_create_info.depthBiasClamp			= 0.0f;
	rasterization_state_create_info.depthBiasSlopeFactor	= 0.0f;
	rasterization_state_create_info.lineWidth				= 1.0f;


	VkPipelineMultisampleStateCreateInfo multisample_state_create_info {};
	multisample_state_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state_create_info.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;
	multisample_state_create_info.sampleShadingEnable	= VK_FALSE;
	multisample_state_create_info.minSampleShading		= 1.0f;
	multisample_state_create_info.pSampleMask			= nullptr;
	multisample_state_create_info.alphaToCoverageEnable	= VK_FALSE;
	multisample_state_create_info.alphaToOneEnable		= VK_FALSE;


	VkStencilOpState stencil_op_state {};
	stencil_op_state.failOp			= VK_STENCIL_OP_KEEP;
	stencil_op_state.passOp			= VK_STENCIL_OP_KEEP;
	stencil_op_state.depthFailOp	= VK_STENCIL_OP_KEEP;
	stencil_op_state.compareOp		= VK_COMPARE_OP_ALWAYS;
	stencil_op_state.compareMask	= 0b00000000;
	stencil_op_state.writeMask		= 0b11111111;
	stencil_op_state.reference		= 0b00000000;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info {};
	depth_stencil_state_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_state_create_info.depthTestEnable			= VK_TRUE;
	depth_stencil_state_create_info.depthWriteEnable		= VK_TRUE;
	depth_stencil_state_create_info.depthCompareOp			= VK_COMPARE_OP_LESS;
	depth_stencil_state_create_info.depthBoundsTestEnable	= VK_FALSE;
	depth_stencil_state_create_info.stencilTestEnable		= VK_FALSE;
	depth_stencil_state_create_info.front					= stencil_op_state;
	depth_stencil_state_create_info.back					= stencil_op_state;
	depth_stencil_state_create_info.minDepthBounds			= 0.0f;
	depth_stencil_state_create_info.maxDepthBounds			= 1.0f;


	std::array<VkPipelineColorBlendAttachmentState, 1> color_blend_attachment_states {};
	color_blend_attachment_states[ 0 ].blendEnable			= VK_FALSE;
	color_blend_attachment_states[ 0 ].srcColorBlendFactor	= VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_states[ 0 ].dstColorBlendFactor	= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_states[ 0 ].colorBlendOp			= VK_BLEND_OP_ADD;
	color_blend_attachment_states[ 0 ].srcAlphaBlendFactor	= VK_BLEND_FACTOR_ONE;
	color_blend_attachment_states[ 0 ].dstAlphaBlendFactor	= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_states[ 0 ].alphaBlendOp			= VK_BLEND_OP_ADD;
	color_blend_attachment_states[ 0 ].colorWriteMask		=
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo color_blend_state_create_info {};
	color_blend_state_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state_create_info.logicOpEnable			= VK_FALSE;
	color_blend_state_create_info.logicOp				= VK_LOGIC_OP_NO_OP;
	color_blend_state_create_info.attachmentCount		= color_blend_attachment_states.size();
	color_blend_state_create_info.pAttachments			= color_blend_attachment_states.data();
	color_blend_state_create_info.blendConstants[ 0 ]	= 1.0f;
	color_blend_state_create_info.blendConstants[ 1 ]	= 1.0f;
	color_blend_state_create_info.blendConstants[ 2 ]	= 1.0f;
	color_blend_state_create_info.blendConstants[ 3 ]	= 1.0f;


	std::array<VkDynamicState, 0> dynamic_states {};

	VkPipelineDynamicStateCreateInfo dynamic_state_create_info {};
	dynamic_state_create_info.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_create_info.dynamicStateCount	= dynamic_states.size();
	dynamic_state_create_info.pDynamicStates	= dynamic_states.data();


	VkGraphicsPipelineCreateInfo pipeline_create_info {};
	pipeline_create_info.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_create_info.flags					= 0;
	pipeline_create_info.stageCount				= shader_stage_create_infos.size();
	pipeline_create_info.pStages				= shader_stage_create_infos.data();
	pipeline_create_info.pVertexInputState		= &vertex_input_state_create_info;
	pipeline_create_info.pInputAssemblyState	= &input_assembly_state_create_info;
	pipeline_create_info.pTessellationState		= nullptr;
	pipeline_create_info.pViewportState			= &viewport_state_create_info;
	pipeline_create_info.pRasterizationState	= &rasterization_state_create_info;
	pipeline_create_info.pMultisampleState		= &multisample_state_create_info;
	pipeline_create_info.pDepthStencilState		= &depth_stencil_state_create_info;
	pipeline_create_info.pColorBlendState		= &color_blend_state_create_info;
	pipeline_create_info.pDynamicState			= dynamic_states.size() ? &dynamic_state_create_info : nullptr;
	pipeline_create_info.layout					= _pipeline_layout;
	pipeline_create_info.renderPass				= _ref_window->GetVulkanRenderPass();
	pipeline_create_info.subpass				= 0;
	pipeline_create_info.basePipelineHandle		= VK_NULL_HANDLE;
	pipeline_create_info.basePipelineIndex		= -1;

	ErrorCheck( vkCreateGraphicsPipelines( _ref_renderer->GetVulkanDevice(), VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &_pipeline ) );
}

void GraphicsPipeline::_DeInitPipeline()
{
	vkDestroyPipeline( _ref_renderer->GetVulkanDevice(), _pipeline, nullptr );
	vkDestroyShaderModule( _ref_renderer->GetVulkanDevice(), _fragment_shader_module, nullptr );
	vkDestroyShaderModule( _ref_renderer->GetVulkanDevice(), _vertex_shader_module, nullptr );
}

void GraphicsPipeline::_InitPipelineLayout()
{
	std::array<VkPushConstantRange, 0> push_constant_ranges {};

	VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
	pipeline_layout_create_info.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.setLayoutCount			= uint32_t( _descriptor_set_layouts.size() );
	pipeline_layout_create_info.pSetLayouts				= _descriptor_set_layouts.data();
	pipeline_layout_create_info.pushConstantRangeCount	= uint32_t( push_constant_ranges.size() );
	pipeline_layout_create_info.pPushConstantRanges		= push_constant_ranges.data();

	ErrorCheck( vkCreatePipelineLayout( _ref_renderer->GetVulkanDevice(), &pipeline_layout_create_info, nullptr, &_pipeline_layout ) );
}

void GraphicsPipeline::_DeInitPipelineLayout()
{
	vkDestroyPipelineLayout( _ref_renderer->GetVulkanDevice(), _pipeline_layout, nullptr );
}
