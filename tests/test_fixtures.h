/**************************************************************************/
/*  test_fixtures.h                                                       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "servers/rendering_server.h"
#include "tests/test_macros.h"

namespace TestFixtures {

/**
 * @brief Macro to skip test if FlecsServer is not available
 * 
 * Use this at the start of tests that require FlecsServer.
 */
#define REQUIRE_FLECS_SERVER() \
	do { \
		if (!FlecsServer::get_singleton()) { \
			MESSAGE("Skipping test - FlecsServer not initialized"); \
			return; \
		} \
	} while (0)

/**
 * @brief Macro to skip test if RenderingServer is not available
 * 
 * Use this at the start of tests that require RenderingServer.
 */
#define REQUIRE_RENDERING_SERVER() \
	do { \
		if (!RenderingServer::get_singleton()) { \
			MESSAGE("Skipping test - RenderingServer not initialized"); \
			return; \
		} \
	} while (0)

/**
 * @brief Macro to skip test if both servers are required
 */
#define REQUIRE_BOTH_SERVERS() \
	do { \
		REQUIRE_FLECS_SERVER(); \
		REQUIRE_RENDERING_SERVER(); \
	} while (0)

/**
 * @brief Base fixture for tests that require FlecsServer
 * 
 * This fixture ensures FlecsServer singleton is properly initialized
 * and provides helper methods for world management.
 */
class FlecsServerFixture {
protected:
	RID world_id;
	FlecsServer *server = nullptr;

	FlecsServerFixture() {
		server = FlecsServer::get_singleton();
		if (!server) {
			ERR_PRINT("FlecsServer singleton not available in test");
		}
	}

	virtual ~FlecsServerFixture() {
		cleanup_world();
	}

	/**
	 * @brief Create a test world
	 * @return RID of the created world
	 */
	RID create_world() {
		if (server) {
			world_id = server->create_world();
			return world_id;
		}
		return RID();
	}

	/**
	 * @brief Get the current test world
	 * @return Pointer to the flecs::world instance
	 */
	flecs::world* get_world() {
		if (server && world_id.is_valid()) {
			return server->_get_world(world_id);
		}
		return nullptr;
	}

	/**
	 * @brief Get an entity from the current world
	 * @param entity_rid RID of the entity
	 * @return flecs::entity instance
	 */
	flecs::entity get_entity(const RID &entity_rid) {
		if (server && world_id.is_valid()) {
			return server->_get_entity(world_id, entity_rid);
		}
		return flecs::entity();
	}

	/**
	 * @brief Clean up the test world
	 */
	void cleanup_world() {
		if (server && world_id.is_valid()) {
			server->free_world(world_id);
			world_id = RID();
		}
	}

	/**
	 * @brief Check if FlecsServer is available
	 */
	bool is_server_available() const {
		return server != nullptr;
	}
};

/**
 * @brief Fixture for tests that require RenderingServer
 * 
 * This fixture ensures RenderingServer singleton is available.
 * Note: RenderingServer is initialized by the test framework,
 * this fixture just provides helper methods.
 */
class RenderingServerFixture {
protected:
	RenderingServer *rs = nullptr;

	RenderingServerFixture() {
		rs = RenderingServer::get_singleton();
		if (!rs) {
			ERR_PRINT("RenderingServer singleton not available in test");
		}
	}

	virtual ~RenderingServerFixture() {
		// RenderingServer is managed by engine, don't clean up
	}

	/**
	 * @brief Check if RenderingServer is available
	 */
	bool is_rendering_server_available() const {
		return rs != nullptr;
	}

	/**
	 * @brief Create a material RID for testing
	 */
	RID create_test_material() {
		if (rs) {
			return rs->material_create();
		}
		return RID();
	}

	/**
	 * @brief Create a mesh RID for testing
	 */
	RID create_test_mesh() {
		if (rs) {
			return rs->mesh_create();
		}
		return RID();
	}

	/**
	 * @brief Free a RenderingServer RID
	 */
	void free_rid(const RID &rid) {
		if (rs && rid.is_valid()) {
			rs->free(rid);
		}
	}
};

/**
 * @brief Combined fixture for tests that need both servers
 */
class CombinedServerFixture : public FlecsServerFixture, public RenderingServerFixture {
protected:
	CombinedServerFixture() : FlecsServerFixture(), RenderingServerFixture() {}

	virtual ~CombinedServerFixture() = default;

	/**
	 * @brief Check if all required servers are available
	 */
	bool are_servers_available() const {
		return is_server_available() && is_rendering_server_available();
	}
};

} // namespace TestFixtures