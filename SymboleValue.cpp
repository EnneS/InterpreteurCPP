#include "SymboleValue.h"
#include "Exceptions.h"
#include <stdlib.h>

SymboleValue::SymboleValue(const Symbole & s) :
Symbole(s.getChaine()) {
  if (s == "<ENTIER>") {
    m_valeur = atoi(s.getChaine().c_str()); // c_str convertit une string en char*
    m_defini = true;
  } else if(s == "<CHAINE>"){ // Si le symbole est une chaine
    m_chaine = s.getChaine(); // On récupère le contenu de cette chaine
    m_defini = true;
  } else {
    m_defini = false;
  }
}

int SymboleValue::executer() {
  if (!m_defini) throw IndefiniException(); // on lève une exception si valeur non définie
  return m_valeur;
}

ostream & operator<<(ostream & cout, const SymboleValue & symbole) {
  cout << (Symbole) symbole << "\t\t - Valeur=";
  if (symbole.m_defini && symbole.m_chaine!= "") cout << symbole.m_chaine << " "; // Si le symbole est défini et que la chaine n'est pas vide alors on affiche la chaine
                                                                                  // Ca marche mais à moitié, quand on affecte une chaine à une variable, on récupère sa valeur en int
                                                                                  // au lieu d'avoir la string...
  else if(symbole.m_defini) cout << symbole.m_valeur << " "; // Sinon si le symbole est défini alors ce sera un entier, on affiche la valeur
  else cout << "indefinie ";
  return cout;
}

void SymboleValue::traduitEnCPP(ostream &cout, unsigned int indentation) const {
  cout << getChaine();
}