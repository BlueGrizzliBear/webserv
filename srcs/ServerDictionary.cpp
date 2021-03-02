#include "./ServerDictionary.hpp"

/* Constructor */
/*	default	(1)	*/
ServerDictionary::ServerDictionary()
{
	context_dictionary.insert(std::make_pair("user", 0));
	context_dictionary.insert(std::make_pair("worker_processes", 1));
	context_dictionary.insert(std::make_pair("pid", 2));
	context_dictionary.insert(std::make_pair("worker_connections", 3));
	context_dictionary.insert(std::make_pair("client_max_body_size", 4));
	context_dictionary.insert(std::make_pair("server", 5));

	location_dictionary.insert(std::make_pair("root", 0));
	location_dictionary.insert(std::make_pair("autoindex", 1));
	location_dictionary.insert(std::make_pair("limit_except", 2));
	location_dictionary.insert(std::make_pair("upload_store", 3));

	server_dictionary.insert(std::make_pair("listen", 0));
	server_dictionary.insert(std::make_pair("server_name", 1));
	server_dictionary.insert(std::make_pair("error_page", 2));
	server_dictionary.insert(std::make_pair("location", 3));
	server_dictionary.insert(std::make_pair("root", 4));
	server_dictionary.insert(std::make_pair("autoindex", 5));

	// const std::string context_dictionary[6] = {"user", "worker_processes", "pid", "worker_connections", "client_max_body_size", "server"};
	// size_t	context_dic_size = 6;
	// const std::string location_dictionary[4] = {"root", "autoindex", "limit_except", "upload_store"};
	// size_t	location_dic_size = 4;
	// const std::string server_dictionary[6] = {"listen", "server_name", "error_page", "location", "root", "autoindex"};
	// size_t	server_dic_size = 6;
}

/*	copy	(2)	*/
ServerDictionary::ServerDictionary(ServerDictionary const & cpy)
{
	*this = cpy;
}

/* Destructor */
ServerDictionary::~ServerDictionary() {}

/* Operators */
ServerDictionary &ServerDictionary::operator=( ServerDictionary const & rhs )
{
	(void)rhs;
	return (*this);
}

/* Member Functions */
