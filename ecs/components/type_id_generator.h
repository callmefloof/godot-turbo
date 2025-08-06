//
// Created by Floof on 27-7-2025.
//

#ifndef TYPE_ID_GENERATOR_H
#define TYPE_ID_GENERATOR_H
#include "core/string/string_name.h"
#include "core/templates/a_hash_map.h"

class TypeIDGenerator {
public:
	static int& global_id() {
		static int id = 0;
		return id;
	}

	static int next_id() {
		return global_id()++;
	}

	template <typename T>
	static int get_type_id() {
		static int id = next_id();
		return id;
	}

	static int get_id_for_string(const StringName &name) {
		static AHashMap<StringName, int> map;
		static int next_id = 0;
		if (map.has(name)) return map[name];
		map[name] = next_id;
		return next_id++;
	}
};
#endif //TYPE_ID_GENERATOR_H
