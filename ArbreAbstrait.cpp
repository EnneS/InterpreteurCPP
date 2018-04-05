#include <stdlib.h>
#include "ArbreAbstrait.h"
#include "Symbole.h"
#include "SymboleValue.h"
#include "Exceptions.h"

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

NoeudInstTq::NoeudInstTq(Noeud* condition, Noeud* sequence)
        : m_condition(condition), m_sequence(sequence) {
}

int NoeudInstTq::executer() {
  while(m_condition->executer()) {
    m_sequence->executer();
  }
  return 0;
}

NoeudInstRepeter::NoeudInstRepeter(Noeud * condition, Noeud * sequence)
        : m_condition(condition), m_sequence(sequence) {
}

int NoeudInstRepeter::executer(){
    do {
        m_sequence->executer();
    } while(!m_condition->executer());

    return 0;
}

NoeudInstPour::NoeudInstPour(Noeud *affectation, Noeud *condition, Noeud *incrementation, Noeud *sequence)
        : m_affectation(affectation), m_condition(condition), m_incrementation(incrementation), m_sequence(sequence) {}

int NoeudInstPour::executer(){
    cout << m_condition->executer();

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