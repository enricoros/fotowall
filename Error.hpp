#ifndef ARNAUD_ERROR_H
#define ARNAUD_ERROR_H

#include <exception>
class Error: public std::exception
{
public:
    Error(char const* phrase="",int numero=0, int niveau=0) throw()
         :m_numero(numero),m_phrase(phrase),m_niveau(niveau)
    {}
 
     virtual const char* what() const throw()
     {
         return m_phrase;
     }
     
     int getNiveau() const throw()
     {
          return m_niveau;
     }
    
    virtual ~Error() throw()
    {}
 
private:
    int m_numero;               //Numéro de l'erreur
    char const* m_phrase;       //Description de l'erreur
    int m_niveau;               //Niveau de l'erreur
};

#endif /* ARNAUD_ERROR_H */
