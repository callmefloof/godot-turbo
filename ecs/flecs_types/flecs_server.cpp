#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"

#include "core/math/quaternion.h"
#include "core/math/vector4.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/os/os.h"
#include "core/string/node_path.h"
#include "core/string/print_string.h"
#include "core/templates/hash_set.h"
#include "core/string/string_name.h"
#include "core/error/error_macros.h"
#include "core/templates/a_hash_map.h"
#include "core/templates/rid.h"
#include "core/templates/rid_owner.h"
#include "core/templates/vector.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_script_system.h"
#include "modules/godot_turbo/ecs/systems/utility/node_storage.h"
#include "modules/godot_turbo/ecs/systems/utility/ref_storage.h"
#include "core/string/ustring.h"
#include "flecs_variant.h"
#include "modules/godot_turbo/ecs/components/all_components.h"
#include <cstdint>
#include <cstdio>

// Maximum recursion depth for cursor serialization to prevent stack overflow
static constexpr int MAX_CURSOR_DEPTH = 32;

// Helper function to recursively convert flecs cursor data to Godot Variant
static Variant cursor_to_variant_impl(flecs::cursor& cur, int depth);

static Variant cursor_to_variant(flecs::cursor& cur) {
	return cursor_to_variant_impl(cur, 0);
}

static Variant cursor_to_variant_impl(flecs::cursor& cur, int depth) {
	// Prevent stack overflow from deeply nested or circular structures
	if (depth > MAX_CURSOR_DEPTH) {
		WARN_PRINT("cursor_to_variant: Maximum recursion depth exceeded, truncating");
		return Variant();
	}

	flecs::entity type = cur.get_type();

	// Check if type entity is valid (non-zero ID)
	if (!type.is_valid()) {
		ERR_PRINT("cursor_to_variant: Type entity is invalid (ID is 0)");
		return Variant();
	}

	// Keep string_view alive while we use the pointer
	flecs::string_view name_view = type.name();
	const char* type_name = name_view.c_str();

	// Check if type name is valid
	if (!type_name || strlen(type_name) == 0) {
		ERR_PRINT("cursor_to_variant: Type has no name");
		return Variant();
	}

	// Handle opaque Godot types by getting the raw pointer and dereferencing
	if (strcmp(type_name, "Variant") == 0) {
		Variant* ptr = static_cast<Variant*>(cur.get_ptr());
		return ptr ? *ptr : Variant();
	}
	if (strcmp(type_name, "Dictionary") == 0) {
		Dictionary* ptr = static_cast<Dictionary*>(cur.get_ptr());
		return ptr ? *ptr : Dictionary();
	}
	if (strcmp(type_name, "Array") == 0) {
		Array* ptr = static_cast<Array*>(cur.get_ptr());
		return ptr ? *ptr : Array();
	}
	if (strcmp(type_name, "String") == 0) {
		String* ptr = static_cast<String*>(cur.get_ptr());
		return ptr ? *ptr : String();
	}
	if (strcmp(type_name, "StringName") == 0) {
		StringName* ptr = static_cast<StringName*>(cur.get_ptr());
		return ptr ? *ptr : StringName();
	}
	if (strcmp(type_name, "NodePath") == 0) {
		NodePath* ptr = static_cast<NodePath*>(cur.get_ptr());
		return ptr ? *ptr : NodePath();
	}
	if (strcmp(type_name, "Callable") == 0) {
		Callable* ptr = static_cast<Callable*>(cur.get_ptr());
		return ptr ? *ptr : Callable();
	}
	if (strcmp(type_name, "Signal") == 0) {
		Signal* ptr = static_cast<Signal*>(cur.get_ptr());
		return ptr ? *ptr : Signal();
	}
	if (strcmp(type_name, "RID") == 0) {
		RID* ptr = static_cast<RID*>(cur.get_ptr());
		if (!ptr) {
			WARN_PRINT("cursor_to_variant: RID type found but pointer is null");
			return RID();
		}
		return *ptr;
	}

	// Handle packed arrays
	if (strcmp(type_name, "PackedByteArray") == 0) {
		PackedByteArray* ptr = static_cast<PackedByteArray*>(cur.get_ptr());
		return ptr ? *ptr : PackedByteArray();
	}
	if (strcmp(type_name, "PackedInt32Array") == 0) {
		PackedInt32Array* ptr = static_cast<PackedInt32Array*>(cur.get_ptr());
		return ptr ? *ptr : PackedInt32Array();
	}
	if (strcmp(type_name, "PackedInt64Array") == 0) {
		PackedInt64Array* ptr = static_cast<PackedInt64Array*>(cur.get_ptr());
		return ptr ? *ptr : PackedInt64Array();
	}
	if (strcmp(type_name, "PackedFloat32Array") == 0) {
		PackedFloat32Array* ptr = static_cast<PackedFloat32Array*>(cur.get_ptr());
		return ptr ? *ptr : PackedFloat32Array();
	}
	if (strcmp(type_name, "PackedFloat64Array") == 0) {
		PackedFloat64Array* ptr = static_cast<PackedFloat64Array*>(cur.get_ptr());
		return ptr ? *ptr : PackedFloat64Array();
	}
	if (strcmp(type_name, "PackedStringArray") == 0) {
		PackedStringArray* ptr = static_cast<PackedStringArray*>(cur.get_ptr());
		return ptr ? *ptr : PackedStringArray();
	}
	if (strcmp(type_name, "PackedVector2Array") == 0) {
		PackedVector2Array* ptr = static_cast<PackedVector2Array*>(cur.get_ptr());
		return ptr ? *ptr : PackedVector2Array();
	}
	if (strcmp(type_name, "PackedVector3Array") == 0) {
		PackedVector3Array* ptr = static_cast<PackedVector3Array*>(cur.get_ptr());
		return ptr ? *ptr : PackedVector3Array();
	}
	if (strcmp(type_name, "PackedColorArray") == 0) {
		PackedColorArray* ptr = static_cast<PackedColorArray*>(cur.get_ptr());
		return ptr ? *ptr : PackedColorArray();
	}

	// Handle Godot math types
	if (strcmp(type_name, "Vector2") == 0) {
		Vector2* ptr = static_cast<Vector2*>(cur.get_ptr());
		return ptr ? *ptr : Vector2();
	}
	if (strcmp(type_name, "Vector2i") == 0) {
		Vector2i* ptr = static_cast<Vector2i*>(cur.get_ptr());
		return ptr ? *ptr : Vector2i();
	}
	if (strcmp(type_name, "Vector3") == 0) {
		Vector3* ptr = static_cast<Vector3*>(cur.get_ptr());
		return ptr ? *ptr : Vector3();
	}
	if (strcmp(type_name, "Vector3i") == 0) {
		Vector3i* ptr = static_cast<Vector3i*>(cur.get_ptr());
		return ptr ? *ptr : Vector3i();
	}
	if (strcmp(type_name, "Vector4") == 0) {
		Vector4* ptr = static_cast<Vector4*>(cur.get_ptr());
		return ptr ? *ptr : Vector4();
	}
	if (strcmp(type_name, "Vector4i") == 0) {
		Vector4i* ptr = static_cast<Vector4i*>(cur.get_ptr());
		return ptr ? *ptr : Vector4i();
	}
	if (strcmp(type_name, "Quaternion") == 0) {
		Quaternion* ptr = static_cast<Quaternion*>(cur.get_ptr());
		return ptr ? *ptr : Quaternion();
	}
	if (strcmp(type_name, "Transform2D") == 0) {
		Transform2D* ptr = static_cast<Transform2D*>(cur.get_ptr());
		return ptr ? *ptr : Transform2D();
	}
	if (strcmp(type_name, "Transform3D") == 0) {
		Transform3D* ptr = static_cast<Transform3D*>(cur.get_ptr());
		return ptr ? *ptr : Transform3D();
	}
	if (strcmp(type_name, "Rect2") == 0) {
		Rect2* ptr = static_cast<Rect2*>(cur.get_ptr());
		return ptr ? *ptr : Rect2();
	}
	if (strcmp(type_name, "Rect2i") == 0) {
		Rect2i* ptr = static_cast<Rect2i*>(cur.get_ptr());
		return ptr ? *ptr : Rect2i();
	}
	if (strcmp(type_name, "AABB") == 0) {
		AABB* ptr = static_cast<AABB*>(cur.get_ptr());
		return ptr ? *ptr : AABB();
	}
	if (strcmp(type_name, "Plane") == 0) {
		Plane* ptr = static_cast<Plane*>(cur.get_ptr());
		return ptr ? *ptr : Plane();
	}
	if (strcmp(type_name, "Basis") == 0) {
		Basis* ptr = static_cast<Basis*>(cur.get_ptr());
		return ptr ? *ptr : Basis();
	}
	if (strcmp(type_name, "Color") == 0) {
		Color* ptr = static_cast<Color*>(cur.get_ptr());
		return ptr ? *ptr : Color();
	}
	if (strcmp(type_name, "Projection") == 0) {
		Projection* ptr = static_cast<Projection*>(cur.get_ptr());
		return ptr ? *ptr : Projection();
	}

	// Try to get type kind from EcsType component
	if (type.has<EcsType>()) {
		const EcsType& ecs_type = type.get<EcsType>();
		if (ecs_type.kind == EcsPrimitiveType && type.has<EcsPrimitive>()) {
			const EcsPrimitive& prim = type.get<EcsPrimitive>();
			switch (prim.kind) {
				case EcsBool: return cur.get_bool();
				case EcsChar: return (int64_t)cur.get_char();
				case EcsU8: return (int64_t)cur.get_uint();
				case EcsU16: return (int64_t)cur.get_uint();
				case EcsU32: return (int64_t)cur.get_uint();
				case EcsU64: return (int64_t)cur.get_uint();
				case EcsI8: return cur.get_int();
				case EcsI16: return cur.get_int();
				case EcsI32: return cur.get_int();
				case EcsI64: return cur.get_int();
				case EcsF32: return (double)cur.get_float();
				case EcsF64: return cur.get_float();
				case EcsUPtr: return (int64_t)cur.get_uint();
				case EcsIPtr: return cur.get_int();
				case EcsString: {
					const char* str = cur.get_string();
					return str ? String(str) : String();
				}
				case EcsEntity: return (int64_t)cur.get_entity().id();
				default: break;
			}
		}

		// Handle structs/nested types
		if (ecs_type.kind == EcsStructType) {
			Dictionary dict;
			if (cur.push() == 0) {
				do {
					const char* member_name = cur.get_member();
					if (member_name && strlen(member_name) > 0) {
						Variant value = cursor_to_variant_impl(cur, depth + 1);
						dict[String(member_name)] = value;
					}
				} while (cur.next() == 0);
				cur.pop();
			}
			return dict;
		}
	}

	// If we reach here, the type wasn't handled - print warning for debugging
	// Check if it's an opaque type without reflection metadata
	if (!type.has<EcsType>()) {
		WARN_PRINT(vformat("cursor_to_variant: Type '%s' has no EcsType metadata (opaque type not in type name checks)", type_name));
	} else {
		const EcsType& ecs_type = type.get<EcsType>();
		WARN_PRINT(vformat("cursor_to_variant: Type '%s' has EcsType with kind=%d but wasn't handled", type_name, (int)ecs_type.kind));
	}
	return Variant();
}

// Helper function to convert component data to Dictionary using flecs cursor
// Maximum number of struct members to iterate to prevent infinite loops
static constexpr int MAX_STRUCT_MEMBERS = 64;

static Dictionary component_to_dict_cursor(flecs::entity entity, flecs::entity_t comp_type_id) {
	// Validate entity is still valid and alive before accessing
	if (!entity.is_valid() || !entity.is_alive()) {
		ERR_PRINT("component_to_dict_cursor: entity is not valid or not alive");
		return Dictionary();
	}

	// Validate component type ID
	if (comp_type_id == 0) {
		ERR_PRINT("component_to_dict_cursor: comp_type_id is 0");
		return Dictionary();
	}

	if (!entity.has(comp_type_id)) {
		return Dictionary();
	}

	const void* comp_ptr = entity.get(comp_type_id);
	if (!comp_ptr) {
		ERR_PRINT("component_to_dict_cursor: entity.get() returned null pointer");
		return Dictionary();
	}

	// Validate world before creating cursor
	flecs::world world = entity.world();
	if (!world.c_ptr()) {
		ERR_PRINT("component_to_dict_cursor: entity.world() returned invalid world");
		return Dictionary();
	}

	flecs::cursor cur = world.cursor(comp_type_id, const_cast<void*>(comp_ptr));

	// Get the type to check if it's a struct
	flecs::entity type = cur.get_type();

	// Check if type is valid (non-zero entity ID)
	if (type.is_valid() && type.has<EcsType>()) {
		const EcsType& ecs_type = type.get<EcsType>();
		if (ecs_type.kind == EcsStructType) {
			// It's a struct, convert members to dictionary
			Dictionary dict;
			if (cur.push() == 0) {
				int member_count = 0;
				do {
					// Prevent infinite loops on corrupted data
					if (member_count >= MAX_STRUCT_MEMBERS) {
						WARN_PRINT("component_to_dict_cursor: Maximum struct member count exceeded, truncating");
						break;
					}
					const char* member_name = cur.get_member();
					if (member_name && strlen(member_name) > 0) {
						Variant value = cursor_to_variant(cur);
						dict[String(member_name)] = value;
					}
					member_count++;
				} while (cur.next() == 0);
				cur.pop();
			}
			return dict;
		}
	}

	// Not a struct (opaque type or primitive), wrap in dictionary with "value" key
	Dictionary result;
	Variant value = cursor_to_variant(cur);
	result["value"] = value;
	return result;
}

// Helper function to set component data from Dictionary using flecs cursor
static void component_from_dict_cursor(flecs::entity entity, flecs::entity_t comp_type_id, const Dictionary& dict) {
	// Use ensure to get or create the component
	void* comp_ptr = entity.ensure(comp_type_id);
	if (!comp_ptr) {
		ERR_PRINT("Failed to get mutable component pointer");
		return;
	}

	// For opaque types, get the type name directly from the component entity
	// instead of relying on cursor (which may not have type info for opaque types)
	flecs::entity comp_entity(entity.world().c_ptr(), comp_type_id);
	// Keep string_view alive while we use the pointer
	flecs::string_view type_name_view = comp_entity.name();
	const char* type_name = type_name_view.c_str();

	if (!type_name || strlen(type_name) == 0) {
		ERR_PRINT("component_from_dict_cursor: Component type has no name");
		return;
	}

	// Handle opaque Godot types directly
	// Check if the dictionary has a single "value" key (opaque type wrapped)
	bool is_wrapped_opaque = dict.size() == 1 && dict.has("value");

	if (strcmp(type_name, "Variant") == 0) {
		Variant* ptr = static_cast<Variant*>(comp_ptr);
		if (ptr) {
			if (is_wrapped_opaque) {
				*ptr = dict["value"];
			} else if (dict.size() > 0) {
				*ptr = dict.values()[0];
			}
		}
		entity.modified(comp_type_id);
		return;
	}
	if (strcmp(type_name, "Dictionary") == 0) {
		Dictionary* ptr = static_cast<Dictionary*>(comp_ptr);
		if (ptr) {
			*ptr = dict;
		}
		entity.modified(comp_type_id);
		return;
	}
	if (strcmp(type_name, "RID") == 0) {
		RID* ptr = static_cast<RID*>(comp_ptr);
		if (ptr) {
			if (is_wrapped_opaque) {
				*ptr = dict["value"];
			}
		}
		entity.modified(comp_type_id);
		return;
	}
	if (strcmp(type_name, "String") == 0) {
		String* ptr = static_cast<String*>(comp_ptr);
		if (ptr) {
			if (is_wrapped_opaque) {
				*ptr = dict["value"];
			}
		}
		entity.modified(comp_type_id);
		return;
	}
	if (strcmp(type_name, "StringName") == 0) {
		StringName* ptr = static_cast<StringName*>(comp_ptr);
		if (ptr) {
			if (is_wrapped_opaque) {
				*ptr = dict["value"];
			}
		}
		entity.modified(comp_type_id);
		return;
	}
	if (strcmp(type_name, "Array") == 0) {
		Array* ptr = static_cast<Array*>(comp_ptr);
		if (ptr) {
			if (is_wrapped_opaque) {
				*ptr = dict["value"];
			}
		}
		entity.modified(comp_type_id);
		return;
	}

	// For structs, iterate through dictionary keys and set members
	// Create cursor after handling opaque types
	flecs::cursor cur = entity.world().cursor(comp_type_id, comp_ptr);
	flecs::entity type = cur.get_type();

	if (type.is_valid() && type.has<EcsType>()) {
		const EcsType& ecs_type = type.get<EcsType>();
		if (ecs_type.kind == EcsStructType && cur.push() == 0) {
			Array keys = dict.keys();
		for (int i = 0; i < keys.size(); i++) {
			String key = keys[i];
			Variant value = dict[key];

			if (cur.member(key.utf8().get_data()) == 0) {
				flecs::entity member_type = cur.get_type();
				// Keep string_view alive while we use the pointer
				flecs::string_view member_name_view = member_type.name();
				const char* member_type_name = member_name_view.c_str();

				// Set value based on type
				if (strcmp(member_type_name, "Variant") == 0) {
					Variant* ptr = static_cast<Variant*>(cur.get_ptr());
					if (ptr) {
						*ptr = value;
					}
				} else if (strcmp(member_type_name, "String") == 0) {
					String* ptr = static_cast<String*>(cur.get_ptr());
					if (ptr) {
						*ptr = value;
					}
				} else if (strcmp(member_type_name, "StringName") == 0) {
					StringName* ptr = static_cast<StringName*>(cur.get_ptr());
					if (ptr) {
						*ptr = value;
					}
				} else if (strcmp(member_type_name, "Dictionary") == 0) {
					Dictionary* ptr = static_cast<Dictionary*>(cur.get_ptr());
					if (ptr) {
						*ptr = value;
					}
				} else if (strcmp(member_type_name, "Array") == 0) {
					Array* ptr = static_cast<Array*>(cur.get_ptr());
					if (ptr) {
						*ptr = value;
					}
				} else if (strcmp(member_type_name, "RID") == 0) {
					RID* ptr = static_cast<RID*>(cur.get_ptr());
					if (ptr) {
						*ptr = value;
					}
				} else if (strcmp(member_type_name, "Vector2") == 0) {
					Vector2* ptr = static_cast<Vector2*>(cur.get_ptr());
					if (ptr) {
						*ptr = value;
					}
				} else if (strcmp(member_type_name, "Vector3") == 0) {
					Vector3* ptr = static_cast<Vector3*>(cur.get_ptr());
					if (ptr) {
						*ptr = value;
					}
				} else if (strcmp(member_type_name, "Quaternion") == 0) {
					Quaternion* ptr = static_cast<Quaternion*>(cur.get_ptr());
					if (ptr) {
						*ptr = value;
					}
				} else {
					// Try primitive types
					switch (value.get_type()) {
						case Variant::BOOL: {
							cur.set_bool(value);
							break;
						}
						case Variant::INT: {
							cur.set_int(value);
							break;
						}
						case Variant::FLOAT: {
							cur.set_float(value);
							break;
						}
						case Variant::STRING: {
							cur.set_string(String(value).utf8().get_data());
							break;
						}
						default:
							break;
					}
				}
			}
			}
			cur.pop();
		}
	}

	entity.modified(comp_type_id);
}

