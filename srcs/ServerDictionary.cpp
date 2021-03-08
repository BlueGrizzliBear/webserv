#include "./ServerDictionary.hpp"

/* Constructor */
/*	default	(1)	*/
ServerDictionary::ServerDictionary()
{
	main_dictionary.insert(std::make_pair("user", 0));
	main_dictionary.insert(std::make_pair("worker_processes", 1));
	main_dictionary.insert(std::make_pair("pid", 2));
	main_dictionary.insert(std::make_pair("worker_connections", 3));
	main_dictionary.insert(std::make_pair("client_max_body_size", 4));
	main_dictionary.insert(std::make_pair("server", 5));

	location_dictionary.insert(std::make_pair("root", 0));
	location_dictionary.insert(std::make_pair("autoindex", 1));
	location_dictionary.insert(std::make_pair("limit_except", 2));
	location_dictionary.insert(std::make_pair("upload_store", 3));
	location_dictionary.insert(std::make_pair("expires", 4));
	location_dictionary.insert(std::make_pair("proxy_pass", 5));
	location_dictionary.insert(std::make_pair("cgi", 6));

	server_dictionary.insert(std::make_pair("listen", 0));
	server_dictionary.insert(std::make_pair("server_name", 1));
	server_dictionary.insert(std::make_pair("error_page", 2));
	server_dictionary.insert(std::make_pair("location", 3));
	server_dictionary.insert(std::make_pair("root", 4));
	server_dictionary.insert(std::make_pair("autoindex", 5));
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
