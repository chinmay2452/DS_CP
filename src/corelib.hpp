#ifndef CORELIB_HPP
#define CORELIB_HPP

extern "C" {

// Basic ops
int _api_add_user(const char* name);
int _api_add_user_with_id(const char* name, int fixedId);
bool _api_add_friend(int a, int b);
bool _api_remove_friend(int a, int b);
bool _api_remove_user(int id);

// Interests
bool _api_add_interests(int id, const char* comma_separated_interests);
char* _api_get_user_interests(int id);

// Queries / algorithms
char* _api_list_all_users();
char* _api_print_user_info(int id);
char* _api_recommend_mutual(int userId, int topK);
char* _api_recommend_weighted(int userId, int topK);
char* _api_shortest_path(int src, int dst);
char* _api_connected_components();
char* _api_suggest_prefix(const char* prefix, int k);

// Persistence
bool _api_save_network(const char* filename);
bool _api_load_network(const char* filename);

// Memory free helper
void _api_free_string(char* s);

} // extern "C"

#endif // CORELIB_HPP