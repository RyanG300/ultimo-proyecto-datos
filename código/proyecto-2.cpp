#include <iostream>
#include <map>
#include "header\json.hpp"
#include <fstream>
#include <unistd.h>


#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

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
verticeOrigen *   buscarVertice(string origen){ //Si encuentra el vertice en el grafo lo devuelve, sino devuelve NULL
        verticeOrigen *tempV = grafoRutas;
        while(tempV != NULL){
            if(tempV->nombreOrigen == origen)
                return tempV;

            tempV = tempV->sigV;
        }
    return NULL;//no lo encontro
}

// 2)
void clearScreen(){ //Borrar pantalla, determina si el programa se esta corriendo desde windows o no. Asi para evitar problemas.
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// 3)
bool comprobarNombreGrafo(string nombre){
    verticeOrigen*temp=grafoRutas;
    while(temp!=NULL){
        if(temp->nombreOrigen==nombre){
            return false;
        }
        temp=temp->sigV;
    }
    return true;
}


// 3) //Duerme
/*void s_sleep(int tiempo){
    #ifdef _WIN32
        Sleep(tiempo);
    #else
        sleep(tiempo);
    #endif
}*/

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
            puntoDeEntrada*listaEntradas[3];
            for(int i=0;i<dataDos["puntosDeEntrada"].size();i++){
                listaEntradas[i]=new puntoDeEntrada(dataDos["puntosDeEntrada"][i]["nombre"],dataDos["puntosDeEntrada"][i]["tipo"]);
            }
            grafoRutas=new verticeOrigen(dataDos["nombre"],listaEntradas);
        }
        else{
            verticeOrigen*temp=grafoRutas;
            while(temp->sigV!=NULL){
                temp=temp->sigV;
            }
            puntoDeEntrada*listaEntradas[3];
            for(int i=0;i<dataDos["puntosDeEntrada"].size();i++){
                listaEntradas[i]=new puntoDeEntrada(dataDos["puntosDeEntrada"][i]["nombre"],dataDos["puntosDeEntrada"][i]["tipo"]);
            }
            temp->sigV=new verticeOrigen(dataDos["nombre"],listaEntradas);
        }
    }

}

void crearGrafoJsonArcos(){ //Función auxiliar a "crearGrafoJsonVertices", encargada de crear las rutas con "arcoRuta".
    ifstream jsonFilePrueba("json\\DatosDestinoRuta.json");
    nlohmann::json dataJson = nlohmann::json::parse(jsonFilePrueba);
    for(int i=0;i<dataJson["rutasArco"].size();i++){
        nlohmann::json dataDos =dataJson["rutasArco"][i];

        //Por si esta mal escrito o no existe el destino o origen en el grafoRutas
        verticeOrigen *vOrigen = buscarVertice(dataDos["origen"]["nombre"].get<std::string>());
        verticeOrigen *vDestino = buscarVertice(dataDos["destino"]["nombre"].get<std::string>());

        if(vOrigen == NULL){
            cout<<"\nNo se encuentra el origen.";
            return;
        }
        if(vDestino == NULL){
            cout<<"\nNo se encuentra el destino.";
            return;
        }

        arcoRuta*arco=new arcoRuta(dataDos["horas"].get<unsigned int>(),dataDos["transporte"].get<std::string>(),dataDos["origen"]["nombre"].get<std::string>(),dataDos["origen"]["punto de entrada"].get<std::string>(),dataDos["destino"]["nombre"].get<std::string>(),dataDos["destino"]["punto de entrada"].get<std::string>());

        arco->sigA=vOrigen->subListaArcos;
        vOrigen->subListaArcos=arco;
    }
}

// 2)
//Agregar y eliminar vértices.
void agregarVertice(string nombreOrigen,puntoDeEntrada*fronteras[3]){ //Agrega un vertice al grafo, sin cambiar el archivo json "DatosDestinoRuta.json"
    verticeOrigen *nuevoVertice = new verticeOrigen(nombreOrigen,fronteras);
    nuevoVertice->sigV = grafoRutas;
    grafoRutas = nuevoVertice;
}



