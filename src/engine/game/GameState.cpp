
#include "Game.hpp"
#include "GameState.hpp"
#include "memory"
#include <vector>
#include <engine/graphics/core/sampler.hpp>
#include <engine/graphics/core/texture.hpp>
#include "engine/graphics/renderer/mesh.hpp"
#include "engine/graphics/camera.hpp"
#include "engine/graphics/renderer/meshrenderer.hpp"


void game::GameState::Cleanup() {
	meshRenderer.clear();
}


bool game::GameState::isFinished() {
	return GameState::is_finished;
};