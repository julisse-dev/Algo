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
//avec un autre couple fait sens, car tout couple sera toujours plus proche qu'un point ou un vide. 
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
//la fonction jusqu'à ce que nuage.size()<=3, et dans ce cas on appelle rechercheNaive. On remonte ensuite les informations. On traite enfin le cas 
//de la bande centrale.
//On pose comme convention que le tableau donné est des indices debut à fin-1, l'indice fin n'est pas inclus.
couplePondere rechercheOptimisee(
    const std::array<std::vector<point>, DIMENSION>& nuageTrieParVar, 
    std::size_t debut, 
    std::size_t fin)
{

    std::size_t N = fin - debut;

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
    
    //REGNER : comparaison des résultats obtenus et return du min

    float delta;
    bool gaucheDroite; //Encode si le couple minimal est à gauche (false) ou à droite (true)
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
    
    //PRINCIPE : On va inclure dans le vector bandeCentrale que les points dont le carré de l'ordonnée est inférieure à detla (cf. commentaire distance, delta =d(a,b)²)
    //On va ensuite rechercher exhaustivement (rechercheNaive) pour chaque point et ses 7 voisins suivants (triés par ordonnée) s'il existe un couple de distance 
    //inférieure. Cela n'augmente pas la complexité, car 8 (=1+7) est constant, et le coût de rechercheNaive est donc lui aussi constant dans ce cadre.
    //On se contente de choisir 7 points suivant par un argument géométrique
    //(source : 2ème page, http://tnsi.free.fr/documents/10.3.%20Diviser_pour_regner_points_plus_proches.pdf)
    
    for (std::size_t i = 0; i<N; i++) //On détermine les points dans la bande centrale, et on les prend triés par ordonnée.
    {
        if (std::abs((nuageTrieParVar[1][debut+i][0] - cdMediane)*(nuageTrieParVar[1][debut+i][0] - cdMediane))<delta)
        {
            bandeCentrale.push_back(nuageTrieParVar[1][debut+i]);
        }
    }
    std::size_t M = bandeCentrale.size();
    couplePondere coupleCentral;
    coupleCentral.min = INF;
    coupleCentral.x[0] = INF;
    coupleCentral.x[1] = INF;
    coupleCentral.y[0] = INF;
    coupleCentral.y[1] = INF;
    
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
    
    couplePondere temp;
    std::vector<point> septPlusProchesVoisins(8);
    
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

    //Recollement des résulats en fonction d'où est le min
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
    point a = {0, 0};
    point b = {0, 1};  // paire optimale : (a,b), distance² = 1
    point c = {0, 5};
    point d = {3, 0};
    point e = {6, 0};
    
    std::vector<point> nuage = {a,b,c,d,e};
    
    std::array<std::vector<point>, DIMENSION> nuageTrieParVar;
    std::size_t N = 5;
    for (int i = 0; i<DIMENSION; i++)
    {
        nuageTrieParVar[i] = triABulles(nuage, i);
    }
    couplePondere coupleNaif = rechercheNaive(nuage);
    couplePondere coupleOpti = rechercheOptimisee(nuageTrieParVar, 0, N);
    
    //Rendus de rechercheNaive vis à vis de rechercheOpti
    std::cout << "(" << coupleNaif.x[0] << "," << coupleNaif.x[1] << ")";
    std::cout << "(" << coupleNaif.y[0] << "," << coupleNaif.y[1] << ")"<<std::endl;
    std::cout << "(" << coupleOpti.x[0] << "," << coupleOpti.x[1] << ")";
    std::cout << "(" << coupleOpti.y[0] << "," << coupleOpti.y[1] << ")"<<std::endl;


    return 0;
}