void FlecsServer::thread_func(void *p_udata) {

}

Error FlecsServer::init() {
	thread_exited = false;
	counter = 0;
	thread.start(FlecsServer::thread_func, this);
	return OK;
}

FlecsServer *FlecsServer::singleton = nullptr;

FlecsServer *FlecsServer::get_singleton() {
	if(!singleton){
		singleton = memnew(FlecsServer);
	}
	return singleton;
}


void FlecsServer::unlock() {
	mutex.unlock();
}

void FlecsServer::lock() {
	mutex.lock();
}

void FlecsServer::finish() {
	exit_thread = true;
	thread.wait_to_finish();
}

// ===== Query API Implementation =====

RID FlecsServer::create_query(const RID &world_id, const PackedStringArray &required_components) {
	CHECK_WORLD_VALIDITY_V(world_id, RID(), create_query);
	FlecsQuery query;
	query.init(world_id, required_components);
	return flecs_variant_owners.get(world_id).query_owner.make_rid(query);
}

Array FlecsServer::query_get_entities(const RID &world_id, const RID &query_id) {
	CHECK_QUERY_VALIDITY_V(query_id, world_id, Array(), query_get_entities);
	return query->get_entities();
}

Array FlecsServer::query_get_entities_with_components(const RID &world_id, const RID &query_id) {
	CHECK_QUERY_VALIDITY_V(query_id, world_id, Array(), query_get_entities_with_components);
	return query->get_entities_with_components();
}

int FlecsServer::query_get_entity_count(const RID &world_id, const RID &query_id) {
	CHECK_QUERY_VALIDITY_V(query_id, world_id, 0, query_get_entity_count);
	return query->get_entity_count();
}

Array FlecsServer::query_get_entities_limited(const RID &world_id, const RID &query_id, int max_count, int offset) {
	CHECK_QUERY_VALIDITY_V(query_id, world_id, Array(), query_get_entities_limited);
	return query->get_entities_limited(max_count, offset);
}

Array FlecsServer::query_get_entities_with_components_limited(const RID &world_id, const RID &query_id, int max_count, int offset) {
	CHECK_QUERY_VALIDITY_V(query_id, world_id, Array(), query_get_entities_with_components_limited);
	return query->get_entities_with_components_limited(max_count, offset);
}

bool FlecsServer::query_matches_entity(const RID &world_id, const RID &query_id, const RID &entity_rid) {
	CHECK_QUERY_VALIDITY_V(query_id, world_id, false, query_matches_entity);
	return query->matches_entity(entity_rid);
}

void FlecsServer::query_set_required_components(const RID &world_id, const RID &query_id, const PackedStringArray &components) {
	CHECK_QUERY_VALIDITY(query_id, world_id, query_set_required_components);
	query->set_required_components(components);
}

PackedStringArray FlecsServer::query_get_required_components(const RID &world_id, const RID &query_id) {
	CHECK_QUERY_VALIDITY_V(query_id, world_id, PackedStringArray(), query_get_required_components);
	return query->get_required_components();
}

void FlecsServer::query_set_caching_strategy(const RID &world_id, const RID &query_id, int strategy) {
	CHECK_QUERY_VALIDITY(query_id, world_id, query_set_caching_strategy);
	query->set_caching_strategy(static_cast<FlecsQuery::CachingStrategy>(strategy));
}

int FlecsServer::query_get_caching_strategy(const RID &world_id, const RID &query_id) {
	CHECK_QUERY_VALIDITY_V(query_id, world_id, 0, query_get_caching_strategy);
	return static_cast<int>(query->get_caching_strategy());
}

void FlecsServer::query_set_filter_name_pattern(const RID &world_id, const RID &query_id, const String &pattern) {
	CHECK_QUERY_VALIDITY(query_id, world_id, query_set_filter_name_pattern);
	query->set_filter_name_pattern(pattern);
}

String FlecsServer::query_get_filter_name_pattern(const RID &world_id, const RID &query_id) {
	CHECK_QUERY_VALIDITY_V(query_id, world_id, String(), query_get_filter_name_pattern);
	return query->get_filter_name_pattern();
}

void FlecsServer::query_clear_filter(const RID &world_id, const RID &query_id) {
	CHECK_QUERY_VALIDITY(query_id, world_id, query_clear_filter);
	query->clear_filter();
}

void FlecsServer::query_force_cache_refresh(const RID &world_id, const RID &query_id) {
	CHECK_QUERY_VALIDITY(query_id, world_id, query_force_cache_refresh);
	query->force_cache_refresh();
}

bool FlecsServer::query_is_cache_dirty(const RID &world_id, const RID &query_id) {
	CHECK_QUERY_VALIDITY_V(query_id, world_id, true, query_is_cache_dirty);
	return query->is_cache_dirty();
}

void FlecsServer::query_set_instrumentation_enabled(const RID &world_id, const RID &query_id, bool enabled) {
	CHECK_QUERY_VALIDITY(query_id, world_id, query_set_instrumentation_enabled);
	query->set_instrumentation_enabled(enabled);
}

bool FlecsServer::query_get_instrumentation_enabled(const RID &world_id, const RID &query_id) {
	CHECK_QUERY_VALIDITY_V(query_id, world_id, false, query_get_instrumentation_enabled);
	return query->get_instrumentation_enabled();
}

Dictionary FlecsServer::query_get_instrumentation_data(const RID &world_id, const RID &query_id) {
	CHECK_QUERY_VALIDITY_V(query_id, world_id, Dictionary(), query_get_instrumentation_data);
	return query->get_instrumentation_data();
}

void FlecsServer::query_reset_instrumentation(const RID &world_id, const RID &query_id) {
	CHECK_QUERY_VALIDITY(query_id, world_id, query_reset_instrumentation);
	query->reset_instrumentation();
}

void FlecsServer::free_query(const RID &world_id, const RID &query_id) {
	CHECK_WORLD_VALIDITY(world_id, free_query);
	flecs_variant_owners.get(world_id).query_owner.free(query_id);
}

FlecsQuery FlecsServer::_get_query(const RID &query_id, const RID &world_id) {
	CHECK_QUERY_VALIDITY_V(query_id, world_id, FlecsQuery(), _get_query);
	return *query;
}

RID FlecsServer::_create_rid_for_query(const RID &world_id, const FlecsQuery &query) {
	CHECK_WORLD_VALIDITY_V(world_id, RID(), _create_rid_for_query);
	return flecs_variant_owners.get(world_id).query_owner.make_rid(query);
}

void FlecsServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_world"), &FlecsServer::create_world);
	ClassDB::bind_method(D_METHOD("get_world_list"), &FlecsServer::get_world_list);
	ClassDB::bind_method(D_METHOD("init_world", "world_id"), &FlecsServer::init_world);
	ClassDB::bind_method(D_METHOD("progress_world", "world_id", "delta"), &FlecsServer::progress_world);
	ClassDB::bind_method(D_METHOD("create_entity", "world_id"), &FlecsServer::create_entity);
	ClassDB::bind_method(D_METHOD("create_entity_with_name", "world_id", "name"), &FlecsServer::create_entity_with_name);
	ClassDB::bind_method(D_METHOD("create_entity_with_name_and_comps", "world_id", "name", "components_type_ids"), &FlecsServer::create_entity_with_name_and_comps);
	ClassDB::bind_method(D_METHOD("lookup", "world_id", "entity_name"), &FlecsServer::lookup);
	ClassDB::bind_method(D_METHOD("get_world_of_entity", "entity_id"), &FlecsServer::get_world_of_entity);
	//all underscore types are not exposed and are only used internally
#ifndef DISABLE_DEPRECATED
	ClassDB::bind_method(D_METHOD("register_component_type", "world_id", "type_name", "script_visible_component_data"), &FlecsServer::register_component_type);
