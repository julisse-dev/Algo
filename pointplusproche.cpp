#include <iostream>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

const int DIMENSION = 2;
const float INF = std::numeric_limits<float>::infinity();

using point = std::array<float, DIMENSION>;


struct couplePondere //Cette structure sera utilisé pour représenter efficacement le couple (actuel) avec la plus faible distance, et leur distance sans avoir à la recalculer
{
    point x;
    point y;
    float min;
};

float distance(point x, point y) //Calcule la distance euclidienne entre deux points x et y, selon la dimension choisie.
{
    float d = 0;
    for (int i = 0; i<DIMENSION; i++)
    {
        d += (x[i] - y[i])*(x[i] - y[i]);
    }
    return d;
}
// Remarque : La distance rendue est en fait le carré de la "vraie" distance. ceci est vrai car si a,b >0, a<b <=> a²<b² par croissance.
// Calculer la racine à chaque itération de distance est un calcul inutile. On la calculera qu'à la fin si besoin de l'afficher. 


//QUICKSORT
//Convention : "dernier" est le dernier indice du nuage, ie nuage va de premier à dernier exactement
int partitionnement(std::vector<point>& nuage, std::size_t premier, std::size_t dernier, std::size_t pivot, int numVar)
{
    point aux;
    aux = nuage[pivot];
    nuage[pivot] = nuage[dernier];
    nuage[dernier] = aux;
    int j = premier;
    for (std::size_t i = premier; i<dernier; i++)  
    {
        if (nuage[i][numVar]<=nuage[dernier][numVar])
        {
            aux = nuage[i];
            nuage[i] = nuage[j];
            nuage[j] = aux;
            j++;
        }
    }
    aux = nuage[dernier];
    nuage[dernier] = nuage[j];
    nuage[j] = aux;
    return j;
}

void tri_rapide(std::vector<point>& nuage, std::size_t premier, std::size_t dernier, int numVar)
{
    if (premier < dernier)
    {
        std::size_t pivot = premier;
        pivot = partitionnement(nuage, premier, dernier, pivot, numVar);
        if (pivot > premier) // évite le underflow
            tri_rapide(nuage, premier, pivot-1, numVar);
        tri_rapide(nuage, pivot+1, dernier, numVar);
}   
}





//On utilise pour l'instant un tri à bulles pour ranger les tableaux. C'est inefficace et devra être amélioré (prob. par un quicksort). O(n²)
std::vector<point> triABulles(std::vector<point> nuage, int numVar)
{
    std::size_t N = nuage.size();
    point aux;
    for (std::size_t i = N-1; i>0; i--)
    {
        for (std::size_t j = 0; j<i; j++)
        {
            if (nuage[j+1][numVar] < nuage[j][numVar])
            {
                aux = nuage[j+1];
                nuage[j+1]=nuage[j];
                nuage[j] = aux;
            }
        }
    }
    return nuage;
}

//Remarque sur les cas N=0, N=1. La fonction renvoie un couple partiellement initialisé, dont juste couplePondere.min = INF est placé. 
//Cela fonctionne dans l'algorithme, car le code n'as pas besoin de couplePondere.x/.y pour ses calculs dans la fonction rechercheOptimisee.
//De plus, si N=0 ou N=1, on recherche un couple dans un ensemble à 0 ou 1 point. Cela n'a pas de sens, et renvoyer l'infini pour comparer
//avec un autre couple fait sens, car tout couple sera toujours plus proche qu'un point où un vide. 
//Par soucis de concision, un garde a été rajouté dans la fonction rechercheOptimisee pour être certain que ce cas n'arrive pas.
couplePondere rechercheNaive(const std::vector<point>& liste) //N(N-1)/2 appels à distance. O(n²)
{
    std::size_t N = liste.size();
    couplePondere plusProcheActuel;
    plusProcheActuel.min = INF;
    for (std::size_t i = 0; i<N; i++) 
    {
        for (std::size_t j = i+1; j<N; j++)
        {
            float dist =distance(liste[i], liste[j]);
            if (dist<plusProcheActuel.min)
            {
                plusProcheActuel.x = liste[i];
                plusProcheActuel.y = liste[j];
                plusProcheActuel.min = dist;
            }
        }
    }
    return plusProcheActuel;
}

//Principe : On divise le tableau en deux parties, de tailles égales en utilisant la médiane verticale donnée par début et fin. On appelle récursivement
//la fonction jusqu'à ce que nuage.size()<=3, et dans ce cas on appelle rechercheNaive. On remonte ensuite les informations.
//On pose comme convention que le tableau donné est des indices debut à fin-1, l'indice fin n'est pas inclus.
couplePondere rechercheOptimisee(
    const std::array<std::vector<point>, DIMENSION>& nuageTrieParVar, 
    std::size_t debut, 
    std::size_t fin)

