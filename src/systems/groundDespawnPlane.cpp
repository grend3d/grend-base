#include <systems/groundDespawnPlane.hpp>
#include <grend/interpolation.hpp>
#include <grend/scancodes.hpp>
#include <grend/ecs/rigidBody.hpp>

using namespace grendx;
using namespace grendx::ecs;
using namespace grendx::engine;
using namespace grendx::interp;

groundDespawnPlane::~groundDespawnPlane() {};

void groundDespawnPlane::update(entityManager *entities, float delta) {
	for (auto [ent, body] : entities->search<rigidBody>()) {
		glm::vec3 pos = ent->transform.getTRS().position;

		// TODO: configurable ground plane
		if (pos.y < -100) {
			LogFmt("Moving entity back near origin: {}", (void*)ent);
			ent->transform.set(TRS {.position = {0, 10, 0}});
			updateEntityTransforms(entities, ent, ent->transform.getTRS());
			//LogFmt("Removing entity: {}", (void*)ent);
			//entities->remove(ent);

		}
	}
}
