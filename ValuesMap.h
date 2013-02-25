/*-----------------------------------------------------------------------------
 * ValuesMap Library Header 
 *
 * $Id$
 * $HeadURL$
 *
 * Copyright (C) 2008-2009 Clone Research Corporation 
 * All Rights Reserved.
 *-----------------------------------------------------------------------------
 */
#ifndef valuesmap_h_included
#define valuesmap_h_included 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct string_map string_map;

string_map *value_map_string_create(void );
void value_map_string_destroy(string_map *map);
long value_map_string_increment(string_map *map, const char *value);

void value_map_string_lengths(string_map *map, int *minl, int *maxl);

#ifdef __cplusplus
}
#endif

#endif

