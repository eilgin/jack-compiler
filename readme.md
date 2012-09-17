# Compilateur Jack
=====

## A propos
=====

Ce projet est une impl�mentation possible de compilateur *front-end* (lexer + parser + g�n�ration de *bytecode* Jack) pour la machine virtuelle [Hack](http://www1.idc.ac.il/tecs/).
Il traduit les fichiers sources �crit en Jack vers une machine virtuelle appel�e "Jack VM".
Le but est de proposer une solution possible au projet du chap�tre 10 & 11 du livre "The elements of Computing Elements" :)

## Pr�-requis
=====

le *back-end* � �t� �crit en C++ et utilise la biblioth�que [Boost 1.5.0](http://www.boost.org/) (notamment Boost::filesystem).
Le projet est regroup� au sein d'une solution [Visual Studio 2010](http://www.microsoft.com/visualstudio/eng/downloads#d-2010-express).

## Utilisation
=====

Les fichiers sources se trouvent � la racine. La solution *VS* se trouve dans le dossier **JackCompiler**.
Les batteries de *test* se trouvent dans le dossier �ponyme (vous trouverez un fichier HTML en anglais qui vous guidera).
A noter que le chemin vers les tests se trouvent dans les propri�t�s du projet
(Une fois la solution ouverte, faites un clic droit sur le projet *JackCompiler* puis `Propri�t�s` > `Propri�t�s de configuration` > `D�bogage`).
Vous pouvez y mettre par exemple dans `R�pertoire de travail` ceci `$(SolutionDir)..\test\p11\` et dans `Arguments de la commande` cela `6ComplexArrays`
Par ailleurs, n'oubliez pas de pr�ciser le chemin vers la librairie boost (Include et Librairies) dans les `Propri�t�s de configuration` (`R�pertoires VC++`).