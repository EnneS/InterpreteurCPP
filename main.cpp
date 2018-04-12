#include <iostream>
using namespace std;
#include "Interpreteur.h"
#include "Exceptions.h"
#include <algorithm>

int main(int argc, char* argv[]) {
    string nomFich;
    if (argc != 2) {
        cout << "Usage : " << argv[0] << " nom_fichier_source" << endl << endl;
        cout << "Entrez le nom du fichier que voulez-vous interpréter : ";
        getline(cin, nomFich);
    } else
        nomFich = argv[1];
    ifstream fichier(nomFich.c_str());
    try {
        Interpreteur interpreteur(fichier);
        interpreteur.analyse();
        // Si pas d'exception levée, l'analyse syntaxique a réussi
        cout << endl << "================ Syntaxe Correcte" << endl;
        // On affiche le contenu de la table des symboles avant d'exécuter le programme
        cout << endl << "================ Table des symboles avant exécution : " << interpreteur.getTable();
        cout << endl << "================ Execution de l'arbre" << endl;
        // On exécute le programme si l'arbre n'est pas vide
        if (interpreteur.getArbre()!=nullptr) interpreteur.getArbre()->executer();
        // Et on vérifie qu'il a fonctionné en regardant comment il a modifié la table des symboles
        cout << endl << "================ Table des symboles apres exécution : " << interpreteur.getTable();

        if(interpreteur.getArbre()!= nullptr){
            // Définition du nom du fichier (enlever l'extension.., les slashs ...)
            size_t lastindexExt = nomFich.find_last_of("."); // extension
            string nomFichTrad = nomFich.substr(0, lastindexExt);
            nomFichTrad.erase(std::remove(nomFichTrad.begin(), nomFichTrad.end(), '.'), nomFichTrad.end());
            nomFichTrad.erase(std::remove(nomFichTrad.begin(), nomFichTrad.end(), '/'), nomFichTrad.end());
            nomFichTrad = nomFichTrad + string(".cpp"); // Et rajout de l'extension .cpp

            ofstream coutF (nomFichTrad); // Sortie avec le nom de fichier .cpp

            cout << endl << "================ Traduction en C++ ..." << endl;
            //interpreteur.traduitEnCPP(coutF, 1);
            cout << "================ Traduction récupérée dans le fichier " << nomFichTrad;
        }

    } catch (InterpreteurException & e) {
        cout << e.what() << endl;
    }
    return 0;
}