#endif // DISABLE_DEPRECATED
	ClassDB::bind_method(D_METHOD("create_runtime_component", "world_id", "component_name", "fields"), &FlecsServer::create_runtime_component);
	ClassDB::bind_method(D_METHOD("add_component", "entity_id", "component_type_id"), &FlecsServer::add_component);
	ClassDB::bind_method(D_METHOD("has_component", "entity_id", "component_type"), &FlecsServer::has_component);
	ClassDB::bind_method(D_METHOD("get_render_system_command_handler", "world_id"), &FlecsServer::get_render_system_command_handler);
	ClassDB::bind_method(D_METHOD("remove_all_components_from_entity", "entity_id"), &FlecsServer::remove_all_components_from_entity);
	ClassDB::bind_method(D_METHOD("get_component_types_as_name", "entity_id"), &FlecsServer::get_component_types_as_name);
	ClassDB::bind_method(D_METHOD("get_component_types_as_id", "entity_id"), &FlecsServer::get_component_types_as_id);
	ClassDB::bind_method(D_METHOD("get_entity_name", "entity_id"), &FlecsServer::get_entity_name);
	ClassDB::bind_method(D_METHOD("set_entity_name", "entity_id", "name"), &FlecsServer::set_entity_name);
	ClassDB::bind_method(D_METHOD("set_component", "entity_id", "component_name", "comp_data"), &FlecsServer::set_component);
	ClassDB::bind_method(D_METHOD("remove_component_from_entity_with_id", "entity_id", "component_type_id"), &FlecsServer::remove_component_from_entity_with_id);
	ClassDB::bind_method(D_METHOD("remove_component_from_entity_with_name", "entity_id", "component_type"), &FlecsServer::remove_component_from_entity_with_name);
	ClassDB::bind_method(D_METHOD("get_component_by_name", "entity_id", "component_type"), &FlecsServer::get_component_by_name);
	ClassDB::bind_method(D_METHOD("get_component_by_id", "entity_id", "component_type_id"), &FlecsServer::get_component_by_id);
	ClassDB::bind_method(D_METHOD("get_component_type_by_name", "entity_id", "component_type"), &FlecsServer::get_component_type_by_name);
	ClassDB::bind_method(D_METHOD("get_parent", "entity_id"), &FlecsServer::get_parent);
	ClassDB::bind_method(D_METHOD("set_parent", "entity_id", "parent_id"), &FlecsServer::set_parent);
	ClassDB::bind_method(D_METHOD("add_child", "parent_id", "child_id"), &FlecsServer::add_child);
	ClassDB::bind_method(D_METHOD("remove_child", "parent_id", "child_id"), &FlecsServer::remove_child);
	ClassDB::bind_method(D_METHOD("get_children", "parent_id"), &FlecsServer::get_children);
	ClassDB::bind_method(D_METHOD("get_child", "parent_id", "index"), &FlecsServer::get_child);
	ClassDB::bind_method(D_METHOD("add_script_system", "world_id", "component_types", "callable"), &FlecsServer::add_script_system);
	ClassDB::bind_method(D_METHOD("set_script_system_dispatch_mode", "world_id", "script_system_id", "mode"), &FlecsServer::set_script_system_dispatch_mode);
	ClassDB::bind_method(D_METHOD("get_script_system_dispatch_mode", "world_id", "script_system_id"), &FlecsServer::get_script_system_dispatch_mode);
	ClassDB::bind_method(D_METHOD("set_script_system_change_only", "world_id", "script_system_id", "change_only"), &FlecsServer::set_script_system_change_only);
	ClassDB::bind_method(D_METHOD("is_script_system_change_only", "world_id", "script_system_id"), &FlecsServer::is_script_system_change_only);
	ClassDB::bind_method(D_METHOD("set_script_system_instrumentation", "world_id", "script_system_id", "enabled"), &FlecsServer::set_script_system_instrumentation);
	ClassDB::bind_method(D_METHOD("get_script_system_instrumentation", "world_id", "script_system_id"), &FlecsServer::get_script_system_instrumentation);
	ClassDB::bind_method(D_METHOD("reset_script_system_instrumentation", "world_id", "script_system_id"), &FlecsServer::reset_script_system_instrumentation);
	ClassDB::bind_method(D_METHOD("set_script_system_paused", "world_id", "script_system_id", "paused"), &FlecsServer::set_script_system_paused);
	ClassDB::bind_method(D_METHOD("is_script_system_paused", "world_id", "script_system_id"), &FlecsServer::is_script_system_paused);
	ClassDB::bind_method(D_METHOD("set_script_system_dependency", "world_id", "script_system_id", "depends_on"), &FlecsServer::set_script_system_dependency);
	ClassDB::bind_method(D_METHOD("get_all_systems", "world_id"), &FlecsServer::get_all_systems);
	ClassDB::bind_method(D_METHOD("set_script_system_change_observe_add_and_set", "world_id", "script_system_id", "both"), &FlecsServer::set_script_system_change_observe_add_and_set);
	ClassDB::bind_method(D_METHOD("get_script_system_change_observe_add_and_set", "world_id", "script_system_id"), &FlecsServer::get_script_system_change_observe_add_and_set);
	ClassDB::bind_method(D_METHOD("set_script_system_change_observe_remove", "world_id", "script_system_id", "enable"), &FlecsServer::set_script_system_change_observe_remove);
	ClassDB::bind_method(D_METHOD("get_script_system_change_observe_remove", "world_id", "script_system_id"), &FlecsServer::get_script_system_change_observe_remove);
	ClassDB::bind_method(D_METHOD("pause_systems", "world_id", "system_ids"), &FlecsServer::pause_systems);
	ClassDB::bind_method(D_METHOD("resume_systems", "world_id", "system_ids"), &FlecsServer::resume_systems);
	ClassDB::bind_method(D_METHOD("pause_all_systems", "world_id"), &FlecsServer::pause_all_systems);
	ClassDB::bind_method(D_METHOD("resume_all_systems", "world_id"), &FlecsServer::resume_all_systems);
	ClassDB::bind_method(D_METHOD("reset_script_system_instrumentation_action", "world_id", "script_system_id"), &FlecsServer::reset_script_system_instrumentation_action);
	ClassDB::bind_method(D_METHOD("get_world_distribution_summary", "world_id"), &FlecsServer::get_world_distribution_summary);
	ClassDB::bind_method(D_METHOD("get_system_metrics", "world_id"), &FlecsServer::get_system_metrics);
	ClassDB::bind_method(D_METHOD("set_script_system_auto_reset", "world_id", "script_system_id", "auto_reset"), &FlecsServer::set_script_system_auto_reset);
	ClassDB::bind_method(D_METHOD("get_script_system_auto_reset", "world_id", "script_system_id"), &FlecsServer::get_script_system_auto_reset);
	ClassDB::bind_method(D_METHOD("get_world_frame_summary", "world_id"), &FlecsServer::get_world_frame_summary);
	ClassDB::bind_method(D_METHOD("reset_world_frame_summary", "world_id"), &FlecsServer::reset_world_frame_summary);
	ClassDB::bind_method(D_METHOD("set_script_system_detailed_timing", "world_id", "script_system_id", "enabled"), &FlecsServer::set_script_system_detailed_timing);
	ClassDB::bind_method(D_METHOD("get_script_system_detailed_timing", "world_id", "script_system_id"), &FlecsServer::get_script_system_detailed_timing);
	ClassDB::bind_method(D_METHOD("set_script_system_multi_threaded", "world_id", "script_system_id", "enable"), &FlecsServer::set_script_system_multi_threaded);
	ClassDB::bind_method(D_METHOD("get_script_system_multi_threaded", "world_id", "script_system_id"), &FlecsServer::get_script_system_multi_threaded);
	ClassDB::bind_method(D_METHOD("set_script_system_batch_chunk_size", "world_id", "script_system_id", "size"), &FlecsServer::set_script_system_batch_chunk_size);
	ClassDB::bind_method(D_METHOD("get_script_system_batch_chunk_size", "world_id", "script_system_id"), &FlecsServer::get_script_system_batch_chunk_size);
	ClassDB::bind_method(D_METHOD("set_script_system_flush_min_interval_msec", "world_id", "script_system_id", "msec"), &FlecsServer::set_script_system_flush_min_interval_msec);
	ClassDB::bind_method(D_METHOD("get_script_system_flush_min_interval_msec", "world_id", "script_system_id"), &FlecsServer::get_script_system_flush_min_interval_msec);
	ClassDB::bind_method(D_METHOD("set_script_system_use_deferred_calls", "world_id", "script_system_id", "use_deferred"), &FlecsServer::set_script_system_use_deferred_calls);
	ClassDB::bind_method(D_METHOD("get_script_system_use_deferred_calls", "world_id", "script_system_id"), &FlecsServer::get_script_system_use_deferred_calls);
	ClassDB::bind_method(D_METHOD("get_script_system_event_totals", "world_id", "script_system_id"), &FlecsServer::get_script_system_event_totals);
	ClassDB::bind_method(D_METHOD("get_script_system_frame_median_usec", "world_id", "script_system_id"), &FlecsServer::get_script_system_frame_median_usec);
	ClassDB::bind_method(D_METHOD("get_script_system_frame_percentile_usec", "world_id", "script_system_id", "percentile"), &FlecsServer::get_script_system_frame_percentile_usec);
	ClassDB::bind_method(D_METHOD("get_script_system_frame_stddev_usec", "world_id", "script_system_id"), &FlecsServer::get_script_system_frame_stddev_usec);
	ClassDB::bind_method(D_METHOD("get_script_system_frame_p99_usec", "world_id", "script_system_id"), &FlecsServer::get_script_system_frame_p99_usec);
	ClassDB::bind_method(D_METHOD("get_script_system_max_sample_count", "world_id", "script_system_id"), &FlecsServer::get_script_system_max_sample_count);
	ClassDB::bind_method(D_METHOD("set_script_system_max_sample_count", "world_id", "script_system_id", "cap"), &FlecsServer::set_script_system_max_sample_count);
	ClassDB::bind_method(D_METHOD("get_script_system_last_frame_entity_count", "world_id", "script_system_id"), &FlecsServer::get_script_system_last_frame_entity_count);
	ClassDB::bind_method(D_METHOD("get_script_system_last_frame_dispatch_usec", "world_id", "script_system_id"), &FlecsServer::get_script_system_last_frame_dispatch_usec);
	ClassDB::bind_method(D_METHOD("get_script_system_frame_dispatch_invocations", "world_id", "script_system_id"), &FlecsServer::get_script_system_frame_dispatch_invocations);
	ClassDB::bind_method(D_METHOD("get_script_system_frame_dispatch_accum_usec", "world_id", "script_system_id"), &FlecsServer::get_script_system_frame_dispatch_accum_usec);
	ClassDB::bind_method(D_METHOD("get_script_system_frame_min_usec", "world_id", "script_system_id"), &FlecsServer::get_script_system_frame_min_usec);
	ClassDB::bind_method(D_METHOD("get_script_system_frame_max_usec", "world_id", "script_system_id"), &FlecsServer::get_script_system_frame_max_usec);
	ClassDB::bind_method(D_METHOD("get_script_system_last_frame_onadd", "world_id", "script_system_id"), &FlecsServer::get_script_system_last_frame_onadd);
	ClassDB::bind_method(D_METHOD("get_script_system_last_frame_onset", "world_id", "script_system_id"), &FlecsServer::get_script_system_last_frame_onset);
	ClassDB::bind_method(D_METHOD("get_script_system_last_frame_onremove", "world_id", "script_system_id"), &FlecsServer::get_script_system_last_frame_onremove);
	ClassDB::bind_method(D_METHOD("get_script_system_total_callbacks", "world_id", "script_system_id"), &FlecsServer::get_script_system_total_callbacks);
	ClassDB::bind_method(D_METHOD("get_script_system_total_entities_processed", "world_id", "script_system_id"), &FlecsServer::get_script_system_total_entities_processed);
	ClassDB::bind_method(D_METHOD("make_script_system_inspector", "world_id", "script_system_id"), &FlecsServer::make_script_system_inspector);
	ClassDB::bind_method(D_METHOD("get_script_system_info", "world_id", "script_system_id"), &FlecsServer::get_script_system_info);
	ClassDB::bind_method(D_METHOD("get_system_info", "world_id", "system_id"), &FlecsServer::get_system_info);
	ClassDB::bind_method(D_METHOD("set_system_paused", "world_id", "system_id", "paused"), &FlecsServer::set_system_paused);
	ClassDB::bind_method(D_METHOD("is_system_paused", "world_id", "system_id"), &FlecsServer::is_system_paused);
	// Query API bindings
	ClassDB::bind_method(D_METHOD("create_query", "world_id", "required_components"), &FlecsServer::create_query);
	ClassDB::bind_method(D_METHOD("query_get_entities", "world_id", "query_id"), &FlecsServer::query_get_entities);
	ClassDB::bind_method(D_METHOD("query_get_entities_with_components", "world_id", "query_id"), &FlecsServer::query_get_entities_with_components);
	ClassDB::bind_method(D_METHOD("query_get_entity_count", "world_id", "query_id"), &FlecsServer::query_get_entity_count);
	ClassDB::bind_method(D_METHOD("query_get_entities_limited", "world_id", "query_id", "max_count", "offset"), &FlecsServer::query_get_entities_limited);
	ClassDB::bind_method(D_METHOD("query_get_entities_with_components_limited", "world_id", "query_id", "max_count", "offset"), &FlecsServer::query_get_entities_with_components_limited);
	ClassDB::bind_method(D_METHOD("query_matches_entity", "world_id", "query_id", "entity_rid"), &FlecsServer::query_matches_entity);
	ClassDB::bind_method(D_METHOD("query_set_required_components", "world_id", "query_id", "components"), &FlecsServer::query_set_required_components);
	ClassDB::bind_method(D_METHOD("query_get_required_components", "world_id", "query_id"), &FlecsServer::query_get_required_components);
	ClassDB::bind_method(D_METHOD("query_set_caching_strategy", "world_id", "query_id", "strategy"), &FlecsServer::query_set_caching_strategy);
	ClassDB::bind_method(D_METHOD("query_get_caching_strategy", "world_id", "query_id"), &FlecsServer::query_get_caching_strategy);
	ClassDB::bind_method(D_METHOD("query_set_filter_name_pattern", "world_id", "query_id", "pattern"), &FlecsServer::query_set_filter_name_pattern);
	ClassDB::bind_method(D_METHOD("query_get_filter_name_pattern", "world_id", "query_id"), &FlecsServer::query_get_filter_name_pattern);
	ClassDB::bind_method(D_METHOD("query_clear_filter", "world_id", "query_id"), &FlecsServer::query_clear_filter);
	ClassDB::bind_method(D_METHOD("query_force_cache_refresh", "world_id", "query_id"), &FlecsServer::query_force_cache_refresh);
	ClassDB::bind_method(D_METHOD("query_is_cache_dirty", "world_id", "query_id"), &FlecsServer::query_is_cache_dirty);
	ClassDB::bind_method(D_METHOD("query_set_instrumentation_enabled", "world_id", "query_id", "enabled"), &FlecsServer::query_set_instrumentation_enabled);
	ClassDB::bind_method(D_METHOD("query_get_instrumentation_enabled", "world_id", "query_id"), &FlecsServer::query_get_instrumentation_enabled);
	ClassDB::bind_method(D_METHOD("query_get_instrumentation_data", "world_id", "query_id"), &FlecsServer::query_get_instrumentation_data);
	ClassDB::bind_method(D_METHOD("query_reset_instrumentation", "world_id", "query_id"), &FlecsServer::query_reset_instrumentation);
	ClassDB::bind_method(D_METHOD("free_query", "world_id", "query_id"), &FlecsServer::free_query);

	// Script system constants (dispatch modes)
	BIND_ENUM_CONSTANT(DISPATCH_PER_ENTITY);
	BIND_ENUM_CONSTANT(DISPATCH_BATCH);


	ClassDB::bind_method(D_METHOD("set_children", "parent_id", "children"), &FlecsServer::set_children);
	ClassDB::bind_method(D_METHOD("get_child_by_name", "parent_id", "name"), &FlecsServer::get_child_by_name);
	ClassDB::bind_method(D_METHOD("remove_child_by_name", "parent_id", "name"), &FlecsServer::remove_child_by_name);
	ClassDB::bind_method(D_METHOD("remove_child_by_index", "parent_id", "index"), &FlecsServer::remove_child_by_index);
	ClassDB::bind_method(D_METHOD("remove_all_children", "parent_id"), &FlecsServer::remove_all_children);
	ClassDB::bind_method(D_METHOD("add_relationship", "entity_id", "relationship"), &FlecsServer::add_relationship);
	ClassDB::bind_method(D_METHOD("remove_relationship", "entity_id", "relationship"), &FlecsServer::remove_relationship);
	ClassDB::bind_method(D_METHOD("get_relationships", "entity_id"), &FlecsServer::get_relationships);
	ClassDB::bind_method(D_METHOD("get_relationship", "entity_id", "first_entity", "second_entity"), &FlecsServer::get_relationship);
	ClassDB::bind_method(D_METHOD("free_world", "world_id"), &FlecsServer::free_world);
	ClassDB::bind_method(D_METHOD("free_system", "world_id", "system_id", "include_flecs_world"), &FlecsServer::free_system);
	ClassDB::bind_method(D_METHOD("free_script_system", "world_id", "script_system_id"), &FlecsServer::free_script_system);
	ClassDB::bind_method(D_METHOD("free_entity", "world_id", "entity_id", "include_flecs_world"), &FlecsServer::free_entity);
	ClassDB::bind_method(D_METHOD("free_type_id", "world_id", "type_id"), &FlecsServer::free_type_id);
	ClassDB::bind_method(D_METHOD("add_to_ref_storage", "resource", "world_id"), &FlecsServer::add_to_ref_storage);
	ClassDB::bind_method(D_METHOD("remove_from_ref_storage", "resource_rid", "world_id"), &FlecsServer::remove_from_ref_storage);
	ClassDB::bind_method(D_METHOD("get_resource_from_ref_storage", "resource_id", "world_id"), &FlecsServer::get_resource_from_ref_storage);
	ClassDB::bind_method(D_METHOD("add_to_node_storage", "node", "world_id"), &FlecsServer::add_to_node_storage);
	ClassDB::bind_method(D_METHOD("remove_from_node_storage", "node_id", "world_id"), &FlecsServer::remove_from_node_storage);
	ClassDB::bind_method(D_METHOD("get_node_from_node_storage", "node_id", "world_id"), &FlecsServer::get_node_from_node_storage);
	ClassDB::bind_method(D_METHOD("set_world_singleton_with_name", "world_id", "component_type", "component_data"), &FlecsServer::set_world_singleton_with_name);
	ClassDB::bind_method(D_METHOD("set_world_singleton_with_id", "world_id", "comp_type_id", "comp_data"), &FlecsServer::set_world_singleton_with_id);
	ClassDB::bind_method(D_METHOD("get_world_singleton_with_name", "world_id", "name"), &FlecsServer::get_world_singleton_with_name);
	ClassDB::bind_method(D_METHOD("get_world_singleton_with_id", "world_id", "comp_type_id"), &FlecsServer::get_world_singleton_with_id);


	ClassDB::bind_method(D_METHOD("set_script_system_name", "world_id", "script_system_id", "name"), &FlecsServer::set_script_system_name);
	ClassDB::bind_method(D_METHOD("get_script_system_name", "world_id", "script_system_id"), &FlecsServer::get_script_system_name);



	// Debug helpers
	ClassDB::bind_method(D_METHOD("debug_check_rid", "rid"), &FlecsServer::debug_check_rid);

	//ClassDB::bind_static_method(get_class_static(), "get_singleton", &FlecsServer::get_singleton);
}

FlecsServer::FlecsServer() {
	if(!singleton) {
		singleton = this;
	}
	worlds = Vector<RID>();
	worlds.resize(MAX_WORLD_COUNT);
	render_system_command_handler = Ref<CommandHandler>(memnew(CommandHandler));

	command_handler_callback = Callable(render_system_command_handler.ptr(), "process_commands");
	exit_thread = false;
}

FlecsServer::~FlecsServer() {
	singleton = nullptr;
}

RID FlecsServer::create_world() {
	if(counter >= std::numeric_limits<uint8_t>::max()) {
		ERR_PRINT("FlecsServer::create_world: Maximum number of worlds " + itos(std::numeric_limits<uint8_t>::max()) + " reached");
		return RID();
	}
	// Create the FlecsWorldVariant locally first to avoid partially-published
	// state being observed by other threads while we initialize maps.
	FlecsWorldVariant tmp_world{flecs::world()};

	// Lock the FlecsServer to serialize modifications to the owners/maps.
	lock();
	RID flecs_world = flecs_world_owners.make_rid(tmp_world);

	// Ensure the RID is retrievable immediately while holding the lock.
	FlecsWorldVariant *immediate = flecs_world_owners.get_or_null(flecs_world);
	if (!immediate) {
		bool owns = flecs_world_owners.owns(flecs_world);
		uint32_t rid_count = flecs_world_owners.get_rid_count();
		ERR_PRINT("FlecsServer::create_world: make_rid succeeded but get_or_null returned null; owns=" + (owns ? String("true") : String("false")) + ", rid_count=" + itos(rid_count));
		unlock();
		return RID();
	}

	// Use the world reference from the initialized variant
	flecs::world &world_ref = immediate->get_world();


	world_ref.component<Variant>();
	world_ref.component<Dictionary>();
	world_ref.component<Array>();
	world_ref.component<Vector2>();
	world_ref.component<Vector3>();
	world_ref.component<Rect2>();
	world_ref.component<Quaternion>();
	world_ref.component<Plane>();
	world_ref.component<Basis>();
	world_ref.component<Transform2D>();
	world_ref.component<Transform3D>();
	world_ref.component<PackedInt64Array>();
	world_ref.component<PackedInt32Array>();
	world_ref.component<PackedByteArray>();
	world_ref.component<PackedColorArray>();
	world_ref.component<PackedStringArray>();
	world_ref.component<PackedVector2Array>();
	world_ref.component<PackedVector3Array>();
	world_ref.component<PackedVector4Array>();
	world_ref.component<PackedFloat32Array>();
	world_ref.component<PackedFloat64Array>();
	world_ref.component<String>();
	world_ref.component<StringName>();
	world_ref.component<NodePath>();
	world_ref.component<Callable>();
	world_ref.component<Signal>();
	world_ref.component<RID>();


	// Register all components using the new reflection system
	AllComponents::register_all(world_ref, false);



	flecs_variant_owners.insert(flecs_world, RID_Owner_Wrapper{
		flecs_world
	});

	node_storages.insert(flecs_world, memnew(NodeStorage()));
	ref_storages.insert(flecs_world, memnew(RefStorage()));
	// Record the world RID in the worlds vector so _get_world can find it.
	worlds.insert(counter++, flecs_world);

	auto pipeline_manager = PipelineManager();
	pipeline_manager.set_world(flecs_world);
	pipeline_managers.insert(flecs_world, pipeline_manager);

	unlock();

	return flecs_world;
}

void FlecsServer::debug_check_rid(const RID &rid) {
	bool owns = flecs_world_owners.owns(rid);
	uint32_t total = flecs_world_owners.get_rid_count();
	uint64_t id_u64 = rid.get_id();
	char hexbuf2[32];
	snprintf(hexbuf2, sizeof(hexbuf2), "%llx", (unsigned long long)id_u64);
	print_line("debug_check_rid: rid=" + itos(id_u64) + " (hex=0x" + String(hexbuf2) + ", local_index=" + itos(rid.get_local_index()) + "), owns=" + (owns ? String("true") : String("false")) + ", rid_count=" + itos(total));
	print_line("debug_check_rid: worlds vector size=" + itos(worlds.size()));
	const int max_print = 64;
	int printed = 0;
	for (int i = 0; i < worlds.size() && printed < max_print; ++i) {
		const RID &r = worlds[i];
		if (r != RID()) {
			print_line("debug_check_rid: worlds[" + itos(i) + "] -> rid_id=" + itos(r.get_id()));
			++printed;
		}
	}
}

int8_t FlecsServer::get_world_count() const {
	return counter;
}

TypedArray<RID> FlecsServer::get_world_list() const {
	TypedArray<RID> result;
	
	// Iterate through the worlds vector and collect valid RIDs
	for (int i = 0; i < worlds.size(); ++i) {
		const RID &world_rid = worlds[i];
		if (world_rid != RID() && flecs_world_owners.owns(world_rid)) {
			result.append(world_rid);
		}
	}
	
	return result;
}

void FlecsServer::init_world(const RID& world_id) {
	CHECK_WORLD_VALIDITY(world_id, init_world);
	flecs::world &world = world_variant->get_world();
	world.import<flecs::stats>();

	int rest_port = 27750;
	String rest_env = OS::get_singleton()->get_environment("GODOT_FLECS_REST_PORT");
	if (!rest_env.is_empty()) {
		rest_port = rest_env.to_int();
	}
	if (rest_port <= 0) {
		print_line("Flecs REST explorer disabled (GODOT_FLECS_REST_PORT<=0)");
	} else {
		world.set<flecs::Rest>({.port = (uint16_t)rest_port});
		print_line(vformat("Flecs REST explorer available at http://localhost:%d", rest_port));
	}

	print_line("World initialized: " + itos((uint64_t)world.c_ptr()));

	// Configure Flecs to use multiple threads for systems marked with multi_threaded()
	auto threads = std::thread::hardware_concurrency();
	print_line("Detected hardware concurrency: " + itos(threads));
	world.set_threads(threads);
}

bool FlecsServer::progress_world(const RID& world_id, const double delta) {
	// Log the incoming RID and snapshot owner/vector state immediately so we can
	// detect any mismatches that occur when the value is stored in GDScript
	// and later passed back into C++.
	// ERR_PRINT("FlecsServer::progress_world: called with world_id=" + itos(world_id.get_id()));
	// debug_check_rid(world_id);

	flecs::world *world = _get_world(world_id);
	if (!world) {
		ERR_PRINT("FlecsServer::progress_world: world not found");
		return false;
	}

	const bool progress = world->progress(delta);
	// Aggregate per-frame summary: totals across script systems + breakdown
	Dictionary summary;
	uint64_t total_entities = 0; uint64_t total_callbacks_all_time = 0; uint64_t batch_systems = 0;
	uint64_t max_dispatch_usec = 0; uint64_t script_system_count = 0; uint64_t total_dispatch_invocations = 0; uint64_t accum_dispatch_usec = 0;
	Array systems_breakdown;
	for (RID ss_rid : flecs_variant_owners.get(world_id).script_system_owner.get_owned_list()) {
		FlecsScriptSystem *ss = flecs_variant_owners.get(world_id).script_system_owner.get_or_null(ss_rid);
		if (!ss) { continue; }
		++script_system_count;
		uint64_t ent = ss->get_last_frame_entity_count();
		uint64_t last_usec = ss->get_last_frame_dispatch_usec();
		uint64_t inv = ss->get_frame_dispatch_invocations();
		uint64_t accum = ss->get_frame_dispatch_accum_usec();
		total_entities += ent;
		total_callbacks_all_time += ss->get_total_callbacks_invoked();
		total_dispatch_invocations += inv;
		accum_dispatch_usec += accum;
		if (ss->get_dispatch_mode() == FlecsScriptSystem::DISPATCH_BATCH) { batch_systems += 1; }
		if (last_usec > max_dispatch_usec) { max_dispatch_usec = last_usec; }
		Dictionary row;
		row["rid"] = ss_rid;
		row["entities"] = (int64_t)ent;
		row["last_dispatch_usec"] = (int64_t)last_usec;
		row["dispatch_invocations"] = (int64_t)inv;
		row["dispatch_accum_usec"] = (int64_t)accum;
		row["dispatch_avg_usec"] = inv == 0 ? Variant() : Variant((int64_t)(accum / inv));
		row["mode"] = (int64_t)ss->get_dispatch_mode();
		row["min_dispatch_usec"] = (int64_t)ss->get_frame_dispatch_min_usec();
		row["max_dispatch_usec_system"] = (int64_t)ss->get_frame_dispatch_max_usec();
		if (ss->get_detailed_timing_enabled() && ss->get_frame_dispatch_invocations() > 0) {
			row["median_dispatch_usec"] = ss->get_frame_dispatch_median_usec();
			row["p99_dispatch_usec"] = ss->get_frame_dispatch_percentile_usec(99.0);
			row["stddev_dispatch_usec"] = ss->get_frame_dispatch_stddev_usec();
		}
		row["onadd"] = (int64_t)ss->get_last_frame_onadd();
		row["onset"] = (int64_t)ss->get_last_frame_onset();
		row["onremove"] = (int64_t)ss->get_last_frame_onremove();
		systems_breakdown.push_back(row);
	}
	summary["script_systems"] = (int64_t)script_system_count;
	summary["total_entities_this_frame"] = (int64_t)total_entities;
	summary["total_callbacks_all_time"] = (int64_t)total_callbacks_all_time;
	summary["batch_system_count"] = (int64_t)batch_systems;
	summary["max_dispatch_usec"] = (int64_t)max_dispatch_usec;
	summary["dispatch_invocations"] = (int64_t)total_dispatch_invocations;
	summary["dispatch_accum_usec"] = (int64_t)accum_dispatch_usec;
	summary["dispatch_avg_usec"] = total_dispatch_invocations == 0 ? Variant() : Variant((int64_t)(accum_dispatch_usec / total_dispatch_invocations));
	// For simplicity, we don't aggregate median/p99 across systems accurately (would need merge of distributions);
	// could approximate by weighting but omitted for now. Per-system stats above carry detail.
	summary["systems"] = systems_breakdown;
	last_frame_summaries.insert(world_id, summary);

	RS::get_singleton()->call_on_render_thread(command_handler_callback);

	return progress;
}



