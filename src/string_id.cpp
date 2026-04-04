#include "html.h"
#include "string_id.h"
#include <cassert>

#ifndef LITEHTML_NO_THREADS
	#include <mutex>
	static std::mutex mutex;
	#define lock_guard std::lock_guard<std::mutex> lock(mutex)
#else
	#define lock_guard
#endif

namespace litehtml
{

// Using function-local statics (Meyer's singleton) avoids double-free at exit:
// file-scope statics in static libraries can be initialized/destroyed multiple
// times when the translation unit is linked into more than one DSO, or when
// LTO merges COMDATs unexpectedly.  Function-local statics are guaranteed to
// be initialized exactly once (C++11 §6.7) and destroyed in reverse order of
// construction at program exit — after all TU-scope statics.
static std::map<string, string_id>& get_map() {
    static std::map<string, string_id> map;
    return map;
}
static std::vector<string>& get_array() {
    static std::vector<string> array;
    return array;
}

static int init()
{
	string_vector names;
	split_string(initial_string_ids, names, ",");
	for (auto& name : names)
	{
		trim(name);
		assert(name[0] == '_' && name.back() == '_');
		name = name.substr(1, name.size() - 2);				// _border_color_ -> border_color
		std::replace(name.begin(), name.end(), '_', '-');	// border_color   -> border-color
		_id(name);  // this will create association _border_color_ <-> "border-color"
	}
	return 0;
}
static int dummy = init();

const string_id empty_id = _id("");
const string_id star_id = _id("*");

string_id _id(const string& str)
{
	lock_guard;
	auto& map = get_map();
	auto& array = get_array();
	auto it = map.find(str);
	if (it != map.end()) return it->second;
	// else: str not found, add it to the array and the map
	array.push_back(str);
	return map[str] = (string_id)(array.size() - 1);
}

const string& _s(string_id id)
{
	lock_guard;
	return get_array()[id];
}

} // namespace litehtml