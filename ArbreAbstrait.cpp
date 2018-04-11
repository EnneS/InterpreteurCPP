#include <stdlib.h>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"
#include <typeinfo>
#include <algorithm>
////////////////////////////////////////////////////////////////////////////////
// NoeudSeqInst
////////////////////////////////////////////////////////////////////////////////

NoeudSeqInst::NoeudSeqInst() : m_instructions() {
}

int NoeudSeqInst::executer() {
  for (unsigned int i = 0; i < m_instructions.size(); i++)
    m_instructions[i]->executer(); // on exécute chaque instruction de la séquence
  return 0; // La valeur renvoyée ne représente rien !
}

void NoeudSeqInst::ajoute(Noeud* instruction) {
  if (instruction!=nullptr) m_instructions.push_back(instruction);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudAffectation
////////////////////////////////////////////////////////////////////////////////

NoeudAffectation::NoeudAffectation(Noeud* variable, Noeud* expression)
: m_variable(variable), m_expression(expression) {
}

int NoeudAffectation::executer() {
  int valeur = m_expression->executer(); // On exécute (évalue) l'expression
  ((SymboleValue*) m_variable)->setValeur(valeur); // On affecte la variable
  return 0; // La valeur renvoyée ne représente rien !
}

////////////////////////////////////////////////////////////////////////////////
// NoeudOperateurBinaire
////////////////////////////////////////////////////////////////////////////////

NoeudOperateurBinaire::NoeudOperateurBinaire(Symbole operateur, Noeud* operandeGauche, Noeud* operandeDroit)
: m_operateur(operateur), m_operandeGauche(operandeGauche), m_operandeDroit(operandeDroit) {
}

int NoeudOperateurBinaire::executer() {
  int og, od, valeur;
  if (m_operandeGauche != nullptr) og = m_operandeGauche->executer(); // On évalue l'opérande gauche
  if (m_operandeDroit != nullptr) od = m_operandeDroit->executer(); // On évalue l'opérande droit
  // Et on combine les deux opérandes en fonctions de l'opérateur
  if (this->m_operateur == "+") valeur = (og + od);
  else if (this->m_operateur == "-") valeur = (og - od);
  else if (this->m_operateur == "*") valeur = (og * od);
  else if (this->m_operateur == "==") valeur = (og == od);
  else if (this->m_operateur == "!=") valeur = (og != od);
  else if (this->m_operateur == "<") valeur = (og < od);
  else if (this->m_operateur == ">") valeur = (og > od);
  else if (this->m_operateur == "<=") valeur = (og <= od);
  else if (this->m_operateur == ">=") valeur = (og >= od);
  else if (this->m_operateur == "et") valeur = (og && od);
  else if (this->m_operateur == "ou") valeur = (og || od);
  else if (this->m_operateur == "non") valeur = (!og);
  else if (this->m_operateur == "/") {
    if (od == 0) throw DivParZeroException();
    valeur = og / od;
  }
  return valeur; // On retourne la valeur calculée
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstSi
////////////////////////////////////////////////////////////////////////////////

NoeudInstSi::NoeudInstSi() {}

int NoeudInstSi::executer() {
    int i = 0;
    while (i < m_conditions.size() && !m_conditions[i]->executer()){
        i++;
    }

    if(m_sequences.size() < m_conditions.size() || i < m_sequences.size()) m_sequences[i]->executer();

    return 0;

}

void NoeudInstSi::ajouterCondition(Noeud* cond){
    m_conditions.push_back(cond);
}
void NoeudInstSi::ajouterSequence(Noeud* seq){
    m_sequences.push_back(seq);
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstTq
////////////////////////////////////////////////////////////////////////////////

NoeudInstTq::NoeudInstTq(Noeud* condition, Noeud* sequence)
        : m_condition(condition), m_sequence(sequence) {
}

int NoeudInstTq::executer() {
  while(m_condition->executer()) {
    m_sequence->executer();
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstRepeter
////////////////////////////////////////////////////////////////////////////////


NoeudInstRepeter::NoeudInstRepeter(Noeud * condition, Noeud * sequence)
        : m_condition(condition), m_sequence(sequence) {
}

int NoeudInstRepeter::executer(){
    do {
        m_sequence->executer();
    } while(!m_condition->executer());

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstPour
////////////////////////////////////////////////////////////////////////////////

NoeudInstPour::NoeudInstPour(Noeud *affectation, Noeud *condition, Noeud *incrementation, Noeud *sequence)
        : m_affectation(affectation), m_condition(condition), m_incrementation(incrementation), m_sequence(sequence) {}

int NoeudInstPour::executer(){
    if(m_incrementation && m_affectation){
        for(m_affectation->executer(); m_condition->executer(); m_incrementation->executer()){
            m_sequence->executer();
        }
    } else if(m_incrementation){
        for(; m_condition->executer(); m_incrementation->executer()){
            m_sequence->executer();
        }
    } else if(m_affectation){
        // Boucle infinie
        for(m_affectation->executer(); m_condition->executer();){
            m_sequence->executer();
        }
    } else {
        // Boucle infinie
        for(; m_condition->executer();){
            m_sequence->executer();
        }
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// NoeudInstEcrire
////////////////////////////////////////////////////////////////////////////////

NoeudInstEcrire::NoeudInstEcrire() {}

void NoeudInstEcrire::ajouter(Noeud *expression) {
    m_expressions.push_back(expression);
}

int NoeudInstEcrire::executer(){
     for(auto exp : m_expressions){
            if((typeid(*exp)) == typeid(SymboleValue) && *((SymboleValue*)exp) == "<CHAINE>"){
                string str = ((SymboleValue*) exp)->getChaine();
                str.erase(std::remove(str.begin(), str.end(), '"'), str.end());
                cout << str;
            } else {
                cout << exp->executer();
            }
        }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Traduction en C++
////////////////////////////////////////////////////////////////////////////////

void NoeudSeqInst::traduitEnCPP(ostream &cout, unsigned int indentation) const {
    for (unsigned int i = 0; i < m_instructions.size(); i++) {
        m_instructions[i]->traduitEnCPP(cout, indentation);

        // Si l'instruction était une affectation, il faut lui ajouter un semicolon.
        // On le fait ici pour différencier les affectations des initialisations de boucles et celles faisant parti de la séquence d'instruction.
        if(typeid(*m_instructions[i]) == typeid(NoeudAffectation)){
            cout << ";";
        }

        cout << endl;
    }
}

void NoeudAffectation::traduitEnCPP(ostream &cout, unsigned int indentation) const {
    cout << setw(2*indentation) << "";
    m_variable->traduitEnCPP(cout, indentation);
    cout << " = ";
    m_expression->traduitEnCPP(cout, 0);
}

void NoeudOperateurBinaire::traduitEnCPP(ostream &cout, unsigned int indentation) const {
    if(m_operandeGauche != nullptr) m_operandeGauche->traduitEnCPP(cout, 0);
    cout << m_operateur.getChaine();
    if(m_operandeDroit != nullptr) m_operandeDroit->traduitEnCPP(cout, 0);
}

void NoeudInstSi::traduitEnCPP(ostream &cout, unsigned int indentation) const {

    cout << setw(4*indentation);

    // Première condition & séquence
    cout << "if(";
    m_conditions[0]->traduitEnCPP(cout, 0);
    cout << ") {" << endl;
    m_sequences[0]->traduitEnCPP(cout, indentation+1);
    cout << endl;

    // Traduction des else if
    int i = 1;
    while(i < m_conditions.size()){
        cout << setw(4*indentation);
        cout << " } else if(";
        m_conditions[i]->traduitEnCPP(cout, 0);
        cout << ") {" << endl;
        m_sequences[i]->traduitEnCPP(cout, indentation+1);
        cout << endl;
    }
    cout << setw(4*indentation);
    cout << " }";

    // Traduction du else s'il existe
    if(m_sequences.size() > m_conditions.size()) {
        cout << " else {" << endl;
        m_sequences[i]->traduitEnCPP(cout, indentation+1);
        cout << endl;
        cout << setw(4*indentation);
        cout << "}";
    }
}

void NoeudInstTq::traduitEnCPP(ostream &cout, unsigned int indentation) const {
    cout << setw(4*indentation+2);
    cout << "while(";
    m_condition->traduitEnCPP(cout, 0);
    cout << ") {" << endl;
    m_sequence->traduitEnCPP(cout, indentation+1);
    cout << setw(indentation+3);
    cout << "}";
}

void NoeudInstRepeter::traduitEnCPP(ostream &cout, unsigned int indentation) const {
    cout << setw(4*indentation);
    cout << "do {" << endl;
    m_sequence->traduitEnCPP(cout, indentation+1);
    cout << setw(2*indentation) << "" << "} while(!(";
    m_condition->traduitEnCPP(cout, 0);
    cout << "));";
}

void NoeudInstPour::traduitEnCPP(ostream &cout, unsigned int indentation) const {
    cout << setw(4*indentation);
    cout << "for(";
    if(m_affectation){ // Si l'affectation est définie on la traduit
        m_affectation->traduitEnCPP(cout, 0);
    }
    cout << "; ";
    m_condition->traduitEnCPP(cout, 0);
    cout << "; ";
    if(m_incrementation){ // Si l'incrémentation est définie on la traduit
        m_incrementation->traduitEnCPP(cout, 0);
    }
    cout << ") {" << endl;
    m_sequence->traduitEnCPP(cout, indentation+1);
    cout << setw(2*indentation+1);
    cout << "}";
}

void NoeudInstEcrire::traduitEnCPP(ostream &cout, unsigned int indentation) const {
    cout << setw(2*indentation) << "" << "std::cout ";
    for(Noeud * exp : m_expressions){
        cout << "<< ";
        exp->traduitEnCPP(cout, 0);
    }
    cout << ";";
}


NoeudInstLire::NoeudInstLire () {}

void NoeudInstLire::ajouterVariable(Noeud *var) {
    m_variables.push_back(var);
}

int NoeudInstLire::executer() {

    int var;

    for (auto variable : m_variables){
        cin >> var;
        ((SymboleValue*) variable)->setValeur(var);
    }

    return 0;
}