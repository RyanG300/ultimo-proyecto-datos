#incluide <iostream>
#include <map>

using namespace std;

struct verticeOrigen{
    string nombreOrigen;
    string aeropuerto;
    string puerto;
    string frontera;

    struct Vertice * sigV;//para enlazar todos los vertices en una lista

    struct Arco *subListaArcos;//representa los arcos que salen de ese vertice

    bool visitado;
    Vertice (string nom,string aer,string puer,string fron){
            nombreOrigen = nom;
            aeropuerto =aer;
            puerto=puer;
            frontera =fron;
            sigV = NULL;
            subListaArcos = NULL;
            visitado = false;
    }
};

struct arcoRuta{
    int horasDeRuta;
    string medioDeTransporte;
    struct Arco *sigA;//siguiente arco.
    Arco(int hor, string med){
            horasDeRuta = hor;
            medioDeTransporte = med;
            sigA = NULL;
    }
};

struct Vertice *grafoRutas;// el apuntador inicial del grafo


int main(){




}
