#include "Texture.h"

#include "Platform.h"
#include "Renderer.h"
#include "Shared.h"

#include <FreeImage.h>
#include <memory>
#include <algorithm>
#include <list>
#include <vector>

struct MipMap
{
	VkExtent2D					dimensions_size;
	uint32_t					byte_size;
	FIBITMAP				*	image;
	uint32_t					offset;
};

uint32_t RoundToMultipleOf( uint32_t value, uint32_t multiple_of )
{
	return ( ( value / multiple_of + !!( value % multiple_of ) ) * multiple_of );
}

Texture::Texture( Renderer * renderer, std::wstring path )
{
	_ref_renderer			= renderer;
	_ref_vk_device			= _ref_renderer->GetVulkanDevice();

	// load image on the cpu side, Also try OpenImageIO
	auto fi_image			= FreeImage_LoadU( FreeImage_GetFileTypeU( path.c_str() ), path.c_str() );
	if( nullptr == fi_image ) {
		assert( 0 && "Couldn't load image." );
		return;
	}
	if( FreeImage_GetBPP( fi_image ) != 32 ) {
		auto fi_temp_image	= FreeImage_ConvertTo32Bits( fi_image );
		FreeImage_Unload( fi_image );
		fi_image			= fi_temp_image;
	}
	FreeImage_FlipVertical( fi_image );
	_image_format			= VK_FORMAT_B8G8R8A8_UNORM;
	auto fi_image_type		= FreeImage_GetImageType( fi_image );
	auto fi_color_type		= FreeImage_GetColorType( fi_image );
	_size.width				= uint32_t( FreeImage_GetWidth( fi_image ) );
	_size.height			= uint32_t( FreeImage_GetHeight( fi_image ) );
	auto fi_bpp				= uint32_t( FreeImage_GetBPP( fi_image ) );

	auto fi_byte_size				= ( fi_bpp / 8 ) * _size.width * _size.height;

	// generate mipmaps
	std::vector<MipMap> mipmaps;
	mipmaps.reserve( 16 );
	{
		uint32_t current_offset	= 0;

		MipMap last;
		last.dimensions_size	= _size;
		last.byte_size			= _size.width * _size.height * ( fi_bpp / 8 );
		last.image				= fi_image;
		last.offset				= current_offset;
		mipmaps.push_back( last );

		while( last.dimensions_size.width != 1 && last.dimensions_size.height != 1 ) {
			VkExtent2D current_dim_size	= { last.dimensions_size.width / 2, last.dimensions_size.height / 2 };
			if( current_dim_size.width < 1 )	current_dim_size.width = 1;
			if( current_dim_size.height < 1 )	current_dim_size.height = 1;
			uint32_t current_byte_size	= current_dim_size.width * current_dim_size.height * ( fi_bpp / 8 );
			current_offset				+= RoundToMultipleOf( last.byte_size, 4 );

			MipMap current;
			current.dimensions_size		= current_dim_size;
			current.byte_size			= current_byte_size;
			current.image				= FreeImage_Rescale( last.image, current_dim_size.width, current_dim_size.height );
			current.offset				= current_offset;

			mipmaps.push_back( current );
			last		= current;
		}
	}
	uint32_t request_buffer_size	= mipmaps.back().offset + mipmaps.back().byte_size;
	/*
	{
		auto & m			= mipmaps[ 1 ];
		auto save_image		= FreeImage_Allocate( m.dimensions_size.width, m.dimensions_size.height, fi_bpp );
		std::memcpy( FreeImage_GetBits( save_image ), FreeImage_GetBits( m.image ), m.dimensions_size.width * m.dimensions_size.height * ( fi_bpp / 8 ) );
		FreeImage_SaveU( FIF_DDS, save_image, ( path + L".test.dds" ).c_str() );
		FreeImage_Unload( save_image );
	}
	*/
	VkBuffer		staging_buffer			= VK_NULL_HANDLE;
	VkDeviceMemory	staging_buffer_memory	= VK_NULL_HANDLE;
	{
		VkBufferCreateInfo buffer_create_info {};
		buffer_create_info.sType		= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.flags		= 0;
		buffer_create_info.size			= request_buffer_size;
		buffer_create_info.usage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		buffer_create_info.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;
		ErrorCheck( vkCreateBuffer( _ref_vk_device, &buffer_create_info, nullptr, &staging_buffer ) );

		VkMemoryRequirements memory_requirements {};
		vkGetBufferMemoryRequirements( _ref_vk_device, staging_buffer, &memory_requirements );
		auto memory_index	= FindMemoryTypeIndex( &_ref_renderer->GetVulkanPhysicalDeviceMemoryProperties(), &memory_requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

		VkMemoryAllocateInfo memory_allocate_info {};
		memory_allocate_info.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.allocationSize		= memory_requirements.size;
		memory_allocate_info.memoryTypeIndex	= memory_index;
		ErrorCheck( vkAllocateMemory( _ref_vk_device, &memory_allocate_info, nullptr, &staging_buffer_memory ) );
		ErrorCheck( vkBindBufferMemory( _ref_vk_device, staging_buffer, staging_buffer_memory, 0 ) );
		{
			uint8_t * data = nullptr;
			ErrorCheck( vkMapMemory( _ref_vk_device, staging_buffer_memory, 0, memory_requirements.size, 0, (void**)&data ) );
			for( auto & mip : mipmaps ) {
				uint32_t mip_byte_size = mip.dimensions_size.width * mip.dimensions_size.height * ( fi_bpp / 8 );
				std::memcpy( &data[ mip.offset ], FreeImage_GetBits( mip.image ), mip_byte_size );
			}
			vkUnmapMemory( _ref_vk_device, staging_buffer_memory );
		}
	}
	for( auto & mip : mipmaps ) {
		FreeImage_Unload( mip.image );
		mip.image = nullptr;
	}

	// Create on-device image
	{
		VkImageCreateInfo image_create_info {};
		image_create_info.sType				= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.flags				= 0;
		image_create_info.imageType			= VK_IMAGE_TYPE_2D;
		image_create_info.format			= _image_format;
		image_create_info.extent			= { _size.width, _size.height, 1 };
		image_create_info.mipLevels			= uint32_t( mipmaps.size() );
		image_create_info.arrayLayers		= 1;
		image_create_info.samples			= VK_SAMPLE_COUNT_1_BIT;
		image_create_info.tiling			= VK_IMAGE_TILING_OPTIMAL;
		image_create_info.usage				= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		image_create_info.sharingMode		= VK_SHARING_MODE_EXCLUSIVE;
		image_create_info.initialLayout		= VK_IMAGE_LAYOUT_UNDEFINED;
		ErrorCheck( vkCreateImage( _ref_vk_device, &image_create_info, nullptr, &_image ) );

		VkMemoryRequirements memory_requirements {};
		vkGetImageMemoryRequirements( _ref_vk_device, _image, &memory_requirements );

		auto memory_index = FindMemoryTypeIndex( &_ref_renderer->GetVulkanPhysicalDeviceMemoryProperties(), &memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

		VkMemoryAllocateInfo allocate_info {};
		allocate_info.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocate_info.allocationSize	= memory_requirements.size;
		allocate_info.memoryTypeIndex	= memory_index;
		ErrorCheck( vkAllocateMemory( _ref_vk_device, &allocate_info, nullptr, &_image_memory ) );
		ErrorCheck( vkBindImageMemory( _ref_vk_device, _image, _image_memory, 0 ) );

		VkImageViewCreateInfo image_view_create_info {};
		image_view_create_info.sType			= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.flags			= 0;
		image_view_create_info.image			= _image;
		image_view_create_info.viewType			= VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format			= _image_format;
		image_view_create_info.components.r		= VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.g		= VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.b		= VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.a		= VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.baseMipLevel	= 0;
		image_view_create_info.subresourceRange.levelCount		= uint32_t( mipmaps.size() );
		image_view_create_info.subresourceRange.baseArrayLayer	= 0;
		image_view_create_info.subresourceRange.layerCount		= 1;
		ErrorCheck( vkCreateImageView( _ref_vk_device, &image_view_create_info, nullptr, &_image_view ) );
	}

	// Copy the staging buffer to the final image and set the image layouts properly
	// We can't directly just copy this to the GPU, instead we submit a command buffer to do it.
	{
		// Create a few necessitites
		VkCommandPool pool = VK_NULL_HANDLE;
		VkCommandPoolCreateInfo pool_create_info {};
		pool_create_info.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_create_info.flags				= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		pool_create_info.queueFamilyIndex	= _ref_renderer->GetVulkanGraphicsQueueFamilyIndex();
		ErrorCheck( vkCreateCommandPool( _ref_vk_device, &pool_create_info, nullptr, &pool ) );

		VkCommandBuffer buffer = VK_NULL_HANDLE;
		VkCommandBufferAllocateInfo buffer_allocate_info {};
		buffer_allocate_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		buffer_allocate_info.commandPool		= pool;
		buffer_allocate_info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		buffer_allocate_info.commandBufferCount	= 1;
		ErrorCheck( vkAllocateCommandBuffers( _ref_vk_device, &buffer_allocate_info, &buffer ) );

		// Begin command buffer
		VkCommandBufferBeginInfo buffer_begin_info {};
		buffer_begin_info.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		buffer_begin_info.flags					= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		ErrorCheck( vkBeginCommandBuffer( buffer, &buffer_begin_info ) );

		// set up common values for both images we handle here.
		VkImageMemoryBarrier image_memory_barrier {};
		image_memory_barrier.sType					= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barrier.srcQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.dstQueueFamilyIndex	= VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		image_memory_barrier.subresourceRange.baseMipLevel		= 0;
		image_memory_barrier.subresourceRange.levelCount		= uint32_t( mipmaps.size() );
		image_memory_barrier.subresourceRange.baseArrayLayer	= 0;
		image_memory_barrier.subresourceRange.layerCount		= 1;

		// translate the layout of the final image from undefined to transfer destination optimal
		image_memory_barrier.srcAccessMask			= 0;
		image_memory_barrier.dstAccessMask			= VK_ACCESS_TRANSFER_WRITE_BIT;
		image_memory_barrier.oldLayout				= VK_IMAGE_LAYOUT_UNDEFINED;
		image_memory_barrier.newLayout				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_memory_barrier.image					= _image;
		vkCmdPipelineBarrier( buffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &image_memory_barrier );

		// copy buffer to image
		std::vector<VkBufferImageCopy> regions;
		regions.reserve( 16 );
		for( size_t i=0; i < mipmaps.size(); ++i ) {
			auto & m = mipmaps[ i ];
			VkBufferImageCopy region {};
			region.bufferOffset						= m.offset;
			region.bufferRowLength					= 0;
			region.bufferImageHeight				= 0;
			region.imageSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel		= i;
			region.imageSubresource.baseArrayLayer	= 0;
			region.imageSubresource.layerCount		= 1;
			region.imageOffset						= { 0, 0, 0 };
			region.imageExtent						= { m.dimensions_size.width, m.dimensions_size.height, 1 };
			regions.push_back( region );
		}
		vkCmdCopyBufferToImage( buffer,
			staging_buffer,
			_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			uint32_t( regions.size() ), regions.data() );

		// translate the layout of the final image from transfer destination optimal into shader read only optimal
		image_memory_barrier.srcAccessMask			= VK_ACCESS_TRANSFER_WRITE_BIT;
		image_memory_barrier.dstAccessMask			= VK_ACCESS_SHADER_READ_BIT;
		image_memory_barrier.oldLayout				= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_memory_barrier.newLayout				= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_memory_barrier.image					= _image;
		vkCmdPipelineBarrier( buffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &image_memory_barrier );

		ErrorCheck( vkEndCommandBuffer( buffer ) );

		VkSubmitInfo submit_info {};
		submit_info.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount	= 1;
		submit_info.pCommandBuffers		= &buffer;
		ErrorCheck( vkQueueSubmit( _ref_renderer->GetVulkanQueue(), 1, &submit_info, VK_NULL_HANDLE ) );
		vkQueueWaitIdle( _ref_renderer->GetVulkanQueue() );

		vkDestroyCommandPool( _ref_vk_device, pool, nullptr );
	}
	// delete temporary data
	vkDestroyBuffer( _ref_vk_device, staging_buffer, nullptr );
	vkFreeMemory( _ref_vk_device, staging_buffer_memory, nullptr );

	// Finally create a sampler
	{
		VkSamplerCreateInfo sampler_create_info {};
		sampler_create_info.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_create_info.flags					= 0;
		sampler_create_info.magFilter				= VK_FILTER_LINEAR;
		sampler_create_info.minFilter				= VK_FILTER_LINEAR;
		sampler_create_info.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_create_info.addressModeU			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_create_info.addressModeV			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_create_info.addressModeW			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_create_info.mipLodBias				= 0.0f;
		sampler_create_info.anisotropyEnable		= _ref_renderer->GetVulkanPhysicalDeviceFeatures().samplerAnisotropy;
		sampler_create_info.maxAnisotropy			= _ref_renderer->GetVulkanPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
		sampler_create_info.compareEnable			= VK_FALSE;
		sampler_create_info.compareOp				= VK_COMPARE_OP_NEVER;
		sampler_create_info.minLod					= 0.0f;
		sampler_create_info.maxLod					= float( mipmaps.size() );
		sampler_create_info.borderColor				= VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		sampler_create_info.unnormalizedCoordinates	= VK_FALSE;
		vkCreateSampler( _ref_vk_device, &sampler_create_info, nullptr, &_sampler );
	}
}


Texture::~Texture()
{
	vkDestroySampler( _ref_vk_device, _sampler, nullptr );
	vkDestroyImage( _ref_vk_device, _image, nullptr );
	vkDestroyImageView( _ref_vk_device, _image_view, nullptr );
	vkFreeMemory( _ref_vk_device, _image_memory, nullptr );
}

VkImage Texture::GetVulkanImage()
{
	return _image;
}

VkImageView Texture::GetVulkanImageView()
{
	return _image_view;
}

VkSampler Texture::GetVulkanSampler()
{
	return _sampler;
}

VkFormat Texture::GetFormat()
{
	return _image_format;
}