RID FlecsServer::create_entity(const RID &world_id) {
	CHECK_WORLD_VALIDITY_V(world_id, RID(), create_entity);
	flecs::world &world = world_variant->get_world();
	flecs::entity entity = world.entity();
	RID rid = flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(entity));
	// Add to reverse lookup map for O(1) lookups
	flecs_variant_owners.get(world_id).entity_id_to_rid[entity.id()] = rid;

	return rid;
}

RID FlecsServer::create_entity_with_name(const RID &world_id, const String &p_name) {
	CHECK_WORLD_VALIDITY_V(world_id, RID(), create_entity_with_name);
	RID flecs_entity = create_entity(world_id);
	FlecsEntityVariant *flecs_entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(flecs_entity);
	if (flecs_entity_variant) {
		flecs_entity_variant->get_entity().set_name(p_name.ascii().get_data());
	}
	return flecs_entity;
}

RID FlecsServer::create_entity_with_name_and_comps(const RID& world_id, const String &name, const TypedArray<RID> &components_type_ids) {
	CHECK_WORLD_VALIDITY_V(world_id, RID(), create_entity_with_name);
	RID flecs_entity = create_entity_with_name(world_id,name);
	for(const RID comp_type_id : components_type_ids) {
		FlecsTypeIDVariant *type_id_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(comp_type_id);
		if (type_id_variant) {
			add_component(flecs_entity, comp_type_id);
		} else {
			ERR_PRINT("FlecsServer::create_entity_with_name_and_comp: Component type ID not found");
		}
	}
	return flecs_entity;
}

RID FlecsServer::lookup(const RID &world_id, const String &entity_name) {
	CHECK_WORLD_VALIDITY_V(world_id, RID(), create_entity_with_name);
	if (world_variant) {
		flecs::world &world = world_variant->get_world();
		flecs::entity entity = world.lookup(entity_name.ascii().get_data());
		if (!entity.is_valid()) {
			ERR_PRINT("FlecsServer::lookup: entity not found");
			return RID();
		}
		return flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(entity));
	}
	ERR_FAIL_V_MSG(RID(), "FlecsServer::lookup: world_id is not a valid world");
}

flecs::world *FlecsServer::_get_world(const RID &world_id) {
	// Diagnostic flow: check both the worlds vector and the RID owner so we can
	// print helpful information when an invalid world_id is observed.
	if (!worlds.has(world_id)) {
		FlecsWorldVariant *world_variant = flecs_world_owners.get_or_null(world_id);
		bool owns = flecs_world_owners.owns(world_id);
		uint32_t total = flecs_world_owners.get_rid_count();

		ERR_PRINT("FlecsServer::_get_world: worlds.has returned false for world_id=" + itos(world_id.get_id()) + ", owns=" + (owns ? String("true") : String("false")) + ", rid_count=" + itos(total));
		ERR_PRINT("FlecsServer::_get_world: worlds vector size=" + itos(worlds.size()));

		// Print a compact listing of currently stored world RIDs (avoid huge logs).
		const int max_print = 32;
		int printed = 0;
		for (int i = 0; i < worlds.size() && printed < max_print; ++i) {
			const RID &r = worlds[i];
			if (r != RID()) {
				ERR_PRINT("FlecsServer::_get_world: worlds[" + itos(i) + "] -> rid_id=" + itos(r.get_id()));
				++printed;
			}
		}

		if (world_variant) {
			// Strange: the worlds vector doesn't have the entry but the owner does.
			ERR_PRINT("FlecsServer::_get_world: flecs_world_owners.get_or_null returned a variant despite worlds.has == false; returning its world reference.");
			return &world_variant->get_world();
		}

		ERR_PRINT("FlecsServer::_get_world: world_id is not a valid world");
		return nullptr;
	}

	// If the worlds vector reports the RID exists, try to fetch the stored variant.
	FlecsWorldVariant *world_variant = flecs_world_owners.get_or_null(world_id);
	if (world_variant) {
		return &world_variant->get_world();
	}

	bool owns = flecs_world_owners.owns(world_id);
	uint32_t total = flecs_world_owners.get_rid_count();
	ERR_PRINT("FlecsServer::_get_world: lookup returned null for world_id=" + itos(world_id.get_id()) + ", owns=" + (owns ? String("true") : String("false")) + ", rid_count=" + itos(total));
	ERR_PRINT("FlecsServer::_get_world: available worlds (worlds vector size)=" + itos(worlds.size()));
	return nullptr;

}

RID FlecsServer::get_world_of_entity(const RID &entity_id) {
	for (auto &pair : flecs_variant_owners) {
		FlecsEntityVariant *entity_variant = pair.value.entity_owner.get_or_null(entity_id);
		if (entity_variant) {
			return pair.key;
		}
	}
	ERR_FAIL_V_MSG(RID(), "FlecsServer::get_world_of_entity: entity_id is not a valid entity");
}



void FlecsServer::set_log_level(const int level) {
	flecs::log::set_level(level);
}

#ifndef DISABLE_DEPRECATED
RID FlecsServer::register_component_type(const RID& world_id, const String &type_name, const Dictionary &script_visible_component_data) {
	WARN_PRINT_ONCE("FlecsServer::register_component_type() is deprecated and will be removed in v2.0.0. "
		"Use create_runtime_component() instead, which provides better performance and uses Flecs reflection API. "
		"Old: register_component_type(world_id, name, data) -> New: create_runtime_component(world_id, name, fields)");

	CHECK_WORLD_VALIDITY_V(world_id,RID(), register_component_type);
	const char *ctype_name = String(type_name).ascii().get_data();
	ecs_component_desc_t desc = { 0 };
	flecs::world *world = _get_world(world_id);
	desc.entity = world->entity(ctype_name);
	desc.type.size = sizeof(ScriptVisibleComponent);
	desc.type.alignment = alignof(ScriptVisibleComponent);
	flecs::entity_t comp = ecs_component_init(world->c_ptr(), &desc);
	return flecs_variant_owners.get(world_id).type_id_owner.make_rid(FlecsTypeIDVariant(comp));
}
#endif // DISABLE_DEPRECATED

RID FlecsServer::create_runtime_component(const RID& world_id, const String &component_name, const Dictionary &fields) {
	CHECK_WORLD_VALIDITY_V(world_id, RID(), create_runtime_component);

	flecs::world *world = _get_world(world_id);
	if (!world) {
		ERR_PRINT("FlecsServer::create_runtime_component: Invalid world");
		return RID();
	}

	// Check if component already exists
	flecs::entity existing = world->lookup(component_name.utf8().get_data());
	if (existing.is_valid() && existing.has<EcsComponent>()) {
		ERR_PRINT(vformat("FlecsServer::create_runtime_component: Component '%s' already exists", component_name));
		return RID();
	}

	// Helper to get Flecs type entity for Godot types
	// These types must be registered as opaque types or components beforehand
	// (see FlecsOpaqueTypes::register_opaque_types and AllComponents::register_all)
	auto get_flecs_type = [&](const Variant &value) -> flecs::entity_t {
		switch (value.get_type()) {
			case Variant::BOOL:
				return world->component<bool>();
			case Variant::INT:
				return world->component<int64_t>();
			case Variant::FLOAT:
				return world->component<double>();
			case Variant::STRING:
				return world->component<String>();
			case Variant::STRING_NAME:
				return world->component<StringName>();
			case Variant::VECTOR2:
				return world->component<Vector2>();
			case Variant::VECTOR3:
				return world->component<Vector3>();
			case Variant::QUATERNION:
				return world->component<Quaternion>();
			case Variant::COLOR:
				return world->component<Color>();
			case Variant::RID:
				return world->component<RID>();
			case Variant::ARRAY:
				return world->component<Array>();
			case Variant::DICTIONARY:
				return world->component<Dictionary>();
			case Variant::VECTOR2I:
				return world->component<Vector2i>();
			case Variant::VECTOR3I:
				return world->component<Vector3i>();
			case Variant::VECTOR4:
				return world->component<Vector4>();
			case Variant::VECTOR4I:
				return world->component<Vector4i>();
			case Variant::TRANSFORM2D:
				return world->component<Transform2D>();
			case Variant::TRANSFORM3D:
				return world->component<Transform3D>();
			case Variant::PLANE:
				return world->component<Plane>();
			case Variant::AABB:
				return world->component<AABB>();
			case Variant::BASIS:
				return world->component<Basis>();
			case Variant::PROJECTION:
				return world->component<Projection>();
			case Variant::RECT2:
				return world->component<Rect2>();
			case Variant::RECT2I:
				return world->component<Rect2i>();
			default:
				// Default to Variant for unsupported types
				return world->component<Variant>();
		}
	};

	// Build struct members array
	ecs_member_t members[ECS_MEMBER_DESC_CACHE_SIZE];
	memset(members, 0, sizeof(members));

	Array keys = fields.keys();
	int member_count = MIN(keys.size(), ECS_MEMBER_DESC_CACHE_SIZE);

	// Track member names to keep them alive
	Vector<CharString> member_name_storage;
	member_name_storage.resize(member_count);

	for (int i = 0; i < member_count; i++) {
		String field_name = keys[i];
		Variant field_value = fields[field_name];

		// Store the C string
		member_name_storage.write[i] = field_name.utf8();
		members[i].name = member_name_storage[i].get_data();
		members[i].type = get_flecs_type(field_value);
		members[i].count = 0; // Not an array
		members[i].offset = 0; // Let Flecs calculate offsets
	}

	// Create the struct component with reflection metadata
	// ecs_struct_init automatically provides reflection data for the component,
	// allowing Flecs to:
	// - Open scopes for the component (e.g., for .has<>() checks)
	// - Access member data through the reflection API
	// - Serialize/deserialize component data
	// - Display component info in the Flecs explorer
	// This is equivalent to using .member<>() in the C++ API
	ecs_struct_desc_t struct_desc = {};
	struct_desc.entity = world->entity(component_name.utf8().get_data());
	memcpy(struct_desc.members, members, sizeof(members));

	flecs::entity_t comp_id = ecs_struct_init(world->c_ptr(), &struct_desc);

	if (!comp_id) {
		ERR_PRINT(vformat("FlecsServer::create_runtime_component: Failed to create component '%s'", component_name));
		return RID();
	}

	// Create and return RID for the component type
	return flecs_variant_owners.get(world_id).type_id_owner.make_rid(FlecsTypeIDVariant(comp_id));
}

RID FlecsServer::add_script_system(const RID& world_id, const Array &component_types, const Callable &callable) {
	CHECK_WORLD_VALIDITY_V(world_id, RID(), add_script_system);
	FlecsScriptSystem flecs_script_system;
	flecs_script_system.set_world(world_id);
	PackedStringArray component_names;
	component_names.resize(component_types.size());
	int count = 0;
	for (auto it = component_types.begin(); it != component_types.end(); ++it) {
		component_names.set(count, *it);
		count++;
	}
	flecs_script_system.init(world_id,component_names,callable);
	return flecs_variant_owners.get(world_id).script_system_owner.make_rid(flecs_script_system);
}



PipelineManager* FlecsServer::_get_pipeline_manager(const RID &world_id) {
	CHECK_WORLD_VALIDITY_V(world_id, nullptr, _get_pipeline_manager);
	if (worlds.has(world_id)) {
		auto it = pipeline_managers.find(world_id);
		if (it != pipeline_managers.end()) {
			return &it->value;
		}
	}
	ERR_FAIL_V_MSG(nullptr, "PipelineManager not found for world_id: " + itos(world_id.get_id()));
}

Ref<CommandHandler> FlecsServer::get_render_system_command_handler(const RID &world_id) {
	CHECK_WORLD_VALIDITY_V(world_id, nullptr, get_render_system_command_handler);
	return render_system_command_handler;
}


void FlecsServer::remove_all_components_from_entity(const RID &entity_id) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::remove_all_components_from_entity: world_id is not valid");
		return;
	}
	CHECK_ENTITY_VALIDITY(entity_id, world_id, remove_all_components_from_entity);
	flecs::entity entity = entity_variant->get_entity();
	entity.clear(); // Clear all components from the entity
}


Dictionary FlecsServer::get_component_by_name(const RID &entity_id, const String &component_type)  {
	Dictionary component_data;
	
	// Skip pair/relationship format strings - they can't be looked up by name
	// These are formatted as "(First, Second)" from get_component_types_as_name
	if (component_type.begins_with("(")) {
		// Pairs don't have serializable component data in the same way
		return component_data;
	}
	
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::get_component_by_name: world_id is not valid");
		return component_data;
	}
	
	FlecsEntityVariant *entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		
		// Validate entity is still valid and alive in Flecs world
		if (!entity.is_valid() || !entity.is_alive()) {
			ERR_PRINT("FlecsServer::get_component_by_name: entity is no longer valid/alive in Flecs world");
			return component_data;
		}
		
		flecs::entity component = entity.world().lookup(component_type.ascii().get_data());
		if (!component.is_valid()) {
			ERR_PRINT("FlecsServer::get_component_by_name: component type not found: " + component_type);
			return component_data;
		}

		// Use cursor-based conversion for all types
		Dictionary result = component_to_dict_cursor(entity, component.id());
		return result;
	}
	ERR_PRINT("FlecsServer::get_component_by_name: entity_id is not a valid entity");
	return component_data;
}
bool FlecsServer::has_component(const RID& entity_id, const String &component_type) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::has_component: world_id is not valid");
		return false;
	}
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		flecs::entity comp_type = entity.world().component(String(component_type).ascii().get_data());
		return entity.has(comp_type);
	}
	ERR_PRINT("FlecsServer::has_component: entity_id is not a valid entity");
	return false;
}


PackedStringArray FlecsServer::get_component_types_as_name(const RID &entity_id) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::get_component_types_as_name: world_id is not valid");
		return PackedStringArray();
	}
	flecs::world *world = _get_world(world_id);
	if (!world) {
		ERR_PRINT("FlecsServer::get_component_types_as_name: world is null");
		return PackedStringArray();
	}
	
	// Validate world pointer is usable
	ecs_world_t *raw_world = const_cast<ecs_world_t *>(world->c_ptr());
	if (!raw_world) {
		ERR_PRINT("FlecsServer::get_component_types_as_name: raw_world is null");
		return PackedStringArray();
	}
	
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		// Validate entity is still alive in Flecs world before iterating
		if (!entity.is_valid() || !entity.is_alive()) {
			ERR_PRINT("FlecsServer::get_component_types_as_name: entity is no longer valid/alive in Flecs world");
			return PackedStringArray();
		}
		
		// Double-check with raw Flecs API
		ecs_entity_t raw_entity = entity.id();
		if (!ecs_is_alive(raw_world, raw_entity)) {
			ERR_PRINT("FlecsServer::get_component_types_as_name: entity not alive according to ecs_is_alive");
			return PackedStringArray();
		}
		
		PackedStringArray component_types;
		
		// Use try-catch style safety with a flag to detect issues
		bool iteration_error = false;
		
		// Enter readonly mode to safely iterate over entity components
		const bool multi_threaded = ecs_get_stage_count(raw_world) > 1;
		ecs_readonly_begin(raw_world, multi_threaded);
		
		entity.each([&](flecs::id type) {
			// Early exit if we've already had an error
			if (iteration_error) {
				return;
			}
			
			// Defensive: check type validity
			ecs_id_t raw_id = type.raw_id();
			if (raw_id == 0) {
				return; // Skip invalid IDs
			}
			
			// Skip pairs and relationships - they can cause issues with component()
			if (type.is_pair()) {
				// For pairs, try to get a readable name safely
				ecs_entity_t first_id = ecs_pair_first(raw_world, raw_id);
				ecs_entity_t second_id = ecs_pair_second(raw_world, raw_id);
				
				// Validate both parts of the pair exist
				if (first_id == 0 || second_id == 0) {
					return;
				}
				
				if (!ecs_is_alive(raw_world, first_id) || !ecs_is_alive(raw_world, second_id)) {
					return;
				}
				
				flecs::entity first = type.first();
				flecs::entity second = type.second();
				if (first.is_valid() && second.is_valid()) {
					flecs::string_view first_name = first.name();
					flecs::string_view second_name = second.name();
					const char* first_cstr = first_name.c_str();
					const char* second_cstr = second_name.c_str();
					if (first_cstr && second_cstr && first_cstr[0] != '\0' && second_cstr[0] != '\0') {
						String pair_name = String("(") + String(first_cstr) + ", " + String(second_cstr) + ")";
						component_types.push_back(pair_name);
					}
				}
				return;
			}
			
			// For regular components, validate the entity ID first
			if (!ecs_is_alive(raw_world, raw_id)) {
				return; // Skip IDs that don't correspond to alive entities
			}
			
			// For regular components, get the entity directly
			flecs::entity comp = world->entity(raw_id);
			if (!comp.is_valid()) {
				return; // Skip invalid component entities
			}
			
			flecs::string_view name_view = comp.name();
			const char* name = name_view.c_str();
			if (name && name[0] != '\0') {
				component_types.push_back(String(name));
			}
		});
		
		ecs_readonly_end(raw_world);
		
		return component_types;
	}
	ERR_PRINT("FlecsServer::get_component_types_as_name: entity_id is not a valid entity");
	return PackedStringArray();
}

