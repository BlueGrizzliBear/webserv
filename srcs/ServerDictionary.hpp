#ifndef SERVERDICTIONARY_HPP
#define SERVERDICTIONARY_HPP

#include "./webserv.hpp"

class	ServerDictionary
{
	/* Member Types */
	public:
		typedef std::map<std::string, int>	Dic;

	/* Constructor */
		/*	default	(1)	*/	ServerDictionary(void);
		/*	copy	(2)	*/	ServerDictionary(ServerDictionary const & cpy);

	/* Destructor */
		~ServerDictionary();

	/* Operators */
		ServerDictionary &	operator=(ServerDictionary const & rhs);

	/* Member Functions */
	private:
		void	_createDic(Dic & dic, std::string const * tab, size_t size);

	/* Member Attributes */
	public:
		Dic	mainDic;
		Dic	locationDic;
		Dic	serverDic;
		Dic	methodDic;
		Dic	headerDic;
};

#endif
