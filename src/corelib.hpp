// corelib.hpp
#ifndef CORELIB_HPP
#define CORELIB_HPP

extern "C" {

// Basic ops
int api_add_user(const char* name);
int api_add_user_with_id(const char* name, int fixedId); // used by persistence loading
bool api_add_friend(int a, int b);
bool api_remove_friend(int a, int b);
bool api_remove_user(int id);

// Interests
bool api_add_interests(int id, const char* comma_separated_interests);
char* api_get_user_interests(int id);

// Queries / algorithms -> return JSON-encoded C string (caller must free)
char* api_list_all_users(); // [{"id":1,"name":"xxx"},...]
char* api_print_user_info(int id); // {"id":..,"name":"..","friends":[..],"interests":[..]}
char* api_recommend_mutual(int userId, int topK); // [{"id":..,"score":..,"name":".."},...]
char* api_recommend_weighted(int userId, int topK); // [{"id":..,"score":..,"mutuals":..,"shared_interests":[..], "name":".."},...]
char* api_shortest_path(int src, int dst); // {"path":[1,2,3]}
char* api_connected_components(); // [[1,2,3],[4,5],...]
char* api_suggest_prefix(const char* prefix, int k); // [{"id":..,"name":".."},...]

// Persistence
bool api_save_network(const char* filename);
bool api_load_network(const char* filename);

// Memory helper (free strings returned above)
void api_free_string(char* s);

} // extern "C"

#endif // CORELIB_HPP