#include <iostream>
#include <vector>
#include <array>
#include <limits>


const float inf = std::numeric_limits<float>::infinity();

struct arrete
{
    int direction;
    float poids;
};

using sommet = std::vector<arrete>;

using graphe = std::vector<sommet>;

float sommeVecteur(std::vector<float> v) //fonction réalisant la somme des éléments d'un std::vector
{
    float somme = 0;
    for (int i = 0; i < v.size(); i++)
    {
        somme += v[i];
    }
    return somme;
}

int distanceMin(const std::vector<float>& distance, const std::vector<bool>& nonVisite)
//On utilise des passages par référence pour s'éviter une copie complète des objets, qui ne seront de toute façon pas modifiés 
//("const" est une sécurité)
{
    float minimum = inf; 
    int sommetCherche = -1;
    for (int i = 0; i < distance.size(); i++)
    {
        if (distance[i] < minimum && nonVisite[i])
        {
            minimum = distance[i];
            sommetCherche = i;
        }
    }
    return sommetCherche;
}

std::vector<int> dijkstra(const graphe& g, int depart, int fin)
{
    //INITIALISATION
    int n = g.size(); //n est le nombre de sommets du graphe g 
    std::vector<float> distance(n); //tableau des distances minimales entre les sommets de g et le point de départ
    for (int i = 0; i < n; i++)
    {
        distance[i] = inf;
    }
    distance[depart] = 0;

    std::vector<int> predecesseur(n, -1);

    std::vector<bool> nonVisite(n, 1);

    bool continuer = true;
    while(continuer)
    {
        int s1 = distanceMin(distance, nonVisite);
        nonVisite[s1] = false;
        if (s1 == -1)
            break; //Ce cas n'est pas censé arriver si le graphe admet un chemin entre "debut" et "fin".

        //Mise à jour des distances entre s1 et s2:= les voisins de s1
        for (const arrete& a : g[s1])
        {
            int s2 = a.direction;
            float poids = a.poids;

            if (nonVisite[s2] && distance[s2] > distance[s1] + poids) //Un sommet déjà visité ne pourra pas être amélioré
            {
                distance[s2] = distance[s1] + poids;
                predecesseur[s2] = s1;
            }
        }

        //Cas d'arrêt : on continue si le chemin minimal n'a pas été atteint, ie que le sommet "fin" n'est pas visité
        continuer = nonVisite[fin]; 
    }

    //Reconsturction du chemin
    std::vector<int> A;
    int s = fin;
    int i = 0;
    while (s != depart)
    {
        A.insert(A.begin(), s);;
        s = predecesseur[s];
    }
    A.insert(A.begin(), depart);
    return A;
}

int main() //Exemple avec le graphe g proposé ci dessous
{
    graphe g = {
    // 0
    {
        {1, 2.0f},
        {2, 5.0f}
    },
    // 1
    {
        {2, 1.0f},
        {3, 2.0f}
    },
    // 2
    {
        {3, 3.0f},
        {4, 1.0f}
    },
    // 3
    {
        {4, 1.0f}
    },
    // 4
    {
        // pas de sorties
    }
};
    std::vector<int> A;
    A = dijkstra(g, 0, 4);
    for (int i = 0 ; i < A.size(); i++)
        std::cout << A[i] << std::endl;
    return 0;
}
