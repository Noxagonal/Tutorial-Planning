#include "Scene.h"

#include "Platform.h"
#include "SceneObject.h"

Scene::Scene()
{
}


Scene::~Scene()
{
}

void Scene::UpdateLogic()
{
	for( auto & o : objects ) {
		o->UpdateLogic();
	}
}

void Scene::CmdRender( VkCommandBuffer command_buffer )
{
	for( auto & o : objects ) {
		o->CmdRender( command_buffer );
	}
}

void Scene::AddObject( SceneObject * object )
{
	assert( nullptr != object );
	objects.push_back( object );
}

void Scene::RemoveObject( SceneObject * object )
{
	assert( nullptr != object );
	objects.remove( object );
}
