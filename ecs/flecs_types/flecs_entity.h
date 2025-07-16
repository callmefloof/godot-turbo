//
// Created by Floof on 13-7-2025.
//
#pragma once
#include "../../../../core/object/object.h"
#include "flecs_component_base.h"
#include "../../../../core/string/ustring.h"
#include "../../thirdparty/flecs/distr/flecs.h"
#include "../../../../core/io/resource.h"

class FlecsEntity : public Resource {
	GDCLASS(FlecsEntity, Resource)
private:
	flecs::entity& entity;

protected:
	static void _bind_methods();
	static Vector<FlecsComponentBase> components;
public:
	FlecsEntity() : entity(){}
	~FlecsEntity() override = default;
	virtual void remove(const StringName& component_type);
	virtual void remove_all_components();
	virtual Ref<FlecsComponentBase> get(const StringName& component_type);
	virtual PackedStringArray get_component_types();
	StringName get_entity_name() const;
	void set_entity_name(const StringName &name) const;
	void set_entity(const flecs::entity& p_entity) const;
	flecs::entity& get_entity() const;
	virtual void set(const StringName &component_type, const Ref<FlecsComponentBase> &component);
	virtual void set(const StringName &component_type);
	virtual void set(const StringName &component_type, const Dictionary &component);

};

inline void FlecsEntity::_bind_methods() {
	//fill in methods

}
inline void FlecsEntity::remove(const StringName &component_type) {
	uint32_t i = 0;
	for ( ; i != components.size()-1; ++i) {
		if (components[i].get_type_name() == component_type) {
			break;
		}
	}
	const char* c_component_type = String(component_type).ascii().get_data();
	if (const flecs::entity comp = entity.world().lookup(c_component_type); entity.has(comp)) {
		entity = entity.remove(comp);
		components.remove_at(i);
	}
	else {
		ERR_PRINT("component type not found in entity");
	}

}
inline void FlecsEntity::remove_all_components() {
	components.clear();
}
inline Ref<FlecsComponentBase> FlecsEntity::get(const StringName &component_type) {
	for (auto it = components.begin(); it != components.end(); ++it) {
		if (it->get_type_name() == component_type) {
			// dereference and get the address of component
			return Ref(&(*it));
		}
	}
	ERR_PRINT("component type not found.");
	return Ref<FlecsComponentBase>();
}
inline PackedStringArray FlecsEntity::get_component_types() {
	PackedStringArray ret;
	for (auto it = components.begin(); it != components.end(); ++it) {
		ret.push_back(it->get_type_name());
	}
	return ret;
}
inline StringName FlecsEntity::get_entity_name() const {
	if (entity) {
		return StringName(entity.name());
	}
	return "ERROR";
}
inline void FlecsEntity::set_entity_name(const StringName &name) const {
	if (entity) {
		entity = entity.set_name(String(name).ascii().get_data());
		return;
	}
	ERR_PRINT("no entity referenced");
	return;
}
inline void FlecsEntity::set_entity(const flecs::entity& p_entity) const {
	entity = p_entity;
}
inline flecs::entity& FlecsEntity::get_entity() const {
	return entity;
}
inline void FlecsEntity::set(const StringName &component_type, const Ref<FlecsComponentBase> &component) {
	if (component.is_null() || !component.is_valid()) {
		ERR_PRINT("passed component is null");
		return;
	}
	for (auto it = components.begin(); it != components.end(); ++it) {
		if (it->get_type_name() == component_type) {
			remove(it->get_type_name());
			//some work to do here
		}
	}
}

inline void FlecsEntity::set(const StringName &component_type, const Dictionary &component) {
	for (auto it = components.begin(); it != components.end(); ++it) {
		if (it->get_type_name() == component_type) {
				// do something here
		}
	}
}

