cmake_minimum_required(VERSION 3.9)
project(M3105_TP5)

set(CMAKE_CXX_STANDARD 11)

add_executable(M3105_TP5 main.cpp ArbreAbstrait.cpp ArbreAbstrait.h Exceptions.h Interpreteur.cpp Interpreteur.h Lecteur.cpp Lecteur.h Symbole.cpp Symbole.h SymboleValue.cpp SymboleValue.h TableSymboles.cpp TableSymboles.h)