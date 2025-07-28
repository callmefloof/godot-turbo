//
// Created by Floof on 13-7-2025.
//
#pragma once
#include "../../../../core/io/resource.h"
#include "../../../../core/object/object.h"
#include "../../../../core/string/string_name.h"
#include "../../../../core/string/ustring.h"
#include "../../../../core/templates/vector.h"
#include "../../../../core/typedefs.h"
#include "../../../../core/variant/variant.h"
#include "../../../../core/object/ref_counted.h"

#include "../../thirdparty/flecs/distr/flecs.h"

class FlecsComponentBase;

class FlecsEntity : public Resource {
	GDCLASS(FlecsEntity, Resource)

protected:
	static void _bind_methods();
	flecs::entity entity;
	Vector<Ref<FlecsComponentBase>> components;

public:
	FlecsEntity() = default;
	virtual ~FlecsEntity() = default;
	virtual void remove(const Ref<FlecsComponentBase> &comp);
	virtual void remove_all_components();
	virtual Ref<FlecsComponentBase> get_component(const StringName &component_type) const;
	PackedStringArray get_component_types() const;
	StringName get_entity_name() const;
	void set_entity_name(const StringName &p_name) const;
	void set_entity(const flecs::entity &p_entity);
	flecs::entity get_entity() const;
	virtual void set_component(const Ref<FlecsComponentBase> &comp_ref);
	virtual void remove(String &component_type);
	virtual Ref<FlecsComponentBase> get_component_by_name(const StringName &component_type);
};

