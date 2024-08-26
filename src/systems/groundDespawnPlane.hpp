#pragma once

#include <grend/ecs/ecs.hpp>
#include <grend/camera.hpp>

using namespace grendx;
using namespace grendx::ecs;

class groundDespawnPlane : public entitySystem {
	public:
		typedef std::shared_ptr<groundDespawnPlane> ptr;
		typedef std::weak_ptr<groundDespawnPlane>   weakptr;

		virtual ~groundDespawnPlane();
		virtual void update(entityManager *manager, float delta);

		camera::ptr cam = std::make_shared<camera>();
};

