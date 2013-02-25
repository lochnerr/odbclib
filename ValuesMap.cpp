#include <iostream>
#include <string>
#include <map>
#include <set>

#include "ValuesMap.h"

/***
template<class T> // simulated template typedef
struct value_map
{
	typedef std::map<T, long> Type;
};
***/

string_map *value_map_string_create(void )
{
	return (string_map *)new std::map<std::string, long>;
}

void value_map_string_destroy(string_map *map)
{
	std::map<std::string, long> *obj = reinterpret_cast< std::map<std::string, long> *> (map);

	delete obj;
}

long value_map_string_increment(string_map *map, const char *value)
{	
	std::map<std::string, long> *obj = (std::map<std::string, long> *)map;
	std::pair<std::map<std::string, long>::iterator ,bool > rv;
	
	rv = obj->insert( std::pair<std::string, long >(value, 1) ); /* try to insert the value */
	if (!rv.second) { /* no item inserted */ 
		rv.first->second++; /* item fouund, so increment the count */
		return rv.first->second++;
		//std::cout << "Incremented, count = " << rv.first->second << std::endl;
	} else {
		//std::cout << "Added" << std::endl;
		return 1;
	}
	
}

void value_map_string_lengths(string_map *map, int *minl, int *maxl)
{
	std::map<std::string, long> *obj = (std::map<std::string, long> *)map;
	std::map<std::string, long>::iterator iter;

	int mini, maxi, len;

	iter = obj->begin();

	if (obj->begin() == obj->end()) {
		*minl = 0;
		*maxl = 0;
		return;
	}
	
	mini = (*iter).first.length();
	maxi = (*iter).first.length();

	for (; iter != obj->end(); iter++) {
		len = (*iter).first.length();
		if (len < mini) mini = len;
		if (len > maxi) maxi = len;
	}

	*minl = mini;
	*maxl = maxi;
}