TypedArray<RID> FlecsServer::get_component_types_as_id(const RID &entity_id) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::get_component_types_as_id: world_id is not valid");
		return TypedArray<RID>();
	}
	TypedArray<RID> component_ids;
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		// Validate entity is still alive in Flecs world before iterating
		if (!entity.is_valid() || !entity.is_alive()) {
			ERR_PRINT("FlecsServer::get_component_types_as_id: entity is no longer valid/alive in Flecs world");
			return TypedArray<RID>();
		}
		entity.each([&](flecs::id type) {
			if (type.raw_id() != 0) {
				component_ids.push_back(flecs_variant_owners.get(world_id).type_id_owner.make_rid(FlecsTypeIDVariant(type)));
			}
		});
		return component_ids;
	}
	ERR_PRINT("FlecsServer::get_component_types_as_id: entity_id is not a valid entity");
	return TypedArray<RID>();
}

String FlecsServer::get_entity_name(const RID &entity_id) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::get_entity_name: world_id is not valid");
		return String();
	}
	
	// Get world and validate raw pointer
	flecs::world *world = _get_world(world_id);
	if (!world) {
		ERR_PRINT("FlecsServer::get_entity_name: world is null");
		return String();
	}
	
	ecs_world_t *raw_world = const_cast<ecs_world_t *>(world->c_ptr());
	if (!raw_world) {
		ERR_PRINT("FlecsServer::get_entity_name: raw_world is null");
		return String();
	}
	
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		// Validate entity is still alive before accessing name
		if (!entity.is_valid() || !entity.is_alive()) {
			ERR_PRINT("FlecsServer::get_entity_name: entity is no longer valid/alive in Flecs world");
			return String();
		}
		
		// Double-check with raw Flecs API
		ecs_entity_t raw_entity = entity.id();
		if (!ecs_is_alive(raw_world, raw_entity)) {
			ERR_PRINT("FlecsServer::get_entity_name: entity not alive according to ecs_is_alive");
			return String();
		}
		
		// Enter readonly mode to safely access entity name
		const bool multi_threaded = ecs_get_stage_count(raw_world) > 1;
		ecs_readonly_begin(raw_world, multi_threaded);
		
		// Keep string_view alive while we use the pointer
		flecs::string_view name_view = entity.name();
		const char* name = name_view.c_str();
		String result;
		if (name && name[0] != '\0') {
			result = String(name);
		}
		
		ecs_readonly_end(raw_world);
		return result;
	}
	ERR_PRINT("FlecsServer::get_entity_name: entity_id is not a valid entity");
	return "ERROR";
}
 void FlecsServer::set_entity_name(const RID& entity_id, const String &p_name) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::set_component: world_id is not valid");
		return;
	}
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		entity.set_name(p_name.ascii().get_data());
	} else {
		ERR_PRINT("FlecsServer::set_entity_name: entity_id is not a valid entity");
	}
}

void FlecsServer::set_component(const RID& entity_id, const String& component_type, const Dictionary &comp_data) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::set_component: world_id is not valid");
		return;
	}
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		flecs::entity comp_type = entity.world().component(component_type.ascii().get_data());
		if (comp_type.is_valid()) {
			// Use cursor-based conversion for all types
			component_from_dict_cursor(entity, comp_type.id(), comp_data);
		} else {
			ERR_PRINT("FlecsServer::set_component: component type not found: " + component_type);
		}
	} else {
		ERR_PRINT("FlecsServer::set_component: entity_id is not a valid entity");
	}
}

void FlecsServer::remove_component_from_entity_with_id(const RID &entity_id, const RID &component_id) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::remove_component_from_entity_with_id: world_id is not valid");
		return;
	}
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		flecs::entity_t comp_id = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(component_id)->get_type();
		if (comp_id) {
			entity.remove(comp_id);
		}
	} else {
		ERR_PRINT("FlecsServer::remove_component_from_entity_with_id: entity_id is not a valid entity");
	}
}

void FlecsServer::remove_component_from_entity_with_name(const RID &entity_id, const String &component_type) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::remove_component_from_entity_with_name: world_id is not valid");
		return;
	}
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		flecs::entity_t comp_type = entity.world().component(component_type.ascii().get_data()).id();
		if (comp_type) {
			entity.remove(comp_type);
		}
	} else {
		ERR_PRINT("FlecsServer::remove_component_from_entity_with_name: entity_id is not a valid entity");
	}
}

Dictionary FlecsServer::get_component_by_id(const RID& entity_id, const RID& component_type_id) {

	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::get_component_by_id: world_id is not valid");
		return Dictionary();
	}
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		FlecsTypeIDVariant* comp_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(component_type_id);
		if(comp_variant){
			flecs::entity_t comp_id = comp_variant->get_type();
			if (comp_id) {
				// Use cursor-based conversion for all types
				return component_to_dict_cursor(entity, comp_id);
			}
		}
	}
	ERR_PRINT("FlecsServer::get_component_by_id: entity_id or component_type_id is not valid");
	return Dictionary();
}

RID FlecsServer::get_component_type_by_name(const RID& entity_id, const String &component_type) {
	bool is_world = flecs_world_owners.owns(entity_id);
	RID world_id = is_world ? entity_id : get_world_of_entity(entity_id);
	bool is_entity = false;
	if(!is_world){
		is_entity = flecs_variant_owners.get(world_id).entity_owner.owns(entity_id);
	}
	if(is_entity){
		CHECK_ENTITY_VALIDITY_V(entity_id, world_id, RID(), get_component_type_by_name)
		flecs::entity comp_type;
		comp_type = entity.world().component(component_type.ascii().get_data());
		if (comp_type.is_valid()) {
			return flecs_variant_owners.get(world_id).type_id_owner.make_rid(FlecsTypeIDVariant(comp_type.id()));
		}
		ERR_FAIL_V_MSG(RID(), "Component type not found: " + component_type);
	}else if(is_world){
		CHECK_WORLD_VALIDITY_V(world_id, RID(), get_component_type_by_name)
		flecs::world &world = world_variant->get_world();
		flecs::entity comp_type = world.component(String(component_type).ascii().get_data());
		if (comp_type.is_valid()) {
			return flecs_variant_owners.get(world_id).type_id_owner.make_rid(FlecsTypeIDVariant(comp_type.id()));
		}
		ERR_FAIL_V_MSG(RID(), "Component type not found: " + component_type);
	}else if (!is_world && !is_entity){
		ERR_PRINT("FlecsServer::get_component_type_by_name: id is not valid");
		return RID();
	}
	else{
		CRASH_COND_MSG(is_world == true && is_entity == true, "is_world == true && is_entity == true, this should not happen.");
		return RID();
	}

}

RID FlecsServer::get_parent(const RID& entity_id) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* flecs_entity = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (flecs_entity) {
		flecs::entity entity = flecs_entity->get_entity();
		flecs::entity parent = entity.parent();
		if (parent.is_valid()) {
			return flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(parent));
		}
	}
	ERR_FAIL_V_MSG(RID(), "Parent not found for entity_id: " + itos(entity_id.get_id()));
}

void FlecsServer::set_parent(const RID& entity_id, const RID& parent_id) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* flecs_entity = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	FlecsEntityVariant* parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	if (flecs_entity && parent_variant) {
		flecs::entity entity = flecs_entity->get_entity();
		flecs::entity parent = parent_variant->get_entity();
		entity.add(flecs::ChildOf, parent);
	} else {
		ERR_PRINT("FlecsServer::set_parent: entity_id or parent_id is not a valid entity");
	}
}



RID FlecsServer::get_child(const RID& entity_id, int index) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* flecs_entity = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (flecs_entity) {
		flecs::entity entity = flecs_entity->get_entity();
		int i = 0;
		flecs::entity child;
		entity.children([&](flecs::entity c) {
			if (i == index) {
				child = c;
			}
			i++;
		});
		if (child.is_valid()) {
			return flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(child));
		}
	}
	ERR_FAIL_V_MSG(RID(), "Child not found for entity_id: " + itos(entity_id.get_id()) + " at index: " + itos(index));
}

void FlecsServer::set_children(const RID &parent_id, const TypedArray<RID> &p_children) {
	// Clear existing children.
	remove_all_children(parent_id);
	//Add new children.
	for (int i = 0; i < p_children.size(); i++) {
		RID child_id_value = p_children[i];
		add_child(parent_id, child_id_value);
	}
}

RID FlecsServer::get_child_by_name(const RID &parent_id,const String &name){
	RID world_id = get_world_of_entity(parent_id);
	FlecsEntityVariant* parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	if (parent_variant) {
		flecs::entity parent = parent_variant->get_entity();
		RID child_rid;
		parent.children([&](flecs::entity child) {
			if (child.name() == name.ascii().get_data()) {
				child_rid = flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(child));
			}
		});
		return child_rid;
	}
	ERR_FAIL_V_MSG(RID(), "Child not found for parent_id: " + itos(parent_id.get_id()) + " with name: " + name);
}

void FlecsServer::remove_child_by_name(const RID &parent_id, const String &name){
	RID world_id = get_world_of_entity(parent_id);
	FlecsEntityVariant* parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	if (parent_variant) {
		flecs::entity parent = parent_variant->get_entity();
		parent.children([&](flecs::entity child) {
			if (child.name() == name.ascii().get_data()) {
				child.remove(flecs::ChildOf);
			}
		});
	} else {
		ERR_PRINT("FlecsServer::remove_child_by_name: parent_id is not a valid entity");
	}

}
void FlecsServer::remove_child_by_index(const RID &parent_id, int index) {
	RID world_id = get_world_of_entity(parent_id);
	FlecsEntityVariant* parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	if (parent_variant) {
		flecs::entity parent = parent_variant->get_entity();
		int i = 0;
		parent.children([&](flecs::entity child) {
			if (i == index) {
				child.remove(flecs::ChildOf);
			}
			i++;
		});
	} else {
		ERR_PRINT("FlecsServer::remove_child_by_index: parent_id is not a valid entity");
	}
}

void FlecsServer::remove_all_children(const RID &parent_id) {
	RID world_id = get_world_of_entity(parent_id);
	FlecsEntityVariant* parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	if (parent_variant) {
		flecs::entity parent = parent_variant->get_entity();
		parent.children([&](flecs::entity child) {
			child.remove(flecs::ChildOf);
		});
	} else {
		ERR_PRINT("FlecsServer::remove_all_children: parent_id is not a valid entity");
	}
}

void FlecsServer::add_child(const RID &parent_id, const RID& child_id) {
	// Implementation for adding a child entity
	RID world_id = get_world_of_entity(parent_id);
	FlecsEntityVariant* parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	FlecsEntityVariant* child_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(child_id);
	if (parent_variant && child_variant) {
		flecs::entity parent = parent_variant->get_entity();
		flecs::entity child = child_variant->get_entity();
		child.add(flecs::ChildOf, parent);
		return;
	}
	ERR_PRINT("FlecsServer::add_child: parent or child entity not found");
}

void FlecsServer::remove_child(const RID& parent_id, const RID &child_id) {
	RID world_id = get_world_of_entity(parent_id);
	FlecsEntityVariant* parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	FlecsEntityVariant* child_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(child_id);
	if (parent_variant && child_variant) {
		flecs::entity parent = parent_variant->get_entity();
		flecs::entity child = child_variant->get_entity();
		if(child.parent() != parent) {
			ERR_PRINT("FlecsServer::remove_child: child is not a child of the specified parent");
			return;
		}
		child.remove(flecs::ChildOf);
		return;
	}
	ERR_PRINT("FlecsServer::remove_child: parent or child entity not found");
}

TypedArray<RID> FlecsServer::get_children(const RID &parent_id) {
	TypedArray<RID> child_array;
	RID world_id = get_world_of_entity(parent_id);
	FlecsEntityVariant *parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	if (parent_variant) {
		flecs::entity parent = parent_variant->get_entity();
		parent.children([&](flecs::entity child) {
			child_array.push_back(flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(child)).get_id());
		});
	}
	return child_array;
}

void FlecsServer::add_component(const RID& entity_id, const RID& component_id) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	FlecsTypeIDVariant* type_id_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(component_id);
	if (entity_variant && type_id_variant) {
		flecs::entity entity = entity_variant->get_entity();
		flecs::entity component_type = entity.world().component(type_id_variant->get_type());
		if (component_type.is_valid()) {
			entity.add(component_type);
		} else {
			ERR_PRINT("FlecsServer::add_component: component_type is not valid");
		}
	} else {
		ERR_PRINT("FlecsServer::add_component: entity_id or component_id is not valid");
	}

}

void FlecsServer::add_relationship(const RID& entity_id, const RID &relationship) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	FlecsEntityVariant* relationship_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(relationship);
	if (entity_variant && relationship_variant) {
		flecs::entity entity = entity_variant->get_entity();
		flecs::entity rel_entity = relationship_variant->get_entity();
		entity.add(rel_entity);
	} else {
		ERR_PRINT("FlecsServer::add_relationship: entity_id or relationship is not valid");
	}
}

void FlecsServer::remove_relationship(const RID& entity_id, const RID &relationship) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	FlecsEntityVariant* relationship_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(relationship);
	if (entity_variant && relationship_variant) {
		flecs::entity entity = entity_variant->get_entity();
		flecs::entity rel_entity = relationship_variant->get_entity();
		entity.remove(rel_entity);
	} else {
		ERR_PRINT("FlecsServer::remove_relationship: entity_id or relationship is not valid");
	}

}

RID FlecsServer::get_relationship(const RID &entity_id, const String& first_entity, const String& second_entity) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (!entity_variant) {
		ERR_PRINT("FlecsServer::get_relationship: entity_id is not valid");
		return RID();
	}
	flecs::entity entity = entity_variant->get_entity();
	flecs::entity first = entity.world().component(first_entity.ascii().get_data());
	flecs::entity second = entity.world().component(second_entity.ascii().get_data());
	if (!first.is_valid() || !second.is_valid()) {
		ERR_PRINT("FlecsServer::get_relationship: first or second entity is not valid");
		return RID();
	}
	if(!entity.has(first,second)){
		ERR_PRINT("FlecsServer::get_relationship: entity does not have the relationship between " + first_entity + " and " + second_entity);
		return RID();
	}
	const flecs::entity_t* rel_entity = static_cast<const flecs::entity_t*>(entity.get(first, second));
	if (!rel_entity) {
		ERR_PRINT("FlecsServer::get_relationship: relationship is not valid");
		return RID();
	}
	return flecs_variant_owners.get(world_id).type_id_owner.make_rid(FlecsTypeIDVariant(*rel_entity));
}

TypedArray<RID> FlecsServer::get_relationships(const RID &entity_id) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (!entity_variant) {
		ERR_PRINT("FlecsServer::get_relationships: entity_id is not valid");
		return TypedArray<RID>();
	}
	flecs::entity entity = entity_variant->get_entity();
	TypedArray<RID> relationships;
	Vector<flecs::entity_t> relationship_ids;
	for (const RID& rid : flecs_variant_owners.get(world_id).type_id_owner.get_owned_list()) {
			FlecsTypeIDVariant* type_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(rid);
			if(!type_variant) {
				continue;
			}
			flecs::entity_t type_id = type_variant->get_type();
			if(type_id == 0) {
				continue;
			}
			if(entity.has(type_id)) {
				relationships.append(rid);
				relationship_ids.push_back(type_id);
			}
		}

	entity.children([&](flecs::entity child) {
		if(!child.is_pair()){
			return;
		}
		if(!relationship_ids.has(child.id())) {
			return;
		}
		relationships.append(_create_rid_for_type_id(world_id, child.id()));
	});

	return relationships;
}

RID FlecsServer::_create_rid_for_entity(const RID& world_id, const flecs::entity &entity) {
	return flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(entity));
}

RID FlecsServer::_create_rid_for_system(const RID& world_id, const flecs::system &system) {
	return flecs_variant_owners.get(world_id).system_owner.make_rid(FlecsSystemVariant(system));
}

RID FlecsServer::_get_rid_for_world(const flecs::world *world) {
	if (!world) {
		ERR_PRINT("FlecsServer::_get_rid_for_world: world is null");
		return RID();
	}
	for(auto it = worlds.begin(); it != worlds.end(); ++it) {
		if (_get_world(*it)->c_ptr() == world->c_ptr()) {
			return *it;
		}
	}
	ERR_PRINT("FlecsServer::_get_rid_for_world: world not found");
	return RID();
}

RID FlecsServer::_create_rid_for_type_id(const RID& world_id, const flecs::entity_t &type_id) {
	return flecs_variant_owners.get(world_id).type_id_owner.make_rid(FlecsTypeIDVariant(type_id));
}

RID FlecsServer::_create_rid_for_script_system(const RID& world_id, const FlecsScriptSystem &system) {
	return flecs_variant_owners.get(world_id).script_system_owner.make_rid(system);
}

void FlecsServer::free_world(const RID& rid) {
	if (flecs_world_owners.owns(rid)) {
			for (const RID& owned : flecs_variant_owners.get(rid).entity_owner.get_owned_list()) {
				flecs_variant_owners.get(rid).entity_owner.free(owned);
			}

			for (const RID& owned : flecs_variant_owners.get(rid).type_id_owner.get_owned_list()) {
				flecs_variant_owners.get(rid).type_id_owner.free(owned);
			}

		flecs_variant_owners.get(rid).system_owner.get_owned_list();
		for (const RID& owned : flecs_variant_owners.get(rid).system_owner.get_owned_list()) {
			flecs_variant_owners.get(rid).system_owner.free(owned);
		}

		for (const RID& owned : flecs_variant_owners.get(rid).script_system_owner.get_owned_list()) {
			flecs_variant_owners.get(rid).script_system_owner.free(owned);
		}
		flecs_variant_owners.erase(rid);

		worlds.erase(rid);
		flecs_world_owners.free(rid);

		pipeline_managers.erase(rid);

		if (node_storages.has(rid)) {
			memdelete(node_storages.get(rid));
			node_storages.erase(rid);
		}
		if (ref_storages.has(rid)) {
			memdelete(ref_storages.get(rid));
			ref_storages.erase(rid);
		}
		return;
	}

}

