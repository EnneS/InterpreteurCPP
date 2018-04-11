#include "Interpreteur.h"
#include <stdlib.h>
#include <iostream>
using namespace std;

Interpreteur::Interpreteur(ifstream & fichier) :
m_lecteur(fichier), m_table(), m_arbre(nullptr) {
}

void Interpreteur::analyse() {
  m_arbre = programme(); // on lance l'analyse de la première règle
}

void Interpreteur::tester(const string & symboleAttendu) const throw (SyntaxeException) {
  // Teste si le symbole courant est égal au symboleAttendu... Si non, lève une exception
  static char messageWhat[256];
  if (m_lecteur.getSymbole() != symboleAttendu) {
    sprintf(messageWhat,
            "Ligne %d, Colonne %d - Erreur de syntaxe - Symbole attendu : %s - Symbole trouvé : %s",
            m_lecteur.getLigne(), m_lecteur.getColonne(),
            symboleAttendu.c_str(), m_lecteur.getSymbole().getChaine().c_str());
    throw SyntaxeException(messageWhat);
  }
}

void Interpreteur::testerEtAvancer(const string & symboleAttendu) throw (SyntaxeException) {
  // Teste si le symbole courant est égal au symboleAttendu... Si oui, avance, Sinon, lève une exception
  tester(symboleAttendu);
  m_lecteur.avancer();
}

void Interpreteur::erreur(const string & message) const throw (SyntaxeException) {
  // Lève une exception contenant le message et le symbole courant trouvé
  // Utilisé lorsqu'il y a plusieurs symboles attendus possibles...
  static char messageWhat[256];
  sprintf(messageWhat,
          "Ligne %d, Colonne %d - Erreur de syntaxe - %s - Symbole trouvé : %s",
          m_lecteur.getLigne(), m_lecteur.getColonne(), message.c_str(), m_lecteur.getSymbole().getChaine().c_str());
  throw SyntaxeException(messageWhat);
}

Noeud* Interpreteur::programme() {
  // <programme> ::= procedure principale() <seqInst> finproc FIN_FICHIER
  testerEtAvancer("procedure");
  testerEtAvancer("principale");
  testerEtAvancer("(");
  testerEtAvancer(")");
  Noeud* sequence = seqInst();
  testerEtAvancer("finproc");
  tester("<FINDEFICHIER>");
  return sequence;
}

Noeud* Interpreteur::seqInst() {
  // <seqInst> ::= <inst> { <inst> }
  NoeudSeqInst* sequence = new NoeudSeqInst();
  do {
    sequence->ajoute(inst());
  } while (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "si" || m_lecteur.getSymbole() == "tantque"
           || m_lecteur.getSymbole() == "repeter" || m_lecteur.getSymbole() == "pour" || m_lecteur.getSymbole() == "ecrire" || m_lecteur.getSymbole() == "lire");
  // Tant que le symbole courant est un début possible d'instruction...
  // Il faut compléter cette condition chaque fois qu'on rajoute une nouvelle instruction
  return sequence;
}

Noeud* Interpreteur::inst() {
  // <inst> ::= <affectation>  ; | <instSi>
  if (m_lecteur.getSymbole() == "<VARIABLE>") {
    Noeud *affect = affectation();
    testerEtAvancer(";");
    return affect;
  }
  else if (m_lecteur.getSymbole() == "si")
    return instSi();
  else if (m_lecteur.getSymbole() == "tantque")
    return instTantQue();
  else if(m_lecteur.getSymbole() == "repeter")
    return instRepeter();
  else if(m_lecteur.getSymbole() == "pour")
      return instPour();
  else if(m_lecteur.getSymbole() == "ecrire")
      return instEcrire();
  else if(m_lecteur.getSymbole() == "lire")
      return instLire();
  else erreur("Instruction incorrecte");
}

Noeud* Interpreteur::affectation() {
  // <affectation> ::= <variable> = <expression> 
  tester("<VARIABLE>");
  Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table eton la mémorise
  m_lecteur.avancer();
  testerEtAvancer("=");
  Noeud* exp = expression();             // On mémorise l'expression trouvée
  return new NoeudAffectation(var, exp); // On renvoie un noeud affectation
}

Noeud* Interpreteur::expression() {
  // <expression> ::= <facteur> { <opBinaire> <facteur> }
  //  <opBinaire> ::= + | - | *  | / | < | > | <= | >= | == | != | et | ou
  Noeud* fact = facteur();
  while ( m_lecteur.getSymbole() == "+"  || m_lecteur.getSymbole() == "-"  ||
          m_lecteur.getSymbole() == "*"  || m_lecteur.getSymbole() == "/"  ||
          m_lecteur.getSymbole() == "<"  || m_lecteur.getSymbole() == "<=" ||
          m_lecteur.getSymbole() == ">"  || m_lecteur.getSymbole() == ">=" ||
          m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=" ||
          m_lecteur.getSymbole() == "et" || m_lecteur.getSymbole() == "ou"   ) {
    Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
    m_lecteur.avancer();
    Noeud* factDroit = facteur(); // On mémorise l'opérande droit
    fact = new NoeudOperateurBinaire(operateur, fact, factDroit); // Et on construuit un noeud opérateur binaire
  }
  return fact; // On renvoie fact qui pointe sur la racine de l'expression
}

