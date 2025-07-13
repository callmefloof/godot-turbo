//
// Created by Floof on 13-7-2025.
//

#ifndef SCRIPT_VISIBLE_COMPONENT_H
#define SCRIPT_VISIBLE_COMPONENT_H
#include "../../../../core/variant/dictionary.h"
#include "../../../../core/string/ustring.h"

#define SET_SCRIPT_COMPONENT_VALUE(dict,key, lvalue, variant_type)		\
if (dict.has(String(#key))) {											\
	if(dict[String(#key)].get_type() == (variant_type)){				\
		lvalue = dict[String(#key)];									\
	}																	\
	else {																\
		ERR_PRINT(String(#key) + "is not of right data type");			\
	}																	\
}																		\
else {																	\
	ERR_PRINT(String(#key) + " is required.");							\
	return;																\
}																		\


struct ScriptVisibleComponent {
	virtual Dictionary to_dict() const = 0;
	virtual void from_dict(Dictionary dict) = 0;
};


#endif //SCRIPT_VISIBLE_COMPONENT_H