void FlecsServer::free_system(const RID& world_id, const RID& system_id, const bool include_flecs_world) {
	if (flecs_variant_owners.has(world_id)) {
		if (include_flecs_world) {
			FlecsSystemVariant* system_variant = flecs_variant_owners.get(world_id).system_owner.get_or_null(system_id);
			system_variant->get_system().destruct();
		}
		flecs_variant_owners.get(world_id).system_owner.free(system_id);
	} else {
		ERR_PRINT("FlecsServer::free_system: world_id is not a valid world");
	}
}

void FlecsServer::free_script_system(const RID& world_id, const RID& script_system_id) {
	if (flecs_variant_owners.has(world_id)) {
		flecs_variant_owners.get(world_id).script_system_owner.free(script_system_id);
	} else {
		ERR_PRINT("FlecsServer::free_script_system: world_id is not a valid world");
	}
}

void FlecsServer::free_entity(const RID& world_id, const RID& entity_id, bool include_flecs_world) {
	if (flecs_variant_owners.has(world_id)) {
		FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
		if (entity_variant) {
			// Remove from reverse lookup map before freeing
			flecs::entity entity = entity_variant->get_entity();
			if (entity.is_valid()) {
				flecs_variant_owners.get(world_id).entity_id_to_rid.erase(entity.id());
			}
			if (include_flecs_world) {
				entity.destruct();
			}
		} else {
			ERR_PRINT("FlecsServer::free_entity: entity_id is not a valid entity");
		}
		flecs_variant_owners.get(world_id).entity_owner.free(entity_id);
	} else {
		ERR_PRINT("FlecsServer::free_entity: world_id is not a valid world");
	}
}

flecs::entity FlecsServer::_get_entity(const RID& entity_id, const RID& world_id) {
	CHECK_ENTITY_VALIDITY_V(entity_id, world_id, flecs::entity(), _get_entity);
	return entity;
}

void FlecsServer::free_type_id(const RID& world_id, const RID& type_id) {
	if (flecs_variant_owners.has(world_id)) {
		flecs_variant_owners.get(world_id).type_id_owner.free(type_id);
	} else {
		ERR_PRINT("FlecsServer::free_type_id: world_id is not a valid world");
	}
}

void FlecsServer::add_to_ref_storage(const Ref<Resource> &resource, const RID &world_id) {
	if (ref_storages.has(world_id)) {
		ref_storages.get(world_id)->add(resource, resource->get_rid());
	} else {
		ERR_PRINT("FlecsServer::add_to_ref_storage: world_id is not a valid world");
	}
}

void FlecsServer::remove_from_ref_storage(const RID &resource_rid, const RID &world_id) {
	if (ref_storages.has(world_id)) {
		ref_storages.get(world_id)->release(resource_rid);
	} else {
		ERR_PRINT("FlecsServer::remove_from_ref_storage: world_id is not a valid world");
	}
}

void FlecsServer::add_to_node_storage(Node *node, const RID &world_id) {
	if (node_storages.has(world_id)) {
		node_storages.get(world_id)->add(node, node->get_instance_id());
	} else {
		ERR_PRINT("FlecsServer::add_to_node_storage: world_id is not a valid world");
	}
}

void FlecsServer::remove_from_node_storage(const int64_t node_id, const RID &world_id) {
	if (node_storages.has(world_id)) {
		node_storages.get(world_id)->release(ObjectID(node_id));
	} else {
		ERR_PRINT("FlecsServer::remove_from_node_storage: world_id is not a valid world");
	}
}

Ref<Resource> FlecsServer::get_resource_from_ref_storage(const RID &resource_rid, const RID &world_id) {
	if (ref_storages.has(world_id)) {
		RefContainer* ref_storage = ref_storages.get(world_id)->get(resource_rid);
		if(ref_storage){
			return ref_storage->resource;
		}
	}
	ERR_PRINT("FlecsServer::get_resource_from_ref_storage: world_id is not a valid world");
	return Ref<Resource>();

}

Node* FlecsServer::get_node_from_node_storage(const int64_t node_id, const RID &world_id) {
	if (node_storages.has(world_id)) {
		NodeContainer* node_storage = node_storages.get(world_id)->try_get(ObjectID(node_id));
		if (node_storage) {
			return node_storage->node;
		}
		ERR_PRINT("FlecsServer::get_node_from_node_storage: Node not found in storage for node_id: " + itos(node_id));
		return nullptr;
	}
	ERR_PRINT("FlecsServer::get_node_from_node_storage: world_id is not a valid world");
	return nullptr;
}

RID FlecsServer::_get_or_create_rid_for_entity(const RID &world_id, const flecs::entity &entity) {
	// Early validation of entity
	if (!entity.is_valid() || !entity.is_alive()) {
		ERR_PRINT("FlecsServer::_get_or_create_rid_for_entity: entity is not valid or not alive");
		return RID();
	}
	
	if (flecs_variant_owners.has(world_id)) {
		uint64_t entity_id = entity.id();
		
		// O(1) lookup using reverse map
		if (flecs_variant_owners.get(world_id).entity_id_to_rid.has(entity_id)) {
			RID existing_rid = flecs_variant_owners.get(world_id).entity_id_to_rid[entity_id];
			// Verify the RID is still valid
			FlecsEntityVariant* owned_entity = flecs_variant_owners.get(world_id).entity_owner.get_or_null(existing_rid);
			if (owned_entity) {
				flecs::entity owned_flecs_entity = owned_entity->get_entity();
				if (owned_flecs_entity.is_valid() && owned_flecs_entity.id() == entity_id) {
					return existing_rid;
				}
			}
			// RID was stale, remove from map
			flecs_variant_owners.get(world_id).entity_id_to_rid.erase(entity_id);
		}
		
		// Entity not found in existing RIDs, create new one
		RID new_rid = flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(entity));
		// Add to reverse lookup map
		flecs_variant_owners.get(world_id).entity_id_to_rid[entity_id] = new_rid;
		return new_rid;
	} else {
		ERR_PRINT("FlecsServer::_get_or_create_rid_for_entity: world_id is not a valid world");
		return RID();
	}
}

flecs::system FlecsServer::_get_system(const RID &system_id, const RID &world_id) {
	CHECK_SYSTEM_VALIDITY_V(system_id, world_id, flecs::system(), _get_system);
	flecs::system system = system_variant->get_system();
	return system;
}

FlecsScriptSystem FlecsServer::_get_script_system(const RID &script_system_id, const RID &world_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, FlecsScriptSystem(), _get_script_system);
	return *script_system;
}

flecs::entity_t FlecsServer::_get_type_id(const RID &entity_id, const RID &world_id) {
	CHECK_TYPE_ID_VALIDITY_V(entity_id, world_id, flecs::entity_t(), _get_type_id);
	flecs::entity_t type_id = type_id_variant->get_type();
	return type_id;
}

void FlecsServer::set_world_singleton_with_name(const RID &world_id, const String& comp_type, const Dictionary& comp_data){
	RID comp_type_id = get_component_type_by_name(world_id, comp_type);
	if (!comp_type_id.is_valid()) {
		ERR_PRINT("FlecsServer::set_world_singleton_with_name: Component type not found: " + comp_type);
		return;
	}
	set_world_singleton_with_id(world_id, comp_type_id, comp_data);
}
void FlecsServer::set_world_singleton_with_id(const RID &world_id, const RID &comp_type_id, const Dictionary& comp_data){
	CHECK_WORLD_VALIDITY(world_id, set_world_singleton_with_id);
	FlecsTypeIDVariant* type_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(comp_type_id);
	if (!type_variant) {
		ERR_PRINT("FlecsServer::set_world_singleton_with_id: Component type ID not found: " + itos(comp_type_id.get_id()));
		return;
	}
	flecs::entity_t comp_type = type_variant->get_type();
	if (!comp_type) {
		ERR_PRINT("FlecsServer::set_world_singleton_with_id: Component type is not valid");
		return;
	}
	flecs::world &world = world_variant->get_world();

	// In Flecs, singletons are stored on the component entity itself
	flecs::entity comp_entity(world.c_ptr(), comp_type);

	// Use cursor-based conversion for world singletons
	component_from_dict_cursor(comp_entity, comp_type, comp_data);
}
Dictionary FlecsServer::get_world_singleton_with_name(const RID &world_id, const String& comp_type){
	CHECK_WORLD_VALIDITY_V(world_id, Dictionary(), get_world_singleton_with_name);
	RID comp_type_id = get_component_type_by_name(world_id, comp_type);
	if (!comp_type_id.is_valid()) {
		ERR_PRINT("FlecsServer::get_world_singleton_with_name: Component type not found: " + comp_type);
		return Dictionary();
	}
	return get_world_singleton_with_id(world_id, comp_type_id);
}
Dictionary FlecsServer::get_world_singleton_with_id(const RID &world_id, const RID &comp_type_id){
	CHECK_WORLD_VALIDITY_V(world_id, Dictionary(), get_world_singleton_with_id);
	FlecsTypeIDVariant* type_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(comp_type_id);
	if (!type_variant) {
		ERR_PRINT("FlecsServer::get_world_singleton_with_id: Component type ID not found: " + itos(comp_type_id.get_id()));
		return Dictionary();
	}
	flecs::world &world = world_variant->get_world();
	flecs::entity comp_type = world.component(type_variant->get_type());
	if (!comp_type.is_valid()) {
		ERR_PRINT("FlecsServer::get_world_singleton_with_id: Component type is not valid");
		return Dictionary();
	}

	// In Flecs, singletons are stored on the component entity itself
	flecs::entity comp_entity(world.c_ptr(), comp_type);

	// For world singletons, get the component data from the component entity
	return component_to_dict_cursor(comp_entity, comp_type);
}

void FlecsServer::set_script_system_dispatch_mode(const RID &world_id, const RID &script_system_id, int mode) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, set_script_system_dispatch_mode);
	if (mode < 0 || mode > 1) { ERR_PRINT("Invalid dispatch mode"); return; }
	script_system->set_dispatch_mode(static_cast<FlecsScriptSystem::DispatchMode>(mode));
}

int FlecsServer::get_script_system_dispatch_mode(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, -1, get_script_system_dispatch_mode);
	return (int)script_system->get_dispatch_mode();
}

void FlecsServer::set_script_system_change_only(const RID &world_id, const RID &script_system_id, bool change_only) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, set_script_system_change_only);
	script_system->set_change_only(change_only);
}

bool FlecsServer::is_script_system_change_only(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, false, is_script_system_change_only);
	return script_system->is_change_only();
}

void FlecsServer::set_script_system_instrumentation(const RID &world_id, const RID &script_system_id, bool enabled) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, set_script_system_instrumentation);
	script_system->set_instrumentation_enabled(enabled);
}

Dictionary FlecsServer::get_script_system_instrumentation(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, Dictionary(), get_script_system_instrumentation);
	Dictionary d;
	d["last_frame_entity_count"] = (int64_t)script_system->get_last_frame_entity_count();
	d["last_frame_batch_size"] = (int64_t)script_system->get_last_frame_batch_size();
	d["last_frame_dispatch_usec"] = (int64_t)script_system->get_last_frame_dispatch_usec();
	d["total_entities_processed"] = (int64_t)script_system->get_total_entities_processed();
	d["total_callbacks_invoked"] = (int64_t)script_system->get_total_callbacks_invoked();
	d["change_only"] = script_system->is_change_only();
	d["dispatch_mode"] = (int64_t)script_system->get_dispatch_mode();
	return d;
}

void FlecsServer::reset_script_system_instrumentation(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, reset_script_system_instrumentation);
	script_system->reset_instrumentation();
}

void FlecsServer::set_script_system_paused(const RID &world_id, const RID &script_system_id, bool paused) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, set_script_system_paused);
	script_system->set_is_paused(paused);
}

bool FlecsServer::is_script_system_paused(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, false, is_script_system_paused);
	return script_system->get_is_paused();
}

void FlecsServer::set_script_system_dependency(const RID &world_id, const RID &script_system_id, uint32_t dep_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, set_script_system_dependency);
	script_system->set_system_dependency(dep_id);
}

Dictionary FlecsServer::get_all_systems(const RID &world_id) {
	CHECK_WORLD_VALIDITY_V(world_id, Dictionary(), get_all_systems);
	Dictionary result;
	Array cpp_list; Array script_list;
	// C++ systems
	for (RID rid : flecs_variant_owners.get(world_id).system_owner.get_owned_list()) {
		FlecsSystemVariant *sv = flecs_variant_owners.get(world_id).system_owner.get_or_null(rid);
		if (!sv) { continue; }
		flecs::system sys = sv->get_system();
		Dictionary d; d["rid"] = rid; d["name"] = String("cpp_system_") + itos((int64_t)sys.id()); d["depends_on"] = Variant(); d["type"] = String("cpp");
		cpp_list.push_back(d);
	}
	// Script systems
	for (RID rid : flecs_variant_owners.get(world_id).script_system_owner.get_owned_list()) {
		FlecsScriptSystem *ss = flecs_variant_owners.get(world_id).script_system_owner.get_or_null(rid);
		if (!ss) { continue; }
		Dictionary d; d["rid"] = rid; d["name"] = String("ScriptSystem#") + itos(ss->get_system_id());
		uint32_t dep = ss->get_system_dependency_id();
		d["depends_on"] = dep == 0 ? Variant() : Variant((int64_t)dep);
		d["type"] = String("script");
		d["change_only"] = ss->is_change_only();
		d["observe_add_and_set"] = ss->get_change_observe_add_and_set();
		d["observe_remove"] = ss->get_change_observe_remove();
		d["auto_reset"] = ss->get_auto_reset_per_frame();
		d["dispatch_mode"] = (int64_t)ss->get_dispatch_mode();
		script_list.push_back(d);
	}
	result["cpp"] = cpp_list;
	result["script"] = script_list;
	return result;
}

void FlecsServer::set_script_system_change_observe_add_and_set(const RID &world_id, const RID &script_system_id, bool both) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, set_script_system_change_observe_add_and_set);
	script_system->set_change_observe_add_and_set(both);
}

bool FlecsServer::get_script_system_change_observe_add_and_set(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, false, get_script_system_change_observe_add_and_set);
	return script_system->get_change_observe_add_and_set();
}

void FlecsServer::set_script_system_auto_reset(const RID &world_id, const RID &script_system_id, bool auto_reset) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, set_script_system_auto_reset);
	script_system->set_auto_reset_per_frame(auto_reset);
}

bool FlecsServer::get_script_system_auto_reset(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, false, get_script_system_auto_reset);
	return script_system->get_auto_reset_per_frame();
}

Dictionary FlecsServer::get_world_frame_summary(const RID &world_id) {
	CHECK_WORLD_VALIDITY_V(world_id, Dictionary(), get_world_frame_summary);
	Dictionary *found = last_frame_summaries.getptr(world_id);
	if (!found) { return Dictionary(); }
	return *found;
}

void FlecsServer::reset_world_frame_summary(const RID &world_id) {
	CHECK_WORLD_VALIDITY(world_id, reset_world_frame_summary);
	last_frame_summaries.erase(world_id);
}

void FlecsServer::set_script_system_detailed_timing(const RID &world_id, const RID &script_system_id, bool enabled) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, set_script_system_detailed_timing);
	script_system->set_detailed_timing_enabled(enabled);
}

bool FlecsServer::get_script_system_detailed_timing(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, false, get_script_system_detailed_timing);
	return script_system->get_detailed_timing_enabled();
}

Dictionary FlecsServer::get_script_system_event_totals(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, Dictionary(), get_script_system_event_totals);
	Dictionary d;
	d["onadd_total"] = script_system->get_total_onadd();
	d["onset_total"] = script_system->get_total_onset();
	d["onremove_total"] = script_system->get_total_onremove();
	return d;
}

double FlecsServer::get_script_system_frame_median_usec(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0.0, get_script_system_frame_median_usec);
	return script_system->get_frame_dispatch_median_usec();
}

double FlecsServer::get_script_system_frame_percentile_usec(const RID &world_id, const RID &script_system_id, double percentile) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0.0, get_script_system_frame_percentile_usec);
	return script_system->get_frame_dispatch_percentile_usec(percentile);
}

double FlecsServer::get_script_system_frame_stddev_usec(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0.0, get_script_system_frame_stddev_usec);
	return script_system->get_frame_dispatch_stddev_usec();
}

int FlecsServer::get_script_system_max_sample_count(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0, get_script_system_max_sample_count);
	return script_system->get_max_sample_count();
}

void FlecsServer::set_script_system_max_sample_count(const RID &world_id, const RID &script_system_id, int cap) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, set_script_system_max_sample_count);
	script_system->set_max_sample_count(cap);
}

