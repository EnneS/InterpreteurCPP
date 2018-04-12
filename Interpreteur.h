#ifndef INTERPRETEUR_H
#define INTERPRETEUR_H

#include "Symbole.h"
#include "Lecteur.h"
#include "Exceptions.h"
#include "TableSymboles.h"
#include "ArbreAbstrait.h"

class Interpreteur {
public:
	Interpreteur(ifstream & fichier);   // Construit un interpréteur pour interpreter
	                                    //  le programme dans  fichier 
                                      
	void analyse();                     // Si le contenu du fichier est conforme à la grammaire,
	                                    //   cette méthode se termine normalement et affiche un message "Syntaxe correcte".
                                      //   la table des symboles (ts) et l'arbre abstrait (arbre) auront été construits
	                                    // Sinon, une exception sera levée

	inline const TableSymboles & getTable () const  { return m_table;    } // accesseur	
	inline Noeud* getArbre () const { return m_arbre; }                    // accesseur
	void traduitEnCPP(ostream& cout, unsigned int indentation) const;

	private:
    Lecteur        m_lecteur;  // Le lecteur de symboles utilisé pour analyser le fichier
    TableSymboles  m_table;    // La table des symboles valués
    Noeud*         m_arbre;    // L'arbre abstrait

    // Implémentation de la grammaire
    Noeud*  programme();   //   <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
    Noeud*  seqInst();	   //     <seqInst> ::= <inst> { <inst> }
    Noeud*  inst();	       //        <inst> ::= <affectation> ; | <instSi>
    Noeud*  affectation(); // <affectation> ::= <variable> = <expression> 
    Noeud*  expression();  //  <expression> ::= <terme> { + <terme> | - <terme> }
    Noeud*  facteur();     //     <facteur> ::= <entier>  |  <variable>  |  - <expBool>  | non <expBool> | ( <expBool> )
    Noeud* terme();        //       <terme> ::= <facteur> { * <facteur> | / <facteur> }
    Noeud* expBool();      //     <expBool> ::= <relationEt> { ou <relationEt> }
    Noeud* relationEt();   //  <relationEt> ::= <relation> { et <relation> }
    Noeud* relation();     //    <relation> ::= <expression> { <opRel> <expression> }
    bool opRel();        //       <opRel> ::= == | != | < | <= | > | >=

    Noeud*  instSi();      //      <instSi> ::= si ( <expression> ) <seqInst> finsi
	Noeud* instTantQue();
	Noeud* instRepeter();
	Noeud* instPour();
	Noeud* instEcrire();

	// Traduction en CPP

	// outils pour simplifier l'analyse syntaxique
	Noeud* 	instLire();    //	 <instLire> ::= lire(<variable> { ,<variable> })



    // outils pour simplifier l'analyse syntaxique
    void tester (const string & symboleAttendu) const throw (SyntaxeException);   // Si symbole courant != symboleAttendu, on lève une exception
    void testerEtAvancer(const string & symboleAttendu) throw (SyntaxeException); // Si symbole courant != symboleAttendu, on lève une exception, sinon on avance
    void erreur (const string & mess) const throw (SyntaxeException);             // Lève une exception "contenant" le message mess
};

#endif /* INTERPRETEUR_H */
