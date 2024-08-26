#include <grend/gameMainDevWindow.hpp>
#include <grend/loadScene.hpp>
#include <grend/renderUtils.hpp>
#include <grend/interpolation.hpp>

#include <grend/ecs/sceneComponent.hpp>
#include <grend/ecs/collision.hpp>
#include <grend/ecs/rigidBody.hpp>
#include <grend/ecs/shader.hpp>

#include <grend/ecs/editor.hpp>

#include <grend/ecs/ref.hpp>
#include <grend/ecs/link.hpp>

#include <grend/ecs/animationController.hpp>

#include <systems/playerCameraSystem.hpp>
#include <systems/playerMovementSystem.hpp>
#include <systems/groundDespawnPlane.hpp>

#include <iostream>
#include <fstream>
#include <functional>

using namespace grendx;
using namespace grendx::ecs;
using namespace grendx::engine;
using namespace grendx::interp;

renderPostChain::ptr createPost(camera::ptr cam) {
	auto rend = Resolve<renderContext>();
	auto ctx  = Resolve<SDLContext>();

	auto settings = ctx->getSettings();

	return std::make_shared<renderPostChain>(
		std::vector {
			/*
			loadPostShader(GR_PREFIX "shaders/baked/deferred-metal-roughness-pbr.frag",
			               rend->globalShaderOptions),
						   */
			/*
			loadPostShader(GR_PREFIX "shaders/baked/fog-volumetric.frag",
							rend->globalShaderOptions),
			 */
			loadPostShader(GR_PREFIX "shaders/baked/tonemap.frag",
			               rend->globalShaderOptions),
			/*
			loadPostShader(GR_PREFIX "shaders/baked/boxblur.frag",
			               rend->globalShaderOptions),
			 */
		});
}

class gamething : public gameView {
	renderPostChain::ptr post;
	Texture::ptr bluenoise;

	public:
		gamething() {
			cam->setFar(1000);
			post = createPost(cam);

			auto data = std::make_shared<textureData>(GR_PREFIX "assets/tex/bluenoise.png");
			//auto data = std::make_shared<textureData>("/tmp/bayer.ppm");
			data->minFilter = textureData::Nearest;
			data->magFilter = textureData::Nearest;
			bluenoise = texcache(data);
		};

		virtual void handleEvent(const SDL_Event& ev) {
			if (ev.type == SDL_WINDOWEVENT
			    && ev.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				auto rend = Resolve<renderContext>();

				auto width = ev.window.data1;
				auto height = ev.window.data2;

				float scaleX = rend->settings.scaleX;
				float scaleY = rend->settings.scaleY;

				rend->framebuffer->setSize(width*scaleX, height*scaleY);
				rend->defaultFramebuffer->setSize(width, height);

				/*
				if (post != nullptr) {
					post->setSize(width, height);
				}
				*/
			}
		}

		virtual void update(float delta) {
			auto phys     = Resolve<physics>();
			auto entities = Resolve<ecs::entityManager>();

			phys->stepSimulation(delta);
			phys->filterCollisions();;

			entities->update(delta);
		};

		virtual void render(renderFramebuffer::ptr fb) {
			auto rend  = Resolve<renderContext>();
			auto state = Resolve<gameState>();
			auto entities = Resolve<ecs::entityManager>();

			const auto cam = static_pointer_cast<playerCameraSystem>(entities->systems["playerCamera"])->cam;

			auto vportPos  = rend->getDrawOffset();
			auto vportSize = rend->getDrawSize();
			glViewport(vportPos.x, fb->height - (vportPos.y + vportSize.y), vportSize.x, vportSize.y);

			auto que = buildDrawableQueue();
			que.add(rend->getLightingFlags(), state->rootnode);
			drawMultiQueue(que, rend->framebuffer, cam);

			rend->defaultSkybox->draw(cam, rend->framebuffer);
			setPostUniforms(post, cam);
			glActiveTexture(TEX_GL_LIGHTMAP);
			bluenoise->bind();
			post->setUniform("blueNoise", TEXU_LIGHTMAP);
			post->draw(rend->framebuffer, rend->defaultFramebuffer);
		};
};

int main(int argc, char **argv) {
	//unsigned x = 1280;
	//unsigned y = 720;
	//unsigned x = 1920;
	//unsigned y = 1080;
	unsigned x = 2560;
	unsigned y = 1440;

	renderSettings settings = {
		.scaleX     = 1.0,
		.scaleY     = 1.0,
		//.scaleX     = 1.0,
		//.scaleY     = 1.0,
		//.targetResX = 2560,
		//.targetResY = 1440,
		.targetResX = x,
		.targetResY = y,
		.msaaLevel  = 0,
		//.windowResX = 2560,
		//.windowResY = 1440,
		.windowResX = x,
		.windowResY = y,
		.vsync      = 1,
		.UIScale    = 2.0,
		//.UIScale    = 1.0,
	};

	dev::initialize("grend editor", settings);

	auto state     = Resolve<gameState>();
	auto rend      = Resolve<renderContext>();
	auto entities  = Resolve<ecs::entityManager>();
	auto factories = Resolve<ecs::serializer>();

	auto view = std::make_shared<gamething>();
	//dev::setView(view);

	//rend->setDefaultLightModel("unshaded");

	const char *mapfile = (argc > 1)? argv[1] : "save.map";

	if (auto p = loadMapCompiled(mapfile)) {
		state->rootnode = *p;
	}

	for (auto* ent : entities->entities) {
		entities->activate(ent);
	}

	std::ifstream charjson("./char.ent");
	nlohmann::json js = nlohmann::json::parse(charjson);

	entity *temp = factories->build(entities, js);
	//entity *temp = entities->construct<entity>();
	entities->add(temp);
	auto *body = temp->attach<rigidBodySphere>(glm::vec3 {0, 40, 0}, 1, 1);
	//temp->attach<sceneComponent>(DEMO_PREFIX "assets/obj/BoomBox.glb");
	//temp->attach<PBRShader>();
	body->phys->setAngularFactor({0, 1, 0});

	rend->defaultSkybox = std::make_unique<skyRenderHDRI>("share/proj/assets/tex/blocky_photo_studio_2k_alt.hdr");

	entities->systems["collision"] = std::make_shared<entitySystemCollision>();
	entities->systems["syncPhysics"] = std::make_shared<rigidBodyUpdateSystem>();
	entities->systems["playerMovement"] = std::make_shared<playerMovementSystem>();
	entities->systems["playerCamera"] = std::make_shared<playerCameraSystem>();
	entities->systems["groundDespawnPlane"] = std::make_shared<groundDespawnPlane>();
	dev::run(view);

	std::cout << "It's alive!" << std::endl;
	return 0;
}
