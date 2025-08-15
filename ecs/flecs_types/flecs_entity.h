//
// Created by Floof on 13-7-2025.
//
#pragma once
#include "core/io/resource.h"
#include "core/object/object.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include "core/typedefs.h"
#include "core/variant/variant.h"
#include "core/object/ref_counted.h"
#include "../../thirdparty/flecs/distr/flecs.h"

class FlecsComponentBase;


class FlecsEntity : public Resource {
	GDCLASS(FlecsEntity, Resource)

protected:
	static void _bind_methods();
	flecs::entity entity;
	flecs::world* world = nullptr;
	flecs::entity parent;
	Ref<FlecsEntity> gd_parent;
	Vector<Ref<FlecsEntity>> children;
	Vector<Ref<FlecsComponentBase>> components;

public:
	FlecsEntity() = default;
	~FlecsEntity() = default;
	void remove_with_component(const Ref<FlecsComponentBase> &comp);
	void remove_all_components();
	Ref<FlecsComponentBase> get_component(const StringName &component_type) const;
	bool has_component(const StringName &component_type) const;
	PackedStringArray get_component_types() const;
	StringName get_entity_name() const;
	void set_entity_name(const StringName &p_name) const;
	void set_entity(const flecs::entity &p_entity);
	flecs::entity get_entity() const;
	void set_component(const Ref<FlecsComponentBase> &comp_ref);
	void remove(const String &component_type);
	Ref<FlecsComponentBase> get_component_by_name(const StringName &component_type);
	Ref<FlecsEntity> get_parent() const;
	void set_parent(const Ref<FlecsEntity> &p_parent);
	flecs::entity get_internal_parent() const;
	void set_internal_parent(const flecs::entity &p_parent);
	void clear_components();
	void add_component(const Ref<FlecsComponentBase> &comp_ref);
	flecs::entity get_internal_entity() const;
	void set_internal_entity(const flecs::entity &p_entity);
	flecs::world* get_internal_world() const;
	void set_internal_world(flecs::world* p_world);

	void add_child(const Ref<FlecsEntity> &child);
	void remove_child(const Ref<FlecsEntity> &child);
	TypedArray<FlecsEntity> get_children() const;
	Ref<FlecsEntity> get_child(int index) const;
	void set_children(const TypedArray<FlecsEntity> &children);
	Ref<FlecsEntity> get_child_by_name(const StringName &name) const;
	void set_child_by_name(const StringName &name, const Ref<FlecsEntity> &child);
	void set_child_by_index(int index, const Ref<FlecsEntity> &child);
	void remove_child_by_name(const StringName &name);
	void remove_child_by_index(int index);
	void remove_all_children();
};