int FlecsServer::get_script_system_last_frame_entity_count(const RID &world_id, const RID &script_system_id) { CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0, get_script_system_last_frame_entity_count); return (int)script_system->get_last_frame_entity_count(); }
uint64_t FlecsServer::get_script_system_last_frame_dispatch_usec(const RID &world_id, const RID &script_system_id) { CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0, get_script_system_last_frame_dispatch_usec); return script_system->get_last_frame_dispatch_usec(); }
uint64_t FlecsServer::get_script_system_frame_dispatch_invocations(const RID &world_id, const RID &script_system_id) { CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0, get_script_system_frame_dispatch_invocations); return script_system->get_frame_dispatch_invocations(); }
uint64_t FlecsServer::get_script_system_frame_dispatch_accum_usec(const RID &world_id, const RID &script_system_id) { CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0, get_script_system_frame_dispatch_accum_usec); return script_system->get_frame_dispatch_accum_usec(); }
uint64_t FlecsServer::get_script_system_frame_min_usec(const RID &world_id, const RID &script_system_id) { CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0, get_script_system_frame_min_usec); return script_system->get_frame_dispatch_min_usec(); }
uint64_t FlecsServer::get_script_system_frame_max_usec(const RID &world_id, const RID &script_system_id) { CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0, get_script_system_frame_max_usec); return script_system->get_frame_dispatch_max_usec(); }
uint64_t FlecsServer::get_script_system_last_frame_onadd(const RID &world_id, const RID &script_system_id) { CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0, get_script_system_last_frame_onadd); return script_system->get_last_frame_onadd(); }
uint64_t FlecsServer::get_script_system_last_frame_onset(const RID &world_id, const RID &script_system_id) { CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0, get_script_system_last_frame_onset); return script_system->get_last_frame_onset(); }
uint64_t FlecsServer::get_script_system_last_frame_onremove(const RID &world_id, const RID &script_system_id) { CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0, get_script_system_last_frame_onremove); return script_system->get_last_frame_onremove(); }
uint64_t FlecsServer::get_script_system_total_callbacks(const RID &world_id, const RID &script_system_id) { CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0, get_script_system_total_callbacks); return script_system->get_total_callbacks_invoked(); }
uint64_t FlecsServer::get_script_system_total_entities_processed(const RID &world_id, const RID &script_system_id) { CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0, get_script_system_total_entities_processed); return script_system->get_total_entities_processed(); }

Ref<Resource> FlecsServer::make_script_system_inspector(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, Ref<Resource>(), make_script_system_inspector);
	Ref<ScriptSystemInspector> insp;
	insp.instantiate();
	insp->_set_context(world_id, script_system_id, this);
	return insp;
}

Dictionary FlecsServer::get_script_system_info(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, Dictionary(), get_script_system_info);
	Dictionary d;
	d["dispatch_mode"] = (int64_t)script_system->get_dispatch_mode();
	d["change_only"] = script_system->is_change_only();
	d["observe_add_and_set"] = script_system->get_change_observe_add_and_set();
	d["observe_remove"] = script_system->get_change_observe_remove();
	d["instrumentation_enabled"] = script_system->get_instrumentation_enabled();
	d["detailed_timing_enabled"] = script_system->get_detailed_timing_enabled();
	d["auto_reset_per_frame"] = script_system->get_auto_reset_per_frame();
	d["paused"] = script_system->get_is_paused();
	d["last_frame_entity_count"] = (int64_t)script_system->get_last_frame_entity_count();
	d["last_frame_dispatch_usec"] = (int64_t)script_system->get_last_frame_dispatch_usec();
	d["frame_dispatch_invocations"] = (int64_t)script_system->get_frame_dispatch_invocations();
	d["frame_dispatch_accum_usec"] = (int64_t)script_system->get_frame_dispatch_accum_usec();
	d["frame_dispatch_min_usec"] = (int64_t)script_system->get_frame_dispatch_min_usec();
	d["frame_dispatch_max_usec"] = (int64_t)script_system->get_frame_dispatch_max_usec();
	if (script_system->get_detailed_timing_enabled() && script_system->get_frame_dispatch_invocations() > 0) {
		d["frame_dispatch_median_usec"] = script_system->get_frame_dispatch_median_usec();
		d["frame_dispatch_p99_usec"] = script_system->get_frame_dispatch_percentile_usec(99.0);
		d["frame_dispatch_stddev_usec"] = script_system->get_frame_dispatch_stddev_usec();
	}
	d["last_frame_onadd"] = (int64_t)script_system->get_last_frame_onadd();
	d["last_frame_onset"] = (int64_t)script_system->get_last_frame_onset();
	d["last_frame_onremove"] = (int64_t)script_system->get_last_frame_onremove();
	d["total_callbacks_invoked"] = (int64_t)script_system->get_total_callbacks_invoked();
	d["total_entities_processed"] = (int64_t)script_system->get_total_entities_processed();
	return d;
}

Dictionary FlecsServer::get_system_info(const RID &world_id, const RID &system_id) {
	CHECK_WORLD_VALIDITY_V(world_id, Dictionary(), get_system_info);
	FlecsWorldVariant *wv = flecs_world_owners.get_or_null(world_id);
	ERR_FAIL_NULL_V(wv, Dictionary());
	flecs::world &w = wv->get_world();
	flecs::entity e = w.entity(system_id.get_id());
	if (!e.is_valid()) { ERR_PRINT("get_system_info: invalid system entity"); return Dictionary(); }
	Dictionary d;
	d["id"] = (int64_t)system_id.get_id();
	// Keep string_view alive while we use the pointer
	flecs::string_view name_view = e.name();
	d["name"] = String(name_view.c_str());
	// Regular system pause via Disabled tag
	flecs::entity disabled = w.lookup("flecs.core.Disabled");
	bool paused_state = disabled.is_valid() ? e.has(disabled) : false;
	d["paused"] = paused_state;
	// Extended: attempt to extract phase/terms (best-effort)
	// Phase: look for relationship 'flecs.core.Phase'
	// Phase detection (best effort)
	flecs::entity phase_rel = w.lookup("flecs.core.Phase");
	if (phase_rel.is_valid()) { /* future: inspect relationships */ }
	// Placeholder: terms enumeration not directly available without stored query; skip for now.
	return d;
}

void FlecsServer::set_system_paused(const RID &world_id, const RID &system_id, bool paused) {
	CHECK_WORLD_VALIDITY(world_id, set_system_paused);
	FlecsWorldVariant *wv = flecs_world_owners.get_or_null(world_id);
	if (!wv) { return; }
	flecs::world &w = wv->get_world();
	flecs::entity e = w.entity(system_id.get_id());
	if (!e.is_valid()) { ERR_PRINT("set_system_paused: invalid system"); return; }
	if (paused) { e.disable(); } else { e.enable(); }
}

bool FlecsServer::is_system_paused(const RID &world_id, const RID &system_id) {
	CHECK_WORLD_VALIDITY_V(world_id, false, is_system_paused);
	FlecsWorldVariant *wv = flecs_world_owners.get_or_null(world_id);
	ERR_FAIL_NULL_V(wv, false);
	flecs::world &w = wv->get_world();
	flecs::entity e = w.entity(system_id.get_id());
	if (!e.is_valid()) { ERR_PRINT("is_system_paused: invalid system"); return false; }
	flecs::entity disabled = w.lookup("flecs.core.Disabled");
	return disabled.is_valid() ? e.has(disabled) : false;
}

void FlecsServer::pause_systems(const RID &world_id, const PackedInt64Array &system_ids) {
	CHECK_WORLD_VALIDITY(world_id, pause_systems);
	FlecsWorldVariant *wv = flecs_world_owners.get_or_null(world_id); if (!wv) { return; }
	flecs::world &w = wv->get_world();
	for (int i = 0; i < system_ids.size(); ++i) {
		uint64_t raw = (uint64_t)system_ids[i];
		flecs::entity e = w.entity(raw);
		if (e.is_valid()) {
			e.disable();
			regular_system_paused.insert(RID::from_uint64(raw), true);
		}
	}
}

void FlecsServer::set_script_system_change_observe_remove(const RID &world_id, const RID &script_system_id, bool observe_remove) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, set_script_system_change_observe_remove);
	script_system->set_change_observe_remove(observe_remove);
}

bool FlecsServer::get_script_system_change_observe_remove(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, false, get_script_system_change_observe_remove);
	return script_system->get_change_observe_remove();
}

void FlecsServer::resume_systems(const RID &world_id, const PackedInt64Array &system_ids) {
	CHECK_WORLD_VALIDITY(world_id, resume_systems);
	FlecsWorldVariant *wv = flecs_world_owners.get_or_null(world_id); if (!wv) { return; }
	flecs::world &w = wv->get_world();
	for (int i = 0; i < system_ids.size(); ++i) {
		uint64_t raw = (uint64_t)system_ids[i];
		flecs::entity e = w.entity(raw);
		if (e.is_valid()) {
			e.enable();
			regular_system_paused.insert(RID::from_uint64(raw), false);
		}
	}
}

void FlecsServer::pause_all_systems(const RID &world_id) {
	CHECK_WORLD_VALIDITY(world_id, pause_all_systems);
	FlecsWorldVariant *wv = flecs_world_owners.get_or_null(world_id); if (!wv) { return; }
	flecs::world &w = wv->get_world();
	w.each([](flecs::entity e){ if (e.is_valid()) { e.disable(); }});
}

void FlecsServer::resume_all_systems(const RID &world_id) {
	CHECK_WORLD_VALIDITY(world_id, resume_all_systems);
	FlecsWorldVariant *wv = flecs_world_owners.get_or_null(world_id); if (!wv) { return; }
	flecs::world &w = wv->get_world();
	w.each([](flecs::entity e){ if (e.is_valid()) { e.enable(); }});
}

Dictionary FlecsServer::get_world_distribution_summary(const RID &world_id) {
	CHECK_WORLD_VALIDITY_V(world_id, Dictionary(), get_world_distribution_summary);
	Dictionary result;
	FlecsWorldVariant *wv = flecs_world_owners.get_or_null(world_id); if (!wv) { return result; }
	// Build merged sample approximation by concatenating samples (potentially large; cap)
	Vector<double> merged;
	int total_invocations = 0;
	const int MERGE_CAP = 4096;
	for (RID ss_rid : flecs_variant_owners.get(world_id).script_system_owner.get_owned_list()) {
		FlecsScriptSystem *ss = flecs_variant_owners.get(world_id).script_system_owner.get_or_null(ss_rid);
		if (!ss || !ss->get_detailed_timing_enabled()) { continue; }
		const Vector<uint64_t> &samples = ss->_get_frame_dispatch_samples();
		for (int i = 0; i < samples.size() && merged.size() < MERGE_CAP; ++i) {
			merged.push_back((double)samples[i]);
		}
		total_invocations += ss->get_frame_dispatch_invocations();
		if (merged.size() >= MERGE_CAP) { break; }
	}
	if (merged.is_empty()) { return result; }
	merged.sort();
	auto percentile = [&](double p) {
		if (merged.is_empty()) { return 0.0; }
		if (p <= 0.0) { return merged[0]; }
		if (p >= 100.0) { return merged[merged.size() - 1]; }
		double r = (p / 100.0) * (merged.size() - 1);
		int lo = (int)r;
		int hi = MIN(lo + 1, merged.size() - 1);
		double f = r - (double)lo;
		return merged[lo] + (merged[hi] - merged[lo]) * f;
	};
	double median = percentile(50.0);
	double p99 = percentile(99.0);
	// compute stddev
	double sum = 0;
	for (int i = 0; i < merged.size(); ++i) { sum += merged[i]; }
	double mean = sum / merged.size();
	double var = 0;
	for (int i = 0; i < merged.size(); ++i) { double d = merged[i] - mean; var += d * d; }
	var /= (merged.size()>1 ? (merged.size()-1) : 1);
	result["median_usec"] = median;
	result["p99_usec"] = p99;
	result["mean_usec"] = mean;
	result["stddev_usec"] = Math::sqrt(var);
	result["samples_used"] = merged.size();
	result["total_invocations"] = total_invocations;
	result["approximation_cap"] = MERGE_CAP;
	return result;
}

Dictionary FlecsServer::get_system_metrics(const RID &world_id) {
	// Silent check - world may be destroyed during shutdown while profiler is still running
	FlecsWorldVariant* world_variant = flecs_world_owners.get_or_null(world_id);
	if (!world_variant) {
		return Dictionary();
	}
	
	Dictionary result;
	Array systems_array;
	uint64_t total_time_usec = 0;
	
	// Track which system entity IDs we've already added (to avoid duplicates)
	HashSet<uint64_t> added_system_ids;
	
	// Collect metrics for script systems
	for (RID ss_rid : flecs_variant_owners.get(world_id).script_system_owner.get_owned_list()) {
		FlecsScriptSystem *ss = flecs_variant_owners.get(world_id).script_system_owner.get_or_null(ss_rid);
		if (!ss) { continue; }
		
		// Track this system's entity ID if it has one
		if (ss->get_system_id() != 0) {
			added_system_ids.insert(ss->get_system_id());
		}
		
		Dictionary sys_metric;
		sys_metric["rid"] = ss_rid;
		sys_metric["name"] = ss->get_system_name().is_empty() ? 
			String("ScriptSystem#") + itos(ss->get_system_id()) : 
			ss->get_system_name();
		sys_metric["type"] = "script";
		
		// Timing metrics
		uint64_t last_usec = ss->get_last_frame_dispatch_usec();
		uint64_t invocations = ss->get_frame_dispatch_invocations();
		uint64_t accum_usec = ss->get_frame_dispatch_accum_usec();
		
		sys_metric["time_usec"] = (int64_t)last_usec;
		sys_metric["call_count"] = (int64_t)invocations;
		sys_metric["total_time_usec"] = (int64_t)accum_usec;
		sys_metric["avg_time_usec"] = invocations > 0 ? (int64_t)(accum_usec / invocations) : 0;
		sys_metric["min_time_usec"] = (int64_t)ss->get_frame_dispatch_min_usec();
		sys_metric["max_time_usec"] = (int64_t)ss->get_frame_dispatch_max_usec();
		
		total_time_usec += last_usec;
		
		// Entity metrics
		sys_metric["entity_count"] = (int64_t)ss->get_last_frame_entity_count();
		
		// Detailed timing if enabled
		if (ss->get_detailed_timing_enabled() && invocations > 0) {
			sys_metric["median_usec"] = ss->get_frame_dispatch_median_usec();
			sys_metric["p99_usec"] = ss->get_frame_dispatch_percentile_usec(99.0);
			sys_metric["stddev_usec"] = ss->get_frame_dispatch_stddev_usec();
		}
		
		// Event counts
		sys_metric["onadd_count"] = (int64_t)ss->get_last_frame_onadd();
		sys_metric["onset_count"] = (int64_t)ss->get_last_frame_onset();
		sys_metric["onremove_count"] = (int64_t)ss->get_last_frame_onremove();
		
		// State flags
		sys_metric["paused"] = ss->get_is_paused();
		sys_metric["dispatch_mode"] = (int64_t)ss->get_dispatch_mode();
		
		// Lifetime stats
		sys_metric["total_callbacks"] = (int64_t)ss->get_total_callbacks_invoked();
		sys_metric["total_entities_processed"] = (int64_t)ss->get_total_entities_processed();
		
		systems_array.push_back(sys_metric);
	}
	
	// Collect metrics for registered C++ systems
	// Get world pointer for stats queries
	flecs::world* world_ptr = &world_variant->get_world();
	
	for (RID sys_rid : flecs_variant_owners.get(world_id).system_owner.get_owned_list()) {
		FlecsSystemVariant *sv = flecs_variant_owners.get(world_id).system_owner.get_or_null(sys_rid);
		if (!sv) { continue; }
		
		flecs::system sys = sv->get_system();
		if (!sys.is_valid()) { continue; }
		
		// Track this system's entity ID
		added_system_ids.insert(sys.id());
		
		Dictionary sys_metric;
		sys_metric["rid"] = sys_rid;
		
		// Get system name - keep string_view alive while we use the pointer
		flecs::string_view name_view = sys.name();
		const char* name = name_view.c_str();
		sys_metric["name"] = name ? String(name) : String("cpp_system_") + itos((int64_t)sys.id());
		sys_metric["type"] = "cpp";
		
		// Try to get raw system data first (more reliable than stats API for cumulative time)
		const ecs_system_t* sys_ptr = ecs_system_get(world_ptr->c_ptr(), sys.id());
		if (sys_ptr) {
			// time_spent on ecs_system_t is cumulative time in seconds
			double raw_time_sec = sys_ptr->time_spent;
			uint64_t raw_time_usec = (uint64_t)(raw_time_sec * 1000000.0);
			sys_metric["time_usec"] = (int64_t)raw_time_usec;
			sys_metric["total_time_usec"] = (int64_t)raw_time_usec; // Cumulative
			
			// Also try to get stats for min/max/entity count
			ecs_system_stats_t stats = {};
			bool has_stats = ecs_system_stats_get(world_ptr->c_ptr(), sys.id(), &stats);
			
			if (has_stats) {
				int32_t t = stats.query.t;
				if (t < 0 || t >= ECS_STAT_WINDOW) {
					t = 0;
				}
				sys_metric["min_time_usec"] = (int64_t)(stats.time_spent.gauge.min[t] * 1000000.0);
				sys_metric["max_time_usec"] = (int64_t)(stats.time_spent.gauge.max[t] * 1000000.0);
				sys_metric["entity_count"] = (int64_t)stats.query.matched_entity_count.gauge.avg[t];
			} else {
				sys_metric["min_time_usec"] = 0;
				sys_metric["max_time_usec"] = 0;
				sys_metric["entity_count"] = 0;
			}
			
			total_time_usec += raw_time_usec;
		} else {
			sys_metric["time_usec"] = 0;
			sys_metric["total_time_usec"] = 0;
			sys_metric["min_time_usec"] = 0;
			sys_metric["max_time_usec"] = 0;
			sys_metric["entity_count"] = 0;
		}
		
		sys_metric["call_count"] = 0; // Not directly available from ecs_system_stats_t
		
		// State flags
		bool paused_state = false;
		if (regular_system_paused.has(sys_rid)) {
			paused_state = regular_system_paused[sys_rid];
		}
		sys_metric["paused"] = paused_state;
		
		systems_array.push_back(sys_metric);
	}
	
	// Query Flecs directly for ALL system entities (including those not registered with system_owner)
	// This catches systems created directly via world->system<>() like BadAppleSystem's systems
	// Note: world_variant and world_ptr are already defined above
	if (world_ptr) {
		{
			// Query for all entities with EcsSystem component (the System tag)
			world_ptr->each([&](flecs::entity e) {
				// Check if this entity is a system (has System component)
				if (!e.has(flecs::System)) {
					return;
				}
				
				// Skip if we've already added this system
				if (added_system_ids.has(e.id())) {
					return;
				}
				
				// Skip built-in Flecs systems (timer systems, etc.) - they have ChildOf relationship to flecs modules
				// We only want user-created systems. Check if it's a child of flecs.* modules
				flecs::string path_str_flecs = e.path(); // flecs::string is owned, keep it alive
				const char* path = path_str_flecs.c_str();
				if (path) {
					String path_str(path);
					// Skip internal flecs systems (flecs.timer.*, flecs.pipeline.*, etc.)
					if (path_str.begins_with("::flecs.")) {
						return;
					}
				}
			
				Dictionary sys_metric;
				sys_metric["rid"] = RID(); // No RID for unregistered systems
				sys_metric["entity_id"] = (int64_t)e.id(); // Provide entity ID for reference
				
				// Get system name
				flecs::string_view name_view = e.name();
				const char* name = name_view.c_str();
				sys_metric["name"] = name ? String(name) : String("system_") + itos((int64_t)e.id());
				sys_metric["type"] = "native"; // Distinguish from "cpp" (registered) and "script"
				
				// Try to get raw system data first (more reliable than stats API for cumulative time)
				const ecs_system_t* sys_ptr = ecs_system_get(world_ptr->c_ptr(), e.id());
				if (sys_ptr) {
					// time_spent on ecs_system_t is cumulative time in seconds
					double raw_time_sec = sys_ptr->time_spent;
					uint64_t raw_time_usec = (uint64_t)(raw_time_sec * 1000000.0);
					sys_metric["time_usec"] = (int64_t)raw_time_usec;
					sys_metric["total_time_usec"] = (int64_t)raw_time_usec; // Cumulative
					
					// Also try to get stats for min/max/entity count
					ecs_system_stats_t stats = {};
					bool has_stats = ecs_system_stats_get(world_ptr->c_ptr(), e.id(), &stats);
					
					if (has_stats) {
						int32_t t = stats.query.t;
						if (t < 0 || t >= ECS_STAT_WINDOW) {
							t = 0;
						}
						sys_metric["min_time_usec"] = (int64_t)(stats.time_spent.gauge.min[t] * 1000000.0);
						sys_metric["max_time_usec"] = (int64_t)(stats.time_spent.gauge.max[t] * 1000000.0);
						sys_metric["entity_count"] = (int64_t)stats.query.matched_entity_count.gauge.avg[t];
					} else {
						sys_metric["min_time_usec"] = 0;
						sys_metric["max_time_usec"] = 0;
						sys_metric["entity_count"] = 0;
					}
					
					total_time_usec += raw_time_usec;
				} else {
					sys_metric["time_usec"] = 0;
					sys_metric["total_time_usec"] = 0;
					sys_metric["min_time_usec"] = 0;
					sys_metric["max_time_usec"] = 0;
					sys_metric["entity_count"] = 0;
				}
				
				sys_metric["call_count"] = 0; // Not directly available from ecs_system_stats_t
				sys_metric["paused"] = false;
			
				systems_array.push_back(sys_metric);
				added_system_ids.insert(e.id());
			});
		}
	}
	
	result["systems"] = systems_array;
	result["system_count"] = (int64_t)systems_array.size();
	result["total_time_usec"] = (int64_t)total_time_usec;
	result["frame_count"] = Engine::get_singleton()->get_frames_drawn();
	
	return result;
}

