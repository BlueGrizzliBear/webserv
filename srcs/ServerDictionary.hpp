#ifndef SERVERDICTIONARY_HPP
#define SERVERDICTIONARY_HPP

#include "./webserv.hpp"

class	ServerDictionary
{
	/* Member Types */
	public:
		typedef std::map<std::string, std::string>	Dic;

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
		void	_createDic(Dic & dic, std::pair<std::string , std::string> const * tab, size_t size);
		Dic		_parseMimeTypes(void);

	/* Member Attributes */
	public:
		Dic	mainDic;
		Dic	locationDic;
		Dic	serverDic;
		Dic	methodDic;
		Dic	headerDic;
		Dic	errorDic;
		Dic mimeDic;
};

#endif