//---------------------------------------------MAIN-------------------------------------------------
//---------------------------------------------MAIN-------------------------------------------------
//---------------------------------------------MAIN-------------------------------------------------



int main(){
    int opcion1;
    bool carga=false;
    while(true){
        clearScreen();
        std::cout<<"     ==============================================      "<<std::endl;
        std::cout<<"                     VIAJES Y TAL S.A                    "<<std::endl;
        std::cout<<"     ==============================================      "<<std::endl<<std::endl;

        std::cout<<"Bienvenido usuario, por favor seleccione la opción deseada: ";
        std::cout<<std::endl<<"Gestión de datos (1). "
                <<std::endl<<"Reportes (2). "
                <<std::endl<<"Consultas (3). "
                <<std::endl<<"Inserte la opción deseada: ";
        std::cin>>opcion1;
        std::cin.ignore(10000,'\n');

        switch (opcion1){
        case 1:{ //Gestión de datos (1)
            int opcion2;
            while(true){
                clearScreen();
                std::cout<<"     ==============================================      "<<std::endl;
                std::cout<<"                    Gestión de datos                     "<<std::endl;
                std::cout<<"     ==============================================      "<<std::endl<<std::endl;

                std::cout<<"Aqui puedes modificar los datos del sistema de viajes. Por favor digite una opción: "
                    <<std::endl<<"Cargar el grafo guardado en el archivo DatosDestinoRuta.json (1). "
                    <<std::endl<<"Agregar y eliminar destinos (Vértices) (2). "
                    <<std::endl<<"Agregar, eliminar y modificar arcos (3). "
                    <<std::endl<<"Guardar el grafo actualizado en un archivo JSON (4). "
                    <<std::endl<<"Cargar datos de los usuarios desde el archivo clientesDatos.json (5)."
                    <<std::endl<<"Agregar, eliminar y buscar clientes (6)."
                    <<std::endl<<"Guardar los datos de los usuarios en el archivo JSON (7)."
                    <<std::endl<<"Crear, agregar, modificar y borrar en la lista simple de premios (8)."
                    <<std::endl<<"Registrar destino y puntos de viaje a un cliente (9)."
                    <<std::endl<<"Registrar premio obtenido a un cliente (10)."
                    <<std::endl<<"Volver (11)."
                    <<std::endl<<"Inserte la opción deseada: ";
                
                std::cin>>opcion2;
                std::cin.ignore(10000,'\n');

                switch(opcion2){ 
                    case 1:{ //Cargar el grafo guardado en el archivo DatosDestinoRuta.json (1).
                        clearScreen();
                        if(!carga){
                            crearGrafoJsonVertices();
                            crearGrafoJsonArcos();
                            std::cout<<"Carga completada, volviendo al menú."<<std::endl;
                            carga=true;
                        }
                        else{
                            std::cout<<"Datos anteriormente cargados, volviendo al menú."<<std::endl;
                        }
                        sleep(2);
                        continue;
                    }
                    case 2:{ //Agregar y eliminar vértices (2).
                        string nombre;
                        int repeticiones;
                        while(true){
                            clearScreen();
                            std::cout<<"Inserte el nombre del nuevo destino (El nombre no puede ser repetido): "<<std::endl;
                            getline(std::cin,nombre);
                            if(comprobarNombreGrafo(nombre)){ //Comprobamos si el nombre del destino que insertó el usuario no está repetido
                                while(true){
                                    //Por aquí insertamos todas las fronteras que tiene nuestro destino (vértice)
                                    clearScreen();
                                    std::cout<<"Cuantas fronteras tiene nuestro destino (max 3): "<<std::endl;
                                    std::cin>>repeticiones;
                                    std::cin.ignore(10000,'\n');
                                    if(repeticiones<=3 && repeticiones>0){
                                        puntoDeEntrada*fronteras[3];
                                        string nombreFrontera;
                                        string tipoFrontera;
                                        int cualTipo;
                                        for(int r=0;r<3;r++){
                                            clearScreen();
                                            std::cout<<"Cual es el nombre del punto de entrada. "<<std::endl;
                                            getline(std::cin,nombreFrontera);
                                            while(true){ //Aquí establecemos el tipo de la frontera
                                                std::cout<<"Inserte el tipo de entrada "
                                                    <<std::endl<<"Terminal/frontera (1)."
                                                    <<std::endl<<"Aeropuerto (2)."
                                                    <<std::endl<<"Muelle (3)."<<std::endl;
                                                std::cin>>cualTipo;
                                                std::cin.ignore(10000,'\n');
                                                switch (cualTipo){
                                                case 1:{
                                                    tipoFrontera="Terminal/frontera";
                                                    break;
                                                }
                                                case 2:{
                                                    tipoFrontera="Aeropuerto";
                                                    break;
                                                }
                                                case 3:{
                                                    tipoFrontera="Muelle";
                                                    break;
                                                }
                                                default:{
                                                    clearScreen();
                                                    std::cout<<"Tipo inválido, por favor intente de nuevo."<<std::endl;
                                                    sleep(2);
                                                    clearScreen();
                                                    continue;
                                                }
                                                }
                                            }
                                            fronteras[r]=new puntoDeEntrada(nombreFrontera,tipoFrontera);
                                            repeticiones--;
                                            if(repeticiones==0){
                                                break;
                                            }
                                        }
                                        agregarVertice(nombre,fronteras);
                                        string cualquieraTecla; 
                                        std::cout<<"Destino (vértice) añadido con éxito"<<std::endl;
                                        std::cout<<"Inserte enter para continuar: "<<std::endl;
                                        getline(std::cin,cualquieraTecla);
                                        break;

                                    }
                                    else{
                                        clearScreen();
                                        std::cout<<"Cantidad de destinos inválida, volviendo..."<<std::endl;
                                        sleep(2);
                                    }
                                }
                                break;
                            }
                            else{
                                clearScreen();
                                std::cout<<"Nombre inválido, por favor intentelo de nuevo."<<std::endl;
                                sleep(2);
                            }

                        } 
                        continue; 
                    }
                    case 3:{ //Agregar, eliminar y modificar arcos (3).
                        continue;
                    }
                    case 4:{ //Guardar el grafo actualizado en un archivo JSON (4).
                        continue;
                    }
                    case 5:{ //Cargar datos de los usuarios desde el archivo clientesDatos.json (5).
                        continue;
                    }
                    case 6:{ //Agregar, eliminar y buscar clientes (6).
                        continue;
                    }
                    case 7:{ //Guardar los datos de los usuarios en el archivo JSON (7).
                        continue;
                    }
                    case 8:{ //Crear, agregar, modificar y borrar en la lista simple de premios (8).
                        continue;
                    }
                    case 9:{ //Registrar destino y puntos de viaje a un cliente (9).
                        continue;
                    }
                    case 10:{ //Registrar premio obtenido a un cliente (10).
                        continue;
                    }
                    case 11:{ //Volver (11).
                        break;                
                    }
                }
            }
            continue;

        }
        case 2:{ //Reportes (2)
            continue;
        }
        case 3:{ //Consultas (3)
            continue;
        }
        default: {
            clearScreen();
            std::cout<<"Opción invalida, volviendo al menú."<<std::endl;
            sleep(2);
            continue;
        }

        }
    }
    /*ifstream jsonFilePrueba("json\\DatosDestinoRuta.json");
    nlohmann::json dataJson = nlohmann::json::parse(jsonFilePrueba);

    for(int a=0;a<dataJson["destinosVertice"].size();a++){
        nlohmann::json dataDos =dataJson["destinosVertice"][a];
        for(int e=0;e<dataDos.size();e++){
            std::cout<<dataDos[e]<<std::endl;
        }
    }*/


    //cout<<dataJson["destinosVertice"][0]["nombre"].get<std::string>()<<endl;
    //cout<<dataJson<<endl;
}
