#pragma once

#include <grend/ecs/ecs.hpp>

using namespace grendx;
using namespace grendx::ecs;

class playerMovementSystem : public entitySystem {
	public:
		typedef std::shared_ptr<playerMovementSystem> ptr;
		typedef std::weak_ptr<playerMovementSystem>   weakptr;

		virtual ~playerMovementSystem();
		virtual void update(entityManager *manager, float delta);
};
