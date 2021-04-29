#ifndef SERVERDICTIONARY_HPP
# define SERVERDICTIONARY_HPP

# include "./webserv.hpp"

class	ServerDictionary
{
	/* Member Types */
	public:
		typedef std::map<std::string, std::string >	Dic;

	/* Constructor */
		/*	default	(1)	*/	ServerDictionary(void);
		/*	copy	(2)	*/	ServerDictionary(ServerDictionary const & cpy);

	/* Destructor */
		~ServerDictionary();

	/* Operators */
		ServerDictionary &	operator=(ServerDictionary const & rhs);

	/* Member Functions */
	private:
		template < class Compare >
		void	_createDic(std::map< std::string, std::string, Compare > & dic, std::string const * tab, size_t size);
		template < class Compare >
		void	_createDic(std::map<std::string, std::string, Compare > & dic, std::pair<std::string , std::string> const * tab, size_t size);
		void	_parseMimeTypes(void);

	/* Member Attributes */
	public:
		Dic	mainDic;
		Dic	locationDic;
		Dic	serverDic;
		Dic	methodDic;
		std::map<std::string, std::string, ci_less>	headerDic;
		Dic	errorDic;
		std::map<std::string, std::string, ci_less> mimeDic;
};

#endif
