#include <iostream>
#include <map>
#include "header\json.hpp"
#include <fstream>

using namespace std;

//----------------------------------------------STRUCT----------------------------------------------
//----------------------------------------------STRUCT----------------------------------------------
//----------------------------------------------STRUCT----------------------------------------------

struct puntoDeEntrada{
    string nombre;
    string tipo;

    puntoDeEntrada(string nom,string ti){
        nombre=nom;
        tipo=ti;
    }
};

struct verticeOrigen{ //string nombreOrigen / puntoDeEntrada*puntosDeEntrada[3]
    string nombreOrigen;
    puntoDeEntrada*puntosDeEntrada[3];

    struct verticeOrigen * sigV;//para enlazar todos los vertices en una lista

    struct arcoRuta *subListaArcos;//representa los arcos que salen de ese vertice

    bool visitado;
    verticeOrigen(string nom, puntoDeEntrada*puntsEntrada[3]){
        for(int i=0;i<3;i++){
            if(puntsEntrada[i]==NULL){
                continue;
            }
            else{
                puntosDeEntrada[i]=puntsEntrada[i];
            }
        }
        nombreOrigen = nom;
        sigV = NULL;
        subListaArcos = NULL;
        visitado = false;
    }
};

struct indicacionesRuta{
    string nombre;
    string puntoDeEntrada;
    indicacionesRuta(string nom,string pun){
        nombre=nom;
        pun=pun;
    }
};

struct arcoRuta{
    int horasDeRuta;
    string medioDeTransporte;
    struct arcoRuta *sigA;//siguiente arco.
    indicacionesRuta*origen;
    indicacionesRuta*destino;

    arcoRuta(int hor, string med,string nomb1,string pun1,string nomb2,string pun2){
        horasDeRuta = hor;
        medioDeTransporte = med;
        sigA = NULL;
        origen = new indicacionesRuta(nomb1,pun1);
        destino = new indicacionesRuta(nomb2,pun2);
    }
};
verticeOrigen *grafoRutas;// el apuntador inicial del grafo

//--------------------------------------FUNCIONES AUXILIARES----------------------------------------
//--------------------------------------FUNCIONES AUXILIARES----------------------------------------
//--------------------------------------FUNCIONES AUXILIARES----------------------------------------

// 1)
VerticeOrigen *   buscarVertice(string origen){ //Si encuentra el vertice en el grafo lo devuelve, sino devuelve NULL
        VerticeOrigen *tempV = grafoRutas;
        while(tempV != NULL){
            if(tempV->nombreOrigen == origen)
                return tempV;

            tempV = tempV->sigV;
        }
    return NULL;//no lo encontro
}


//---------------------------------------GESTIÓN DE DATOS-------------------------------------------
//---------------------------------------GESTIÓN DE DATOS-------------------------------------------
//---------------------------------------GESTIÓN DE DATOS-------------------------------------------

// 1)
//Crear el grafo en una representación multilista, tomando los datos (vértices y arcos) del archivo JSON.

void crearGrafoJsonVertices(){ //Función para inicializar el grafo "grafoRutas" con la información del json "DatosDestinoRuta.json". 
    ifstream jsonFilePrueba("json\\DatosDestinoRuta.json");
    nlohmann::json dataJson = nlohmann::json::parse(jsonFilePrueba);
    for(int a=0;a<dataJson["destinosVertice"].size();a++){
        nlohmann::json dataDos =dataJson["destinosVertice"][a];
        if(grafoRutas==NULL){
            puntoDeEntrada listaEntradas[3];
            for(int i=0;i<dataDos[1].size;i++){
                listaEntradas[i]=new puntoDeEntrada(dataDos[1][i]["nombre"],dataDos[1][i]["tipo"]);
            }
            grafoRutas=new verticeOrigen(dataDos["nombre"],listaEntradas);
        }
        else{
            verticeOrigen*temp=grafoRutas;
            while(temp->sigV!=NULL){
                temp=temp->sigV;
            }
            puntoDeEntrada listaEntradas[3];
            for(int i=0;i<dataDos[1].size;i++){
                listaEntradas[i]=new puntoDeEntrada(dataDos[1][i]["nombre"],dataDos[1][i]["tipo"]);
            }
            temp->sigV=new verticeOrigen(dataDos["nombre"],listaEntradas);
        }
    }

}

void crearGrafoJsonVertices(){ //Función auxiliar a "crearGrafoJsonVertices", encargada de crear las rutas con "arcoRuta".
    ifstream jsonFilePrueba("json\\DatosDestinoRuta.json");
    nlohmann::json dataJson = nlohmann::json::parse(jsonFilePrueba);
    for(int i=0;i<dataJson["rutasArco"].size;i++){
        nlohmann::json dataDos =dataJson["rutasArco"][a];

        //Por si esta mal escrito o no existe el destino o origen en el grafoRutas
        verticeOrigen *vOrigen = buscarVertice(dataDos["origen"]["nombre"]);
        verticeOrigen *vDestino = buscarVertice(dataDos["destino"]["nombre"]);
        
        if(vOrigen == NULL){
            cout<<"\nNo se encuentra el origen.";
            return;
        }
        if(vDestino == NULL){
            cout<<"\nNo se encuentra el destino.";
            return;
        }

        arcoRuta arco=new arcoRuta(dataDos["horas"],dataDos["transporte"],dataDos["origen"]["nombre"],dataDos["origen"]["punto de entrada"],dataDos["destino"]["nombre"],dataDos["destino"]["punto de entrada"])

        arco->sigA=vOrigen->subListaArcos;
        vOrigen->subListaArcos=arco;
    }
}








int main(){
    ifstream jsonFilePrueba("json\\DatosDestinoRuta.json");
    nlohmann::json dataJson = nlohmann::json::parse(jsonFilePrueba);

    for(int a=0;a<dataJson["destinosVertice"].size();a++){
        nlohmann::json dataDos =dataJson["destinosVertice"][a];
        for(int e=0;e<dataDos.size();e++){
            std::cout<<dataDos[e]<<std::endl;
        }
    }


    //cout<<dataJson["destinosVertice"][0]["nombre"].get<std::string>()<<endl;
    //cout<<dataJson<<endl;

}