void FlecsServer::set_script_system_name(const RID &world_id, const RID &script_system_id, const String &name) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, set_script_system_name);
	script_system->set_system_name(name);
}

String FlecsServer::get_script_system_name(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, String(), get_script_system_name);
	return script_system->get_system_name();
}

void FlecsServer::set_script_system_multi_threaded(const RID &world_id, const RID &script_system_id, bool enable) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, set_script_system_multi_threaded);
	script_system->set_multi_threaded(enable);
}

bool FlecsServer::get_script_system_multi_threaded(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, false, get_script_system_multi_threaded);
	return script_system->get_multi_threaded();
}

void FlecsServer::set_script_system_batch_chunk_size(const RID &world_id, const RID &script_system_id, int chunk_size) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, set_script_system_batch_chunk_size);
	script_system->set_batch_flush_chunk_size(chunk_size);
}

int FlecsServer::get_script_system_batch_chunk_size(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0, get_script_system_batch_chunk_size);
	return script_system->get_batch_flush_chunk_size();
}

void FlecsServer::set_script_system_flush_min_interval_msec(const RID &world_id, const RID &script_system_id, double msec) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, set_script_system_flush_min_interval_msec);
	script_system->set_flush_min_interval_msec(msec);
}

double FlecsServer::get_script_system_flush_min_interval_msec(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, 0.0, get_script_system_flush_min_interval_msec);
	return script_system->get_flush_min_interval_msec();
}

void FlecsServer::set_script_system_use_deferred_calls(const RID &world_id, const RID &script_system_id, bool use_deferred) {
	CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, set_script_system_use_deferred_calls);
	script_system->set_use_deferred_calls(use_deferred);
}

bool FlecsServer::get_script_system_use_deferred_calls(const RID &world_id, const RID &script_system_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, false, get_script_system_use_deferred_calls);
	return script_system->get_use_deferred_calls();
}


// ---- ScriptSystemInspector implementation ----
void ScriptSystemInspector::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_dispatch_mode"), &ScriptSystemInspector::get_dispatch_mode);
	ClassDB::bind_method(D_METHOD("set_dispatch_mode", "mode"), &ScriptSystemInspector::set_dispatch_mode);
	ClassDB::bind_method(D_METHOD("get_change_only"), &ScriptSystemInspector::get_change_only);
	ClassDB::bind_method(D_METHOD("set_change_only", "v"), &ScriptSystemInspector::set_change_only);
	ClassDB::bind_method(D_METHOD("get_observe_add_and_set"), &ScriptSystemInspector::get_observe_add_and_set);
	ClassDB::bind_method(D_METHOD("set_observe_add_and_set", "v"), &ScriptSystemInspector::set_observe_add_and_set);
	ClassDB::bind_method(D_METHOD("get_observe_remove"), &ScriptSystemInspector::get_observe_remove);
	ClassDB::bind_method(D_METHOD("set_observe_remove", "v"), &ScriptSystemInspector::set_observe_remove);
	ClassDB::bind_method(D_METHOD("get_instrumentation"), &ScriptSystemInspector::get_instrumentation);
	ClassDB::bind_method(D_METHOD("set_instrumentation", "v"), &ScriptSystemInspector::set_instrumentation);
	ClassDB::bind_method(D_METHOD("get_detailed_timing"), &ScriptSystemInspector::get_detailed_timing);
	ClassDB::bind_method(D_METHOD("set_detailed_timing", "v"), &ScriptSystemInspector::set_detailed_timing);
	ClassDB::bind_method(D_METHOD("get_multi_threaded"), &ScriptSystemInspector::get_multi_threaded);
	ClassDB::bind_method(D_METHOD("set_multi_threaded", "v"), &ScriptSystemInspector::set_multi_threaded);
	ClassDB::bind_method(D_METHOD("get_batch_chunk_size"), &ScriptSystemInspector::get_batch_chunk_size);
	ClassDB::bind_method(D_METHOD("set_batch_chunk_size", "v"), &ScriptSystemInspector::set_batch_chunk_size);
	ClassDB::bind_method(D_METHOD("get_flush_min_interval_msec"), &ScriptSystemInspector::get_flush_min_interval_msec);
	ClassDB::bind_method(D_METHOD("set_flush_min_interval_msec", "v"), &ScriptSystemInspector::set_flush_min_interval_msec);
	ClassDB::bind_method(D_METHOD("get_max_sample_count"), &ScriptSystemInspector::get_max_sample_count);
	ClassDB::bind_method(D_METHOD("set_max_sample_count", "cap"), &ScriptSystemInspector::set_max_sample_count);
	ClassDB::bind_method(D_METHOD("get_auto_reset"), &ScriptSystemInspector::get_auto_reset);
	ClassDB::bind_method(D_METHOD("set_auto_reset", "v"), &ScriptSystemInspector::set_auto_reset);
	ClassDB::bind_method(D_METHOD("get_paused"), &ScriptSystemInspector::get_paused);
	ClassDB::bind_method(D_METHOD("set_paused", "v"), &ScriptSystemInspector::set_paused);
	ClassDB::bind_method(D_METHOD("get_last_frame_entities"), &ScriptSystemInspector::get_last_frame_entities);
	ClassDB::bind_method(D_METHOD("get_last_frame_dispatch_usec"), &ScriptSystemInspector::get_last_frame_dispatch_usec);
	ClassDB::bind_method(D_METHOD("get_frame_invocations"), &ScriptSystemInspector::get_frame_invocations);
	ClassDB::bind_method(D_METHOD("get_frame_accum_usec"), &ScriptSystemInspector::get_frame_accum_usec);
	ClassDB::bind_method(D_METHOD("get_frame_min_usec"), &ScriptSystemInspector::get_frame_min_usec);
	ClassDB::bind_method(D_METHOD("get_frame_max_usec"), &ScriptSystemInspector::get_frame_max_usec);
	ClassDB::bind_method(D_METHOD("get_frame_median_usec"), &ScriptSystemInspector::get_frame_median_usec);
	ClassDB::bind_method(D_METHOD("get_frame_p99_usec"), &ScriptSystemInspector::get_frame_p99_usec);
	ClassDB::bind_method(D_METHOD("get_frame_stddev_usec"), &ScriptSystemInspector::get_frame_stddev_usec);
	ClassDB::bind_method(D_METHOD("get_last_frame_onadd"), &ScriptSystemInspector::get_last_frame_onadd);
	ClassDB::bind_method(D_METHOD("get_last_frame_onset"), &ScriptSystemInspector::get_last_frame_onset);
	ClassDB::bind_method(D_METHOD("get_last_frame_onremove"), &ScriptSystemInspector::get_last_frame_onremove);
	ClassDB::bind_method(D_METHOD("get_total_callbacks"), &ScriptSystemInspector::get_total_callbacks);
	ClassDB::bind_method(D_METHOD("get_total_entities_processed"), &ScriptSystemInspector::get_total_entities_processed);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "dispatch_mode", PROPERTY_HINT_ENUM, "Per Entity,Batch"), "set_dispatch_mode", "get_dispatch_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "change_only", PROPERTY_HINT_NONE, "Observe only component changes"), "set_change_only", "get_change_only");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "observe_add_and_set", PROPERTY_HINT_NONE, "When change_only: also count OnAdd events"), "set_observe_add_and_set", "get_observe_add_and_set");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "observe_remove", PROPERTY_HINT_NONE, "Track removal events (OnRemove observer)"), "set_observe_remove", "get_observe_remove");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "instrumentation_enabled", PROPERTY_HINT_NONE, "Collect per-frame counters and timings"), "set_instrumentation", "get_instrumentation");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "detailed_timing_enabled", PROPERTY_HINT_NONE, "Collect per-invocation timing samples"), "set_detailed_timing", "get_detailed_timing");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "multi_threaded", PROPERTY_HINT_NONE, "Run multi-threaded; data is batched & flushed on main thread"), "set_multi_threaded", "get_multi_threaded");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "batch/flush_chunk_size", PROPERTY_HINT_RANGE, "0,100000,1"), "set_batch_chunk_size", "get_batch_chunk_size");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "batch/flush_min_interval_msec", PROPERTY_HINT_RANGE, "0,1000,0.1"), "set_flush_min_interval_msec", "get_flush_min_interval_msec");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_sample_count", PROPERTY_HINT_RANGE, "1,4096,1"), "set_max_sample_count", "get_max_sample_count");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_reset_per_frame", PROPERTY_HINT_NONE, "Auto reset frame counters each PreUpdate"), "set_auto_reset", "get_auto_reset");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "paused", PROPERTY_HINT_NONE, "Pause dispatch without destroying systems/observers"), "set_paused", "get_paused");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame/entities", PROPERTY_HINT_NONE, "Entities processed this frame"), Variant(), "get_last_frame_entities");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame/dispatch_usec", PROPERTY_HINT_NONE, "Last dispatch duration (microseconds)"), Variant(), "get_last_frame_dispatch_usec");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame/invocations", PROPERTY_HINT_NONE, "Callback invocations this frame"), Variant(), "get_frame_invocations");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame/accum_usec", PROPERTY_HINT_NONE, "Accumulated dispatch time this frame"), Variant(), "get_frame_accum_usec");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame/min_usec", PROPERTY_HINT_NONE, "Min dispatch time among samples"), Variant(), "get_frame_min_usec");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame/max_usec", PROPERTY_HINT_NONE, "Max dispatch time among samples"), Variant(), "get_frame_max_usec");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "frame/median_usec", PROPERTY_HINT_NONE, "Median dispatch time"), Variant(), "get_frame_median_usec");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "frame/p99_usec", PROPERTY_HINT_NONE, "99th percentile dispatch time"), Variant(), "get_frame_p99_usec");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "frame/stddev_usec", PROPERTY_HINT_NONE, "Stddev of dispatch times"), Variant(), "get_frame_stddev_usec");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame/onadd", PROPERTY_HINT_NONE, "OnAdd events this frame"), Variant(), "get_last_frame_onadd");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame/onset", PROPERTY_HINT_NONE, "OnSet events this frame"), Variant(), "get_last_frame_onset");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame/onremove", PROPERTY_HINT_NONE, "OnRemove events this frame"), Variant(), "get_last_frame_onremove");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "total/callbacks", PROPERTY_HINT_NONE, "Total callbacks invoked"), Variant(), "get_total_callbacks");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "total/entities_processed", PROPERTY_HINT_NONE, "Total entities processed (all time)"), Variant(), "get_total_entities_processed");
}

// Accessors
int ScriptSystemInspector::get_dispatch_mode() const { return server->get_script_system_dispatch_mode(world_id, script_system_id); }
void ScriptSystemInspector::set_dispatch_mode(int m) { server->set_script_system_dispatch_mode(world_id, script_system_id, m); }
bool ScriptSystemInspector::get_change_only() const { return server->is_script_system_change_only(world_id, script_system_id); }
void ScriptSystemInspector::set_change_only(bool v) { server->set_script_system_change_only(world_id, script_system_id, v); }
bool ScriptSystemInspector::get_observe_add_and_set() const { return server->get_script_system_change_observe_add_and_set(world_id, script_system_id); }
void ScriptSystemInspector::set_observe_add_and_set(bool v) { server->set_script_system_change_observe_add_and_set(world_id, script_system_id, v); }
bool ScriptSystemInspector::get_observe_remove() const { return server->get_script_system_change_observe_remove(world_id, script_system_id); }
void ScriptSystemInspector::set_observe_remove(bool v) { server->set_script_system_change_observe_remove(world_id, script_system_id, v); }
bool ScriptSystemInspector::get_instrumentation() const { Dictionary d = server->get_script_system_instrumentation(world_id, script_system_id); return d.has("instrumentation_enabled") ? (bool)d["instrumentation_enabled"] : false; }
void ScriptSystemInspector::set_instrumentation(bool v) { server->set_script_system_instrumentation(world_id, script_system_id, v); }
bool ScriptSystemInspector::get_detailed_timing() const { return server->get_script_system_detailed_timing(world_id, script_system_id); }
void ScriptSystemInspector::set_detailed_timing(bool v) { server->set_script_system_detailed_timing(world_id, script_system_id, v); }
int ScriptSystemInspector::get_max_sample_count() const { return server->get_script_system_max_sample_count(world_id, script_system_id); }
void ScriptSystemInspector::set_max_sample_count(int cap) { server->set_script_system_max_sample_count(world_id, script_system_id, cap); }
bool ScriptSystemInspector::get_auto_reset() const { return server->get_script_system_auto_reset(world_id, script_system_id); }
void ScriptSystemInspector::set_auto_reset(bool v) { server->set_script_system_auto_reset(world_id, script_system_id, v); }
bool ScriptSystemInspector::get_paused() const { return server->is_script_system_paused(world_id, script_system_id); }
void ScriptSystemInspector::set_paused(bool v) { server->set_script_system_paused(world_id, script_system_id, v); }
int ScriptSystemInspector::get_last_frame_entities() const { return server->get_script_system_last_frame_entity_count(world_id, script_system_id); }
uint64_t ScriptSystemInspector::get_last_frame_dispatch_usec() const { return server->get_script_system_last_frame_dispatch_usec(world_id, script_system_id); }
uint64_t ScriptSystemInspector::get_frame_invocations() const { return server->get_script_system_frame_dispatch_invocations(world_id, script_system_id); }
uint64_t ScriptSystemInspector::get_frame_accum_usec() const { return server->get_script_system_frame_dispatch_accum_usec(world_id, script_system_id); }
uint64_t ScriptSystemInspector::get_frame_min_usec() const { return server->get_script_system_frame_min_usec(world_id, script_system_id); }
uint64_t ScriptSystemInspector::get_frame_max_usec() const { return server->get_script_system_frame_max_usec(world_id, script_system_id); }
double ScriptSystemInspector::get_frame_median_usec() const { return server->get_script_system_frame_median_usec(world_id, script_system_id); }
double ScriptSystemInspector::get_frame_p99_usec() const { return server->get_script_system_frame_p99_usec(world_id, script_system_id); }
double ScriptSystemInspector::get_frame_stddev_usec() const { return server->get_script_system_frame_stddev_usec(world_id, script_system_id); }
uint64_t ScriptSystemInspector::get_last_frame_onadd() const { return server->get_script_system_last_frame_onadd(world_id, script_system_id); }
uint64_t ScriptSystemInspector::get_last_frame_onset() const { return server->get_script_system_last_frame_onset(world_id, script_system_id); }
uint64_t ScriptSystemInspector::get_last_frame_onremove() const { return server->get_script_system_last_frame_onremove(world_id, script_system_id); }
uint64_t ScriptSystemInspector::get_total_callbacks() const { return server->get_script_system_total_callbacks(world_id, script_system_id); }
uint64_t ScriptSystemInspector::get_total_entities_processed() const { return server->get_script_system_total_entities_processed(world_id, script_system_id); }

bool ScriptSystemInspector::get_multi_threaded() const { return server->get_script_system_multi_threaded(world_id, script_system_id); }
void ScriptSystemInspector::set_multi_threaded(bool v) { server->set_script_system_multi_threaded(world_id, script_system_id, v); }
int ScriptSystemInspector::get_batch_chunk_size() const { return server->get_script_system_batch_chunk_size(world_id, script_system_id); }
void ScriptSystemInspector::set_batch_chunk_size(int v) { server->set_script_system_batch_chunk_size(world_id, script_system_id, v); }
double ScriptSystemInspector::get_flush_min_interval_msec() const { return server->get_script_system_flush_min_interval_msec(world_id, script_system_id); }
void ScriptSystemInspector::set_flush_min_interval_msec(double v) { server->set_script_system_flush_min_interval_msec(world_id, script_system_id, v); }