Noeud* Interpreteur::facteur() {
  // <facteur> ::= <entier> | <variable> | - <facteur> | non <facteur> | ( <expression> )
  Noeud* fact = nullptr;
  if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>" || m_lecteur.getSymbole() == "<CHAINE>") {
    fact = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
    m_lecteur.avancer();
  } else if (m_lecteur.getSymbole() == "-") { // - <facteur>
    m_lecteur.avancer();
    // on représente le moins unaire (- facteur) par une soustraction binaire (0 - facteur)
    fact = new NoeudOperateurBinaire(Symbole("-"), m_table.chercheAjoute(Symbole("0")), facteur());
  } else if (m_lecteur.getSymbole() == "non") { // non <facteur>
    m_lecteur.avancer();
    // on représente le moins unaire (- facteur) par une soustractin binaire (0 - facteur)
    fact = new NoeudOperateurBinaire(Symbole("non"), facteur(), nullptr);
  } else if (m_lecteur.getSymbole() == "(") { // expression parenthésée
    m_lecteur.avancer();
    fact = expression();
    testerEtAvancer(")");
  } else
    erreur("Facteur incorrect");
  return fact;
}

Noeud* Interpreteur::instSi() {
    // <instSi> ::= si ( <expression> ) <seqInst> finsi
    testerEtAvancer("si");
    testerEtAvancer("(");
    Noeud* condition = expression(); // On mémorise la condition
    testerEtAvancer(")");
    Noeud* sequence = seqInst();     // On mémorise la séquence d'instruction
    NoeudInstSi* noeud = new NoeudInstSi();
    noeud->ajouterCondition(condition);
    noeud->ajouterSequence(sequence);

    while  (m_lecteur.getSymbole() == "sinonsi"){
        testerEtAvancer("sinonsi");
        testerEtAvancer("(");
        Noeud* condition = expression();
        testerEtAvancer(")");
        Noeud* sequence = seqInst();
        noeud->ajouterCondition(condition);
        noeud->ajouterSequence(sequence);
    }

    if (m_lecteur.getSymbole() == "sinon"){
        testerEtAvancer("sinon");
        Noeud* sequence = seqInst();
        noeud->ajouterSequence(sequence);
    }

    testerEtAvancer("finsi");
    return noeud;
}

Noeud * Interpreteur::instTantQue() {
    testerEtAvancer("tantque");
    testerEtAvancer("(");
    Noeud * cond = expression();
    testerEtAvancer(")");
    Noeud * seq = seqInst();
    testerEtAvancer("fintantque");
    return new NoeudInstTq(cond, seq);
};

Noeud * Interpreteur::instRepeter() {
    testerEtAvancer("repeter");
    Noeud* seq = seqInst();
    testerEtAvancer("jusqua");
    testerEtAvancer("(");
    Noeud* cond = expression();
    testerEtAvancer(")");
    return new NoeudInstRepeter(cond, seq);
}

Noeud * Interpreteur::instPour() {
    Noeud * affect = nullptr;
    Noeud * incrementation = nullptr;

    testerEtAvancer("pour");
    testerEtAvancer("(");
    if (m_lecteur.getSymbole() == "<VARIABLE>") {
        affect = affectation();
    }

    testerEtAvancer(";");
    Noeud * cond = expression();
    testerEtAvancer(";");

    if (m_lecteur.getSymbole() == "<VARIABLE>") {
        incrementation = affectation();
    }

    testerEtAvancer(")");

    Noeud * seq = seqInst();
    testerEtAvancer("finpour");

    return new NoeudInstPour(affect, cond, incrementation, seq);
}

Noeud * Interpreteur::instEcrire() {
    NoeudInstEcrire * ecrire = new NoeudInstEcrire();

    testerEtAvancer("ecrire");
    testerEtAvancer("(");
    ecrire->ajouter(expression());
    while(m_lecteur.getSymbole() == ","){
        testerEtAvancer(",");
        ecrire->ajouter(expression());
    }
    testerEtAvancer(")");
    testerEtAvancer(";");
    return ecrire;
}

// =========================
// Traduction en C++

void Interpreteur::traduitEnCPP(ostream & cout, unsigned int indentation) const {
    // Includes
    cout << setw(4*indentation) << "#include <iostream>" << endl;

    // Début de la procédure principale
    cout << setw(4*indentation) << "int main() {" << endl;

    // On parcourt la table, si on trouve une variable alors il faut la déclarer avant d'écrire le programme !
    for(int i = 0; i < m_table.getTaille(); i++) {
        if(m_table[i] == "<VARIABLE>"){
            cout << setw(4*(indentation+1));
            cout << "int " << m_table[i].getChaine() << ";" << endl;
        }
    }

    // Traduction de l'arbre
    getArbre()->traduitEnCPP(cout, indentation + 1);

    // Fin
    cout << setw(4*indentation) << "" << "return 0;" << endl;
    cout << setw(indentation) << "}" << endl;
}

Noeud * Interpreteur::instLire() {

    testerEtAvancer("lire");
    testerEtAvancer("(");

    NoeudInstLire* noeud = new NoeudInstLire();
    tester("<VARIABLE>"); //leve exception sinon variable non reconnue
    Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole());
    noeud->ajouterVariable(var);
    testerEtAvancer("<VARIABLE>");

    while  (m_lecteur.getSymbole() == ","){
        testerEtAvancer(",");
        //tester("<VARIABLE>"); //leve exception sinon variable non reconnue
        Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole());
        noeud->ajouterVariable(var);
        testerEtAvancer("<VARIABLE>");
    }

    testerEtAvancer(")");
    testerEtAvancer(";");

    return noeud;
}