{
    std::size_t N = fin - debut;

    //Cas dégénéré
    if (N < 2)
    {
        couplePondere vide;
        vide.min = INF;
        return vide;
    }

    //Cas d'arrêt : N<=3. On fait une recherche exhaustive.
    if (N<4)
    {   
        std::vector<point> liste(N);
        for (int i = 0; i < N; i++)
        {
            liste[i] = nuageTrieParVar[0][debut+i];
        }
        return rechercheNaive(liste);
    }
        

    std::size_t medianeX = debut + N/2; 
    //RMQ : le type (size_t) impose que (fin+debut)/2 soit de type size_t. De plus, on écrit ainsi pour éviter l'overflow si 
    //debut et fin sont trop grand. Comme fin>debut par construction, et (debut+fin)/2 = debut + (fin-debut)/2, on évite l'overflow.
    //De plus, par construction, il y a floor(N/2) points à gauche de la droite verticale medianeX, et n-floor(n/2) points à droite.
 
    //DIVISER : Initialisation des nuages gauche et droite    
    couplePondere coupleGauche = rechercheOptimisee(nuageTrieParVar, debut, medianeX); //L'intérêt est de travailler avec le même tableau, nuageTrieParVar
    couplePondere coupleDroite = rechercheOptimisee(nuageTrieParVar, medianeX, fin); //Et de ne pas créer des sous-vecteurs pour éviter de charger la mémoire
    
    //REGNER : comparaison des résultats obtenus, cas de la bande centrale et return du min

    float delta;
    bool gaucheDroite; //Sert à enregistrer si le couple minimal est à gauche (false) ou à droite (true)
    if (coupleGauche.min<coupleDroite.min)
    {
        delta = coupleGauche.min;
        gaucheDroite = false;
    }
    else
    {
        delta = coupleDroite.min;
        gaucheDroite = true;
    }
    float cdMediane = nuageTrieParVar[0][medianeX][0]; //On met la médiane exactement au 1er point à droite, c'est suffisant
    std::vector<point> bandeCentrale; //On utilise un std::vector car on ne sait pas a priori le nombre d'éléments de la bande centrale. 


    for (std::size_t i = 0; i<nuageTrieParVar[1].size(); i++) //On détermine les points dans la bande centrale, et on les prend triés par ordonnée.
    {
        if (std::abs((nuageTrieParVar[1][i][0] - cdMediane)*(nuageTrieParVar[1][i][0] - cdMediane))<delta)
        {
            bandeCentrale.push_back(nuageTrieParVar[1][i]);
        }
    }
    std::size_t M = bandeCentrale.size();
    couplePondere coupleCentral;
    coupleCentral.min = INF;
    coupleCentral.x[0] = INF;
    coupleCentral.x[1] = INF;
    coupleCentral.y[0] = INF;
    coupleCentral.y[1] = INF;
    couplePondere temp;
    std::vector<point> septPlusProchesVoisins(8);

    if (M<2) //Garde évoqué dans les commentaires de rechercheNaive
    {
        if (gaucheDroite)
        {
            return coupleDroite;
        }
        else
        {
            return coupleGauche;
        }
    }

    if (M<7)
    {
        coupleCentral = rechercheNaive(bandeCentrale);
    }
    else
    {
        for (std::size_t i = 0; i<M-7; i++)
        {
            septPlusProchesVoisins[0] = bandeCentrale[i];
            for (std::size_t j = 1; j <8; j++)
            {
                septPlusProchesVoisins[j]= bandeCentrale[i+j];
            }
            temp = rechercheNaive(septPlusProchesVoisins);
            if (temp.min < coupleCentral.min)
            {
                coupleCentral = temp;
            }
        }
    }

    if ((coupleCentral.min)*(coupleCentral.min) < delta)
    {
        return coupleCentral;
    }
    else if (gaucheDroite)
    {
        return coupleDroite;
    }
    else 
    {
        return coupleGauche;
    }
}

int main()
{
    point a1  = {1.2,   3.4};
    point a2  = {-5.6,  7.8};
    point a3  = {9.0,  -1.2};
    point a4  = {-3.4,  5.6};
    point a5  = {7.8,  -9.0};
    point a6  = {-1.2,  3.4};
    point a7  = {5.6,  -7.8};
    point a8  = {-9.0,  1.2};
    point a9  = {3.4,  -5.6};
    point a10 = {-7.8,  9.0};
    point a11 = {2.3,   4.5};
    point a12 = {-6.7,  8.9};
    point a13 = {10.1, -2.3};
    point a14 = {-4.5,  6.7};
    point a15 = {8.9,  -10.1};
    point a16 = {-2.3,  4.5};
    point a17 = {6.7,  -8.9};
    point a18 = {-10.1, 2.3};
    point a19 = {4.5,  -6.7};
    point a20 = {-8.9,  10.1};
    point a21 = {0.5,   1.5};
    point a22 = {-4.4,  6.6};
    point a23 = {8.8,  -0.5};
    point a24 = {-2.2,  4.4};
    point a25 = {6.6,  -8.8};
    point a26 = {-0.5,  2.5};
    point a27 = {4.4,  -6.6};
    point a28 = {-8.8,  0.5};
    point a29 = {2.2,  -4.4};
    point a30 = {-6.6,  8.8};
    point a31 = {1.1,   2.2};  // paire proche
    point a32 = {1.2,   2.3};  // paire proche

    std::vector<point> nuage = {a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29,a30,a31,a32};
    std::size_t N = 32;

    std::array<std::vector<point>, DIMENSION> nuageTrieParVar;

    for (int i = 0; i<DIMENSION; i++)
    {
        nuageTrieParVar[i] = nuage;
        tri_rapide(nuageTrieParVar[i], 0, N-1, i);
    }
    
    couplePondere coupleNaif = rechercheNaive(nuage);
    couplePondere coupleOpti = rechercheOptimisee(nuageTrieParVar, 0, N);

    std::cout << "(" << coupleNaif.x[0] << "," << coupleNaif.x[1] << ")";
    std::cout << "(" << coupleNaif.y[0] << "," << coupleNaif.y[1] << ")"<<std::endl;
    std::cout << "(" << coupleOpti.x[0] << "," << coupleOpti.x[1] << ")";
    std::cout << "(" << coupleOpti.y[0] << "," << coupleOpti.y[1] << ")"<<std::endl;
    

    return 0;
}