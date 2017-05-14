/* -----------------------------------------------------
This source code is public domain ( CC0 )
The code is provided as-is without limitations, requirements and responsibilities.
Creators and contributors to this source code are provided as a token of appreciation
and no one associated with this source code can be held responsible for any possible
damages or losses of any kind.

Original file creator:  Niko Kauppi (Code maintenance)
Contributors:
----------------------------------------------------- */

#include <array>
#include <chrono>
#include <iostream>

#include "Shared.h"
#include "Renderer.h"
#include "Window.h"
#include "Pipeline.h"
#include "Surface_Plain.h"
#include "Texture.h"

#include "Scene.h"
#include "SceneObject_Camera.h"
#include "SceneObject_DynamicObject.h"


constexpr double PI				= 3.14159265358979323846;
constexpr double CIRCLE_RAD		= PI * 2;

int main()
{
//	test;

	namespace chrono		= std::chrono;
	auto timer				= chrono::steady_clock();
	auto program_start_time	= timer.now();

	Renderer renderer;

	auto window = renderer.OpenWindow( 1600, 900, "Vulkan API Tutorial series forwards planning project" );

	// textures, can be shared between surfaces
	Texture logo_diff( &renderer, L"textures/Logo.png" );
	Texture dragon_head_diff( &renderer, L"textures/DragonHead_diff.png" );
	Texture monkey_diff( &renderer, L"textures/Monkey_diff.png" );

	// graphics pipelines, can be shared between surfaces
	/*	NOTE:	Graphics pipelines don't create descriptor set layouts, instead we provide already existing ones
				from Renderer object or create new ones ourselves. The amount and types of descriptor sets we use
				determine compatibility with shaders, cameras, objects, surfaces and other objects we may define in the future.
				Usual arrangement for a scene object for now is: Camera, Object, Surface. The shader must comply to this.
	*/
	GraphicsPipeline plain_pipeline( &renderer, window, {
		renderer.GetVulkanCameraDescriptorSetLayout(),
		renderer.GetVulkanObjectDescriptorSetLayout(),
		renderer.GetVulkanSurfacePlainDescriptorSetLayout() } );

	// surfaces, can NOT be shared between objects, (could be called material)
	Surface_Plain logo_surface( &renderer, &plain_pipeline, &logo_diff );
	Surface_Plain dragon_head_surface( &renderer, &plain_pipeline, &dragon_head_diff );
	Surface_Plain monkey_surface( &renderer, &plain_pipeline, &monkey_diff );

	// camera
	SceneObject_Camera camera( &renderer );
	camera.position		= { 0.5, -0.8, -1.0 };
	camera.rotation		= glm::vec3( -0.5, 0.0, 0.0 );

	// scene objects
	SceneObject_DynamicObject logo_object( &renderer, { &logo_surface }, MESH_OBJECT_SHAPE::PLANE );
	logo_object.position.x			= -1;

	SceneObject_DynamicObject dragon_head_object( &renderer, { &dragon_head_surface }, "models/BlackDragonHead.me3d" );
	dragon_head_object.position.x	= 1;
	dragon_head_object.size			= { 0.15, 0.15, 0.15 };

	SceneObject_DynamicObject monkey_object( &renderer, { &monkey_surface }, "models/Monkey.me3d" );
	monkey_object.position			= { 0, 0, 0.5 };
	monkey_object.size				= { 0.35, 0.35, 0.35 };


	// create command pool and buffer that are used for rendering
	VkCommandPool command_pool			= VK_NULL_HANDLE;
	VkCommandPoolCreateInfo pool_create_info {};
	pool_create_info.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_create_info.flags				= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_create_info.queueFamilyIndex	= renderer.GetVulkanGraphicsQueueFamilyIndex();
	vkCreateCommandPool( renderer.GetVulkanDevice(), &pool_create_info, nullptr, &command_pool );

	VkCommandBuffer command_buffer					= VK_NULL_HANDLE;
	VkCommandBufferAllocateInfo	command_buffer_allocate_info {};
	command_buffer_allocate_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool		= command_pool;
	command_buffer_allocate_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount	= 1;
	vkAllocateCommandBuffers( renderer.GetVulkanDevice(), &command_buffer_allocate_info, &command_buffer );

	// create a semaphore to signal when the rendering is complete
	VkSemaphore render_complete_semaphore	= VK_NULL_HANDLE;
	VkSemaphoreCreateInfo semaphore_create_info {};
	semaphore_create_info.sType				= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore( renderer.GetVulkanDevice(), &semaphore_create_info, nullptr, &render_complete_semaphore );

	// not really necessary as texture loaders will synchronize the queue too, but just in case I forgot something
	vkQueueWaitIdle( renderer.GetVulkanQueue() );

	// see how long the resource loading took
	std::cout << "Load time: " << chrono::duration_cast<chrono::milliseconds>( timer.now() - program_start_time ).count() << std::endl;

	auto last_time			= timer.now();
	uint64_t frame_counter	= 0;
	uint64_t fps			= 0;

	double camera_rotator	= 0.0f;
	double rotator			= 0.0f;

	// main loop
	while( renderer.Run() ) {
		// CPU logic calculations

		++frame_counter;
		if( last_time + chrono::seconds( 1 ) < timer.now() ) {
			last_time		= timer.now();
			fps				= frame_counter;
			frame_counter	= 0;
			std::cout << "FPS: " << fps << std::endl;
		}

		// Begin render
		window->BeginRender();

		// Record command buffer
		VkCommandBufferBeginInfo command_buffer_begin_info {};
		command_buffer_begin_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer( command_buffer, &command_buffer_begin_info );

		VkRect2D render_area {};
		render_area.offset.x		= 0;
		render_area.offset.y		= 0;
		render_area.extent			= window->GetVulkanSurfaceSize();

		std::array<VkClearValue, 2> clear_values {};
		clear_values[ 0 ].depthStencil.depth		= 1.0f;			// DO NOT FORGET TO PUT THIS TO 1.0f!!! othervise will not render.
		clear_values[ 0 ].depthStencil.stencil		= 0;
		clear_values[ 1 ].color.float32[ 0 ]		= 0.1f;
		clear_values[ 1 ].color.float32[ 1 ]		= 0.1f;
		clear_values[ 1 ].color.float32[ 2 ]		= 0.1f;
		clear_values[ 1 ].color.float32[ 3 ]		= 1.0f;

		VkRenderPassBeginInfo render_pass_begin_info {};
		render_pass_begin_info.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass			= window->GetVulkanRenderPass();
		render_pass_begin_info.framebuffer			= window->GetVulkanActiveFramebuffer();
		render_pass_begin_info.renderArea			= render_area;
		render_pass_begin_info.clearValueCount		= clear_values.size();
		render_pass_begin_info.pClearValues			= clear_values.data();

		vkCmdBeginRenderPass( command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE );

		// update camera data, ubo and descriptor set, because all pipelines use this camera descriptor set we only need to do this once
		camera_rotator		+= 0.0055;
		camera.position.x	= cos( camera_rotator ) / 2;
		camera.CmdUpdateUBOAndBindDescriptorSetsForPipeline( command_buffer, 60.0f, window->GetVulkanSurfaceSize(), 0.01f, 100.0f );

		vkCmdBindPipeline( command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, plain_pipeline.GetVulkanPipeline() );

		// modify the objects rotation slightly
		rotator += 0.01;
		logo_object.rotation = glm::vec3( 0, rotator, 0 );
		dragon_head_object.rotation = glm::vec3( 0, rotator, 0 );
		monkey_object.rotation = glm::vec3( 0, rotator, 0 );

		// render objects individually
		logo_object.CmdRender( command_buffer );
		dragon_head_object.CmdRender( command_buffer );
		monkey_object.CmdRender( command_buffer );

		vkCmdEndRenderPass( command_buffer );

		vkEndCommandBuffer( command_buffer );

		// Submit command buffer
		VkSubmitInfo submit_info {};
		submit_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount		= 0;
		submit_info.pWaitSemaphores			= nullptr;
		submit_info.pWaitDstStageMask		= nullptr;
		submit_info.commandBufferCount		= 1;
		submit_info.pCommandBuffers			= &command_buffer;
		submit_info.signalSemaphoreCount	= 1;
		submit_info.pSignalSemaphores		= &render_complete_semaphore;

		vkQueueSubmit( renderer.GetVulkanQueue(), 1, &submit_info, VK_NULL_HANDLE );

		// End render
		window->EndRender( { render_complete_semaphore } );
	}

	vkQueueWaitIdle( renderer.GetVulkanQueue() );

	vkDestroySemaphore( renderer.GetVulkanDevice(), render_complete_semaphore, nullptr );
	vkDestroyCommandPool( renderer.GetVulkanDevice(), command_pool, nullptr );

	return 0;
}
