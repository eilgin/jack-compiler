# Compilateur Jack
=====

## A propos
=====

Ce projet est une implémentation possible de compilateur *front-end* (lexer + parser + génération de *bytecode* Jack) pour la machine virtuelle [Hack](http://www1.idc.ac.il/tecs/).
Il traduit les fichiers sources écrit en Jack vers une machine virtuelle appelée "Jack VM".
Le but est de proposer une solution possible au projet du chapître 10 & 11 du livre "The elements of Computing Elements" :)

## Pré-requis
=====

le *back-end* à été écrit en C++ et utilise la bibliothèque [Boost 1.5.0](http://www.boost.org/) (notamment Boost::filesystem).
Le projet est regroupé au sein d'une solution [Visual Studio 2010](http://www.microsoft.com/visualstudio/eng/downloads#d-2010-express).

## Utilisation
=====

Les fichiers sources se trouvent à la racine. La solution *VS* se trouve dans le dossier **JackCompiler**.
Les batteries de *test* se trouvent dans le dossier éponyme (vous trouverez un fichier HTML en anglais qui vous guidera).
A noter que le chemin vers les tests se trouvent dans les propriétés du projet
(Une fois la solution ouverte, faites un clic droit sur le projet *JackCompiler* puis `Propriétés` > `Propriétés de configuration` > `Débogage`).
Vous pouvez y mettre par exemple dans `Répertoire de travail` ceci `$(SolutionDir)..\test\p11\` et dans `Arguments de la commande` cela `6ComplexArrays`
Par ailleurs, n'oubliez pas de préciser le chemin vers la librairie boost (Include et Librairies) dans les `Propriétés de configuration` (`Répertoires VC++`).