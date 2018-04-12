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
    // <programme> ::= procedure princip  ale() <seqInst> finproc FIN_FICHIER
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

    // Lecture du symbole courant en récupérant l'exception si il y a.
    try{
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
    } catch(SyntaxeException const &e){                 // On récupère l'exception
        cout << e.what() << endl;                       // On affiche le message d'erreur de l'exception
        // Tant que le symbole courant n'est pas une instruction ou la fin du programme alors on poursuit la lecture de celui-ci.
        // Cela permet de poursuivre l'analyse après récupération d'une erreur et de traiter entièrement toute les erreurs du programme.
        while(m_lecteur.getSymbole() != "<VARIABLE>" && m_lecteur.getSymbole() != "si" && m_lecteur.getSymbole() != "tantque"
              && m_lecteur.getSymbole() != "repeter" && m_lecteur.getSymbole() != "pour" && m_lecteur.getSymbole() != "ecrire" && m_lecteur.getSymbole() != "lire"
                && m_lecteur.getSymbole() != "<FINDEFICHIER>"){
            m_lecteur.avancer();
        }
    }
}

Noeud* Interpreteur::affectation() {
  // <affectation> ::= <variable> = <expression>
  tester("<VARIABLE>");
  Noeud* var = m_table.chercheAjoute(m_lecteur.getSymbole()); // La variable est ajoutée à la table eton la mémorise
  m_lecteur.avancer();
  testerEtAvancer("=");
  Noeud* exp = expBool();             // On mémorise l'expression trouvée
  return new NoeudAffectation(var, exp); // On renvoie un noeud affectation
}

Noeud* Interpreteur::expression() {
  // <expression> ::= <terme> { + <terme> | - <terme> }
  //  <opBinaire> ::= + | - | *  | / | < | > | <= | >= | == | != | et | ou
  Noeud* expression = terme();
  while ( m_lecteur.getSymbole() == "+"  || m_lecteur.getSymbole() == "-") {
    Symbole operateur = m_lecteur.getSymbole(); // On mémorise le symbole de l'opérateur
    m_lecteur.avancer();
    Noeud* termeDroit = terme(); // On mémorise l'opérande droit
    expression = new NoeudOperateurBinaire(operateur, expression, termeDroit); // Et on construuit un noeud opérateur binaire
  }
  return expression; // On renvoie fact qui pointe sur la racine de l'expression
}

Noeud* Interpreteur::facteur() {
  // <facteur> ::= <entier> | <variable> | - <expBool> | non <expBool> | ( <expBool> )
  Noeud* fact = nullptr;
  if (m_lecteur.getSymbole() == "<VARIABLE>" || m_lecteur.getSymbole() == "<ENTIER>") {
      fact = m_table.chercheAjoute(m_lecteur.getSymbole()); // on ajoute la variable ou l'entier à la table
      m_lecteur.avancer();
  } else if (m_lecteur.getSymbole() == "<CHAINE>"){ // Si le symbole courant est une chaine
      fact = m_table.chercheAjoute(m_lecteur.getSymbole().getChaine()); // on ajoute la chaine à la table
      m_lecteur.avancer();
  } else if (m_lecteur.getSymbole() == "-") { // - <expBool>
    m_lecteur.avancer();
    // on représente le moins unaire (- expBool) par une soustraction binaire (0 - expBool)
    fact = new NoeudOperateurBinaire(Symbole("-"), m_table.chercheAjoute(Symbole("0")), expBool());
  } else if (m_lecteur.getSymbole() == "non") { // non <expBool>
    m_lecteur.avancer();
    // on représente le moins unaire (- expBool) par une soustractin binaire (0 - expBool)
    fact = new NoeudOperateurBinaire(Symbole("non"), expBool(), nullptr);
  } else if (m_lecteur.getSymbole() == "(") { // expBool parenthésée
    m_lecteur.avancer();
    fact = expBool();
    testerEtAvancer(")");
  } else
    erreur("Facteur incorrect");
  return fact;
}


Noeud* Interpreteur::terme() {
    // <terme> ::= <facteur> { * <facteur> | / <facteur> }
    Noeud* terme = facteur();
    while(m_lecteur.getSymbole() == "/" || m_lecteur.getSymbole() == "*"){
        Symbole opreateur = m_lecteur.getSymbole();
        m_lecteur.avancer();
        Noeud* factDroit = facteur();
        terme = new NoeudOperateurBinaire(opreateur, terme, factDroit);
    }
    return terme;
}

Noeud* Interpreteur::expBool() {
    // <expBool> ::= <relationEt> { ou <relationEt> }
    Noeud* expBool = relationEt();
    while (m_lecteur.getSymbole() == "ou" ){
        Symbole operateur = m_lecteur.getSymbole();
        m_lecteur.avancer();
        Noeud* relEtDroite = relationEt();
        expBool = new NoeudOperateurBinaire(operateur, expBool, relEtDroite);
    }
    return expBool;
}

Noeud* Interpreteur::relationEt() {
    //  <relationEt> ::= <relation> { et <relation> }
    Noeud* relationEt = relation();
    while(m_lecteur.getSymbole() == "et"){
        Symbole operateur = m_lecteur.getSymbole(); // opérateur (ce sera toujours "et")
        m_lecteur.avancer();
        Noeud* relDroite = relation();  // Relation de droite
        relationEt = new NoeudOperateurBinaire(operateur, relationEt, relDroite);
    }
    return relationEt;
}

Noeud* Interpreteur::relation() {
    // <relation> ::= <expression> { <opRel> <expression> }
    Noeud* relation = expression();
    // Tant que le symbole courant est une opération booléenne
    while(opRel()){
        Symbole opRel = m_lecteur.getSymbole();
        m_lecteur.avancer();
        Noeud* expDroite = expression();
        relation = new NoeudOperateurBinaire(opRel, relation, expDroite); // On renvoie le noeud avec l'opération, l'expression de gauche et l'expression de droite
    }
    return relation;
}

bool Interpreteur::opRel(){
    // <opRel> ::= == | != | < | <= | > | >=
    return m_lecteur.getSymbole() == "==" || m_lecteur.getSymbole() == "!=" || m_lecteur.getSymbole() == "<"
            || m_lecteur.getSymbole() == "<=" || m_lecteur.getSymbole() == ">" || m_lecteur.getSymbole() == ">=";
};

Noeud* Interpreteur::instSi() {
    // <instSi> ::= si ( <expression> ) <seqInst> finsi
    testerEtAvancer("si");
    testerEtAvancer("(");
    Noeud* condition = expBool(); // On mémorise la condition
    testerEtAvancer(")");
    Noeud* sequence = seqInst();     // On mémorise la séquence d'instruction
    NoeudInstSi* noeud = new NoeudInstSi();
    noeud->ajouterCondition(condition);
    noeud->ajouterSequence(sequence);

    while  (m_lecteur.getSymbole() == "sinonsi"){
        testerEtAvancer("sinonsi");
        testerEtAvancer("(");
        Noeud* condition = expBool();
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
    Noeud * cond = expBool();
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
    Noeud* cond = expBool();
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
    Noeud * cond = expBool();
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
    ecrire->ajouter(expBool());
    while(m_lecteur.getSymbole() == ","){
        testerEtAvancer(",");
        ecrire->ajouter(expBool());
    }
    testerEtAvancer(")");
    testerEtAvancer(";");
    return ecrire;
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