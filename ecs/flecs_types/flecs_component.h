//
// Created by Floof on 13-7-2025.
//

#ifndef FLECSCOMPONENENT_H
#define FLECSCOMPONENENT_H
#include "../../../../core/object/object.h"
#include "../../../../core/object/ref_counted.h"
#include "../../thirdparty/flecs/distr/flecs.h"
#include "../../../../core/string/ustring.h"
#include "../../../../core/variant/dictionary.h"
#include "../components/script_visible_component.h"


#include <typeinfo>

template <typename T = ScriptVisibleComponent>
class FlecsComponent : public RefCounted {
private:
	T &component_ref;
	String component_name;
	flecs::component<T> component;
	flecs::world& world;
	Dictionary component_metadata;

public:
	static Ref<FlecsComponent> create(String component_type, Dictionary data, Dictionary metadata);
	static Ref<FlecsComponent> create(String component_type, Dictionary data, Dictionary metadata);

	static Ref<FlecsComponent> create_new_type(String component_type, Dictionary data, Dictionary metadata);

	Dictionary get_component() {}
	void set_component(String component_type, Dictionary data, Dictionary metadata);
	void set_component(String component_type, Dictionary data);

	String get_component_name();
	FlecsComponent();
	~FlecsComponent() override = default;
};

template <typename T>
void FlecsComponent<T>::set_component(String component_type, Dictionary data, Dictionary metadata) {
	component_name = component_type;
	component.set_name(component_name);
	component_metadata = metadata;

}
template <typename T>
String FlecsComponent<T>::get_component_name() {
}
template <typename T>
FlecsComponent<T>::FlecsComponent() {
}
#endif //FLECSCOMPONENENT_H
