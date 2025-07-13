//
// Created by Floof on 13-7-2025.
//

#ifndef ENTITY_H
#define ENTITY_H
#include "../../../../core/object/object.h"
#include "../../../../core/object/ref_counted.h"
#include "../../../../core/variant/dictionary.h"

#include "../../../../core/string/ustring.h"
#include "../../../../core/variant/array.h"
#include "../../thirdparty/flecs/distr/flecs.h"

class FlecsEntity : public RefCounted {
	GDCLASS(FlecsEntity, RefCounted)
private:
	flecs::entity entity;
public:
	void set_component(const String& component_type, const Dictionary& data, const Dictionary& metadata);
	Dictionary get_component();
	void remove_component(const String& component_type);
	void remove_all_components();
	Array get_components();
	String get_name();
	void set_name(const String& name);
	Dictionary get_metadata();


};

#endif //ENTITY_H
