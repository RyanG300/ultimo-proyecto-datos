#include <iostream>
#include <map>
#include "header/json.hpp"
#include <fstream>
#include <unistd.h>


#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

using namespace std;
using json = nlohmann::json;

//----------------------------------------------STRUCT & CLASS----------------------------------------------
//----------------------------------------------STRUCT & CLASS----------------------------------------------
//----------------------------------------------STRUCT & CLASS----------------------------------------------

struct puntoDeEntrada{
    string nombre;
    string tipo;

    puntoDeEntrada(string nom,string ti){
        nombre=nom;
        tipo=ti;
    }
};

struct verticeOrigen{ //string nombreOrigen / puntoDeEntrada*puntosDeEntrada[3] /Lista de vértices doble
    string nombreOrigen;
    puntoDeEntrada*puntosDeEntrada[3];
    int cantidadVisitas;

    struct verticeOrigen * sigV;//para enlazar todos los vertices en una lista
    verticeOrigen*antV;

    struct arcoRuta *subListaArcos;//representa los arcos que salen de ese vertice

    bool visitado;
    verticeOrigen(string nom, puntoDeEntrada*puntsEntrada[3]){
        for(int i=0;i<3;i++){
            if(puntsEntrada[i]==NULL){
                puntosDeEntrada[i]=NULL;
                continue;
            }
            else{
                puntosDeEntrada[i]=puntsEntrada[i];
            }
        }
        nombreOrigen = nom;
        sigV = NULL;
        antV = NULL;
        subListaArcos = NULL;
        visitado = false;
        cantidadVisitas=0;
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

struct arcoRuta{ //Lista de arcos doble
    int horasDeRuta;
    string medioDeTransporte;
    struct arcoRuta *sigA;//siguiente arco.
    arcoRuta*antA;
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

class Premio {
public:
    std::string nombre;
    int puntosMinimos;

    Premio(const std::string& nombre, int puntos) : nombre(nombre), puntosMinimos(puntos) {}
};

class Cliente {
public:
    string nombre;
    vector<string> destinosVisitados;
    int puntosAcumulados = 0;
    vector<string> premiosObtenidos;

    Cliente() = default;

    Cliente(const string& nombre) : nombre(nombre), puntosAcumulados(0) {}

    void agregarDestino(const string& destino, int horas, const string medio) {
        destinosVisitados.push_back(destino);
        acumularPuntos(horas, medio);
    }

    void acumularPuntos(int horas, string medio) {
        int puntos = 0;
        if (medio == "Avion") {
            puntos += horas * 100;
        } else if (medio == "Crucero") {
            puntos += horas * 70;
        } else if (medio == "Auto") {
            puntos += horas * 25;
        }

        if (destinosVisitados.size() > 15) {
            puntos *= 2;
        } else if (destinosVisitados.size() > 10) {
            puntos *= 1.5;
        } else if (destinosVisitados.size() >= 5) {
            puntos *= 1.2;
        }

        puntosAcumulados += puntos;
    }

    bool canjearPremio(const Premio& premio) {
        if (puntosAcumulados >= premio.puntosMinimos) {
            puntosAcumulados -= premio.puntosMinimos;
            premiosObtenidos.push_back(premio.nombre);
            return true;
        }
        return false;
    }

    json toJSON() const {
        return {
                {"nombre", nombre},
                {"destinosVisitados", destinosVisitados},
                {"puntosAcumulados", puntosAcumulados},
                {"premiosObtenidos", premiosObtenidos}
        };
    }
};

class GestorClientes {
private:
    std::unordered_map<string, Cliente> clientes;
    std::vector<Premio> premios;

public:
    void agregarCliente(const string& nombre) {
        clientes[nombre] = Cliente(nombre);
    }

    Cliente* buscarCliente(const string& nombre) {
        auto it = clientes.find(nombre);
        if (it != clientes.end()) {
            return &it->second;
        }
        return nullptr;
    }

    void eliminarCliente(const string& nombre) {
        auto it = clientes.find(nombre);
        if (it != clientes.end()) {
            clientes.erase(it);
        }
    }

    void registrarViaje(const string& nombreCliente, const string& destino, const int horas, const string& medio) {
        Cliente* cliente = buscarCliente(nombreCliente);
        if (cliente) {
            cliente->agregarDestino(destino, horas, medio);
        }
    }

    void canjearPremio(const std::string& nombreCliente, const std::string& nombrePremio) {
        Cliente* cliente = buscarCliente(nombreCliente);
        for (const Premio& premio : premios) {
            if (premio.nombre == nombrePremio) {
                if (cliente && cliente->canjearPremio(premio)) {
                    std::cout << "Premio canjeado con éxito.\n";
                } else {
                    std::cout << "No tiene puntos suficientes.\n";
                }
                return;
            }
        }
        std::cout << "Premio no encontrado.\n";
    }

    void guardarClientesJSON(const std::string& nombreArchivo) {
        json j;
        for (const auto& par : clientes) {
            j["json/clientesDatos"].push_back(par.second.toJSON());
        }
        std::ofstream archivo(nombreArchivo);
        archivo << j.dump(4);
    }

    void cargarClientesJSON(const string& nombreArchivo) {
        ifstream archivo(nombreArchivo);
        json j;
        archivo >> j;
        for (const auto& item : j["clientes"]) {
            Cliente cliente(item["nombre"]);
            cliente.puntosAcumulados = item["puntosAcumulados"];
            for (const auto& destino : item["destinosVisitados"]) {
                cliente.destinosVisitados.push_back(destino);
            }
            for (const auto& premio : item["premiosObtenidos"]) {
                cliente.premiosObtenidos.push_back(premio);
            }
            clientes[cliente.nombre] = cliente;
        }
    }

    void agregarPremio(const Premio& premio) {
        premios.push_back(premio);
    }
};

//Variables globales
verticeOrigen *grafoRutas;// el apuntador inicial del grafo
string tipoPuntoEntradaGlobal;

//--------------------------------------FUNCIONES AUXILIARES----------------------------------------
//--------------------------------------FUNCIONES AUXILIARES----------------------------------------
//--------------------------------------FUNCIONES AUXILIARES----------------------------------------

// 1) Si encuentra el vertice en el grafo lo devuelve, sino devuelve NULL
verticeOrigen *   buscarVertice(string origen){ 
        verticeOrigen *tempV = grafoRutas;
        while(tempV != NULL){
            if(tempV->nombreOrigen == origen)
                return tempV;

            tempV = tempV->sigV;
        }
    return NULL;//no lo encontro
}

// 2) Borrar pantalla, determina si el programa se esta corriendo desde windows o no. Asi para evitar problemas.
void clearScreen(){ 
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// 3) Comprueba la repetición de un vértice en el grafo
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

// 4) Busca el vertice por nombre y imprime sus puntos de entrada
void imprimirPuntosDeEntrada(string origen,bool bloquearDiferenteTipo=false){ 
    verticeOrigen *tempV = grafoRutas;
    int contador;
    while(tempV != NULL){
        if(tempV->nombreOrigen == origen){
            for(int e=0;e<=2;e++){
                if(tempV->puntosDeEntrada[e]!=NULL && !bloquearDiferenteTipo){
                    std::cout<<contador<<") Nombre: "<<tempV->puntosDeEntrada[e]->nombre<<std::endl;
                    std::cout<<"   Tipo: "<<tempV->puntosDeEntrada[e]->tipo<<std::endl;
                    contador++;
                }
                else if(tempV->puntosDeEntrada[e]!=NULL && bloquearDiferenteTipo){
                    if(tempV->puntosDeEntrada[e]->tipo==tipoPuntoEntradaGlobal){
                        std::cout<<contador<<") Nombre: "<<tempV->puntosDeEntrada[e]->nombre<<std::endl;
                        std::cout<<"   Tipo: "<<tempV->puntosDeEntrada[e]->tipo<<std::endl;
                        contador++;
                    }
                }
            }
            return;

        }
        tempV = tempV->sigV;
    }
    return;
}

// 5) Busca el punto de entrada de un vértice por nombre especifico
bool buscarPuntoDeEntradaEspecifico(string nombreVertice,string nombrePuntoEntrada){ 
    verticeOrigen *tempV = grafoRutas;
    while(tempV != NULL){
        if(tempV->nombreOrigen == nombreVertice){
            for(int e=0;e<3;e++){
                if(tempV->puntosDeEntrada[e]->nombre==nombrePuntoEntrada){
                    tipoPuntoEntradaGlobal=tempV->puntosDeEntrada[e]->tipo;
                    return true;
                }
            }
            return false;
        }
        tempV = tempV->sigV;
    }
    return false;
}

// 6) Cambia visitado de todos los verticeOrigen en grafoRutas 
void desmarcar(){ 
        verticeOrigen *tempV = grafoRutas;

        while(tempV != NULL){

            tempV->visitado = false;

            tempV = tempV->sigV;
        }
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
                listaEntradas[i]=new puntoDeEntrada(dataDos["puntosDeEntrada"][i]["nombre"].get<std::string>(),dataDos["puntosDeEntrada"][i]["tipo"].get<std::string>());
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
                listaEntradas[i]=new puntoDeEntrada(dataDos["puntosDeEntrada"][i]["nombre"].get<std::string>(),dataDos["puntosDeEntrada"][i]["tipo"].get<std::string>());
            }
            temp->sigV=new verticeOrigen(dataDos["nombre"].get<std::string>(),listaEntradas);
            temp->sigV->antV=temp;
        }
    }

}

//Función auxiliar a "crearGrafoJsonVertices", encargada de crear las rutas con "arcoRuta".
void crearGrafoJsonArcos(){ 
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
        if(vOrigen->subListaArcos!=NULL){
            arco->sigA->antA=arco;
        }
        vOrigen->subListaArcos=arco;
    }
}

// 2)
//Agregar y eliminar vértices.
void agregarVertice(string nombreOrigen,puntoDeEntrada*fronteras[3]){ //Agrega un vértice al grafo, sin cambiar el archivo json "DatosDestinoRuta.json"
    verticeOrigen *nuevoVertice = new verticeOrigen(nombreOrigen,fronteras);
    if(grafoRutas==NULL){
        grafoRutas=nuevoVertice;
    }
    else{
        verticeOrigen*temp=grafoRutas;
        while(temp->sigV!=NULL){
            temp=temp->sigV;
        }
        temp->sigV=nuevoVertice;
        nuevoVertice->antV = temp;
    }

}

//Elimina el vértice de grafo, junto a los arcos que apuntan a el
bool eliminarVertice(string nombreAEliminar){ 
    //Aquí borramos el vértice del grafo
    verticeOrigen*temp1=grafoRutas;
    bool eliminado=false;
    while(temp1!=NULL){
        if(temp1->nombreOrigen==nombreAEliminar){
            if(temp1->antV==NULL){
                grafoRutas=temp1->sigV;
                if(grafoRutas!=NULL){
                    grafoRutas->antV=NULL;
                }
                eliminado=true;
                break;
            }
            else if(temp1->sigV==NULL){
                temp1->antV->sigV=NULL;
                eliminado=true;
                break;
            }
            else{
                temp1->antV->sigV=temp1->sigV;
                temp1->sigV->antV=temp1->antV;
                eliminado=true;
                break;
            }
        }
        temp1=temp1->sigV;
    }
    if(!eliminado){
        return eliminado;
    }

    //Y aquí eliminamos los arcos que apuntan al vértice eliminado
    verticeOrigen*temp2=grafoRutas;
    while(temp2!=NULL){
        arcoRuta*tempArco=temp2->subListaArcos;
        while(tempArco!=NULL){
            if(tempArco->destino->nombre==nombreAEliminar){
                if(tempArco->antA==NULL){
                    temp2->subListaArcos=temp2->subListaArcos->sigA;
                    if(temp2->subListaArcos!=NULL){
                        temp2->subListaArcos->antA=NULL;
                    }
                }
                else if(tempArco->sigA==NULL){
                    tempArco->antA->sigA=NULL;
                }
                else{
                    tempArco->antA->sigA=tempArco->sigA;
                    tempArco->sigA->antA=tempArco->antA;
                }
            }
            tempArco=tempArco->sigA;
        }
        temp2=temp2->sigV;
    }
    return eliminado;
}

//Para mostrar el usuario los vertices disponibles para eliminar
void imprimirVertices(){ 
    verticeOrigen*temp=grafoRutas;
    int contador;
    while(temp!=NULL){
        std::cout<<contador<<") "<<temp->nombreOrigen<<std::endl;
        temp=temp->sigV;
        contador++;
    }
}

// 3)
// Agregar, eliminar y modificar arcos.
bool insertarArcos(string origen,int horasRuta,string medioDeTransporte,string nombrePuntoEntrada,string nombrePuntoLlegada,string destino){
    verticeOrigen *vOrigen = buscarVertice(origen);
    verticeOrigen *vDestino = buscarVertice(destino);
    if(vOrigen == NULL){
        //cout<<"\nNo se encuentra el origen.";
        return false;
    }
    if(vDestino == NULL){
        //cout<<"\nNo se encuentra el destino.";
        return false;
    }

    arcoRuta*arco=new arcoRuta(horasRuta,medioDeTransporte,origen,nombrePuntoEntrada,destino,nombrePuntoLlegada);
    arco->sigA=vOrigen->subListaArcos;
    if(vOrigen->subListaArcos!=NULL){
        arco->sigA->antA=arco;
    }
    vOrigen->subListaArcos=arco;
    return true;
}

//Recibe el string de origen y destino, comprueba si existe el arco entre esos vértices y elimina el arco.
bool eliminarArco(string origen,string destino,int indexRuta){ 
    verticeOrigen*temp=grafoRutas;
    int contador=0; //Index exacto de las rutas (arcos)
    std::cout<<"Prueba 1"<<std::endl;
    while(temp!=NULL){
        if(temp->nombreOrigen==origen){
            arcoRuta*tempArco=temp->subListaArcos;
            while(tempArco!=NULL){
                std::cout<<"Prueba 2"<<std::endl;
                if(tempArco->origen->nombre==origen && tempArco->destino->nombre==destino && contador==indexRuta){
                    std::cout<<tempArco->antA<<std::endl;
                    if(tempArco->antA==NULL){
                        std::cout<<"Prueba 5"<<std::endl;
                        temp->subListaArcos=temp->subListaArcos->sigA;
                        if(temp->subListaArcos!=NULL){
                            temp->subListaArcos->antA=NULL;
                        }
                        std::cout<<"Prueba 4"<<std::endl;
                    }
                    else if(tempArco->sigA==NULL){
                        std::cout<<"Prueba 6"<<std::endl;
                        tempArco->antA->sigA=NULL;
                    }
                    else{
                        std::cout<<"Prueba 7"<<std::endl;
                        tempArco->antA->sigA=tempArco->sigA;
                        tempArco->sigA->antA=tempArco->antA;
                    }
                    return true;
                }
                contador++;
                tempArco=tempArco->sigA;
            }
            return false;

        }
        temp=temp->sigV;
    }
    return false;
}

//Imprime todas las rutas que conecten los dos vértices recibidos
void imprimirRuta(string origen,string destino){ 
    verticeOrigen*temp=grafoRutas;
    int contador=0; //Index exacto de las rutas (arcos)
    while(temp!=NULL){
        if(temp->nombreOrigen==origen){
            arcoRuta*tempArco=temp->subListaArcos;
            while(tempArco!=NULL){
                if(tempArco->origen->nombre==origen && tempArco->destino->nombre==destino){
                    std::cout<<std::endl<<contador<<") origen: "<<tempArco->origen->nombre
                    <<std::endl<<"   destino: "<<tempArco->destino->nombre
                    <<std::endl<<"   transporte: "<<tempArco->medioDeTransporte<<std::endl;
                }
                tempArco=tempArco->sigA;
                contador++;
            }
            std::cout<<"Inserte la ruta a eliminar (num): ";
            return;
        }
        temp=temp->sigV;
    }
    std::cout<<"No hay rutas desde "<<origen<<" hacia "<<destino<<". Escriba 'salir' para volver: ";

}

//Modifica el tiempo de la ruta del arco escogido
bool modificarArco(string origen,string destino,int nuevoTiempo,int indexRuta){ 
    verticeOrigen*temp=grafoRutas;
    int contador=0;
    while(temp!=NULL){
        if(temp->nombreOrigen==origen){
            arcoRuta*tempArco=temp->subListaArcos;
            while(tempArco!=NULL){
                if(tempArco->origen->nombre==origen && tempArco->destino->nombre==destino && contador==indexRuta){
                    tempArco->horasDeRuta=nuevoTiempo;
                    return true;
                }
                contador++;
                tempArco=tempArco->sigA;
            }
            return false;
        }
        temp=temp->sigV;
    }
    return false;
}


// 4)
//Guardar el grafo actualizado en un archivo JSON.
void guardarGrafoRutaJson(){ //Guarda el grafo en el archivo json
    std::cout<<"Prueba"<<std::endl;
    nlohmann::json nuevoJson;
    verticeOrigen*temp=grafoRutas;
    int contador=0;
    int menos=0;
    // Iterar sobre los vértices del grafo
    while(temp!=NULL){
        nuevoJson["destinosVertice"][contador]["nombre"]=temp->nombreOrigen;
        for(int r=0;r<3;r++){
            if(temp->puntosDeEntrada[r]==NULL){
                menos++;
            }
            else if(temp->puntosDeEntrada==NULL){
                std::cout<<"IMPOSIBLE"<<std::endl;
            }
            else{
                std::cout<<"PruebaVertices1"<<std::endl;
                nuevoJson["destinosVertice"][contador]["puntosDeEntrada"][r]["nombre"]=temp->puntosDeEntrada[r-menos]->nombre;
                nuevoJson["destinosVertice"][contador]["puntosDeEntrada"][r]["tipo"]=temp->puntosDeEntrada[r-menos]->tipo;
                std::cout<<"PruebaVertices2"<<std::endl;
            }
        }
        // Iterar sobre los arcos de ruta
        arcoRuta*tempRuta=temp->subListaArcos;
        while(tempRuta!=NULL){
            // Json
            std::cout<<"PruebaArco1"<<std::endl;
            nlohmann::json arcoJson;
            //Origen
            arcoJson["origen"]["nombre"]=tempRuta->origen->nombre;
            //std::cout<<tempRuta->origen->puntoDeEntrada<<" pe1"<<std::endl;
            arcoJson["origen"]["punto de entrada"]=tempRuta->origen->puntoDeEntrada;
            //destino
            arcoJson["destino"]["nombre"]=tempRuta->destino->nombre;
            //std::cout<<tempRuta->destino->puntoDeEntrada<<" pe2"<<std::endl;
            arcoJson["destino"]["punto de entrada"]=tempRuta->destino->puntoDeEntrada;

            //Transporte
            arcoJson["rutasArco"]["transporte"]=tempRuta->medioDeTransporte;

            //Horas
            arcoJson["horas"]=tempRuta->horasDeRuta;

            // Agregar el arco a la lista de rutas
            nuevoJson["rutasArco"].push_back(arcoJson);

            tempRuta=tempRuta->sigA;
            std::cout<<"PruebaArco2"<<std::endl;
        }
        contador++;
        temp=temp->sigV;
        menos=0;
    }
    // Guardar el JSON en un archivo
    std::ofstream jsonOut("json\\prueba.json");
    std::cout<<"PruebaFINAL"<<std::endl;
    jsonOut << std::setw(4)<<nuevoJson;
}


//-------------------------------------------REPORTES-----------------------------------------------
//-------------------------------------------REPORTES-----------------------------------------------
//-------------------------------------------REPORTES-----------------------------------------------

// 1)
//Imprimir el grafo en amplitud, mostrar Origen Destino, medio y horas.
void imprimirGrafoAmplitud(){
    verticeOrigen*tempV =grafoRutas;
    int contador;
    while(tempV != NULL){//RECORRE LA LISTA DE VERTICES
        std::cout<<std::endl<<contador<<") Vertice:  "<<tempV->nombreOrigen<<std::endl;
        arcoRuta *tempA = tempV->subListaArcos;

        std::cout<<"Rutas: "<<std::endl;
        while(tempA != NULL){//RECORRE LOS ARCOS DE LA LISTA DE ARCOS DEL VERTICE
            std::cout<<tempA->origen->nombre<<" -> "<<tempA->destino->nombre<<" || Duración: "<<tempA->horasDeRuta<<" || Medio transporte: "<<tempA->medioDeTransporte<<std::endl;
            tempA = tempA->sigA;
        }
        tempV = tempV->sigV;
        contador++;
    }
}

// 2)
//Imprimir el grafo en profundidad, se debe indicar por el usuario final el inicio del recorrido, mostrar Origen Destino.
void profundidad ( verticeOrigen*inicio,int contador=0){
        if((inicio == NULL) || (inicio->visitado== true)){
                cout<<endl;
            return;
        }

        inicio->visitado = true;

        arcoRuta *tempA = inicio->subListaArcos;
        while(tempA != NULL){
            cout<<contador<<") "<<inicio->nombreOrigen<<" -> "<<tempA->destino->nombre<<" Tiempo: "<<tempA->horasDeRuta<<", ";
            
            profundidad(buscarVertice(tempA->destino->nombre),contador+1);

            tempA = tempA->sigA;
        }

}

// 7)
//Imprimir destinos del grafo que no hay recibido visitas.
void imprimirDestinosSinVisitas(){
    verticeOrigen*tempV=grafoRutas;
    int contador;

    while(tempV!=NULL){
        if(tempV->cantidadVisitas==0){
            std::cout<<contador<<") "<<tempV->nombreOrigen<<std::endl;
            contador++;
        }
        tempV=tempV->sigV;
    }
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
            bool salirDatos=false;
            while(true){
                if(salirDatos){
                    salirDatos=false;
                    break;
                }
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
                        int subOpcion;
                        clearScreen();
                        std::cout<<"Inserte la opción deseada: "
                            <<std::endl<<"Agregar vértice (1)."
                            <<std::endl<<"Eliminar vértice (2)."<<std::endl;

                        std::cin>>subOpcion;
                        std::cin.ignore(10000,'\n');
                        if(subOpcion==1){ //Agregar vértice (1)
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
                                            bool salir=false;
                                            for(int r=0;r<3;r++){
                                                clearScreen();
                                                std::cout<<"Cual es el nombre del punto de entrada (num: "<<r<<"). "<<std::endl;
                                                getline(std::cin,nombreFrontera);
                                                while(true){ //Aquí establecemos el tipo de la frontera
                                                    if(salir){
                                                        break;
                                                    }
                                                    std::cout<<"Inserte el tipo de entrada "
                                                        <<std::endl<<"Terminal/frontera (1)."
                                                        <<std::endl<<"Aeropuerto (2)."
                                                        <<std::endl<<"Muelle (3)."<<std::endl;
                                                    std::cin>>cualTipo;
                                                    std::cin.ignore(10000,'\n');
                                                    switch (cualTipo){
                                                    case 1:{
                                                        tipoFrontera="Terminal/frontera";
                                                        salir=true;
                                                        continue;
                                                    }
                                                    case 2:{
                                                        tipoFrontera="Aeropuerto";
                                                        salir=true;
                                                        continue;
                                                    }
                                                    case 3:{
                                                        tipoFrontera="Muelle";
                                                        salir=true;
                                                        continue;
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
                                                    if(r==0){
                                                        fronteras[1]=NULL;
                                                        fronteras[2]=NULL;
                                                    }
                                                    else if(r==1){
                                                        fronteras[2]=NULL;
                                                    }
                                                    break;
                                                }
                                            }
                                            agregarVertice(nombre,fronteras);
                                            string cualquieraTecla;
                                            std::cout<<"Destino (vértice) añadido con éxito"<<std::endl;
                                            std::cout<<"Inserte cualquier tecla para continuar para continuar: "<<std::endl;
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
                        if(subOpcion==2){ //Eliminar vértice (2)
                            while(true){
                                clearScreen();
                                if(grafoRutas!=NULL){
                                    std::cout<<"Destinos disponibles: "<<std::endl;
                                    imprimirVertices();
                                    std::cout<<"Inserte el nombre del vértice a eliminar, o por el contrario escriba salir: ";
                                    getline(std::cin,nombre);
                                    if(eliminarVertice(nombre)){
                                        clearScreen();
                                        std::cout<<"Eliminado con éxito, volviendo al menú..."<<std::endl;
                                        sleep(2);
                                        break;
                                    }
                                    else if(nombre=="salir"){
                                        break;
                                    }
                                    else{
                                        clearScreen();
                                        std::cout<<"El nombre del vértice a eliminar no existe..."<<std::endl;
                                        sleep(2);
                                    }

                                }
                                else{
                                    std::cout<<"Sin vértices en el grafo, volviendo al menú..."<<std::endl;
                                    sleep(2);
                                    break;
                                }
                            }
                            continue;
                        }
                        else{
                            clearScreen();
                            std::cout<<"Opción inválida, volviendo al menú..."<<std::endl;
                            sleep(2);
                            continue;
                        }

                    }
                    case 3:{ //Agregar, eliminar y modificar arcos (3).
                        clearScreen();
                        int subOpcion;
                        std::cout<<"Inserte la opción deseada: "
                            <<std::endl<<"Agregar arco (1)."
                            <<std::endl<<"Eliminar arco (2)."
                            <<std::endl<<"Modificar arco (3)."
                            <<std::endl<<"Salir (4). "<<std::endl;
                            std::cin>>subOpcion;
                            std::cin.ignore(10000,'\n');
                            switch(subOpcion){
                                case 1:{ // Agregar arco (1)
                                    string origen;
                                    string destino;
                                    string puntoDeOrigen;
                                    string puntoDeDestino;
                                    string medioDeTransporte;
                                    int horasRuta;
                                    while(true){
                                        clearScreen();
                                        std::cout<<"Vértices disponibles: "<<std::endl;
                                        imprimirVertices();
                                        std::cout<<"Inserte el vértice el cual será el origen de nuestro arco: ";
                                        getline(std::cin,origen);
                                        if(!comprobarNombreGrafo(origen)){
                                            while(true){
                                                clearScreen();
                                                std::cout<<"Vértices disponibles: "<<std::endl;
                                                imprimirVertices();
                                                std::cout<<"Inserte el vértice el cual será el destino de nuestro arco: ";
                                                getline(std::cin,destino);
                                                if(!comprobarNombreGrafo(destino) && origen!=destino){
                                                    while(true){
                                                        clearScreen();
                                                        std::cout<<"Puntos de partida desde el origen: "<<std::endl;
                                                        imprimirPuntosDeEntrada(origen);
                                                        std::cout<<"Escriba el nombre del punto de origen deseado: ";
                                                        getline(std::cin,puntoDeOrigen);
                                                        if(buscarPuntoDeEntradaEspecifico(origen,puntoDeOrigen)){
                                                            while(true){
                                                                clearScreen();
                                                                if(tipoPuntoEntradaGlobal=="Terminal/frontera"){
                                                                    medioDeTransporte="auto";
                                                                }
                                                                else if(tipoPuntoEntradaGlobal=="Aeropuerto"){
                                                                    medioDeTransporte="avión";
                                                                }
                                                                else if(tipoPuntoEntradaGlobal=="Muelle"){
                                                                    medioDeTransporte="barco";
                                                                }
                                                                std::cout<<"Puntos de partida disponibles desde el destino: "<<std::endl;\
                                                                imprimirPuntosDeEntrada(destino,true);
                                                                std::cout<<"Escriba el nombre del punto de destino deseado."
                                                                <<std::endl<<"(Si no hay puntos de destino es probable que el tipo de partida del origen no calce con ninguna de los puntos del destino)"
                                                                <<std::endl<<"(En ese caso puede escribir 'salir' para volver al menú): ";
                                                                getline(std::cin,puntoDeDestino);
                                                                if(puntoDeDestino=="salir"){
                                                                    break;
                                                                }
                                                                else if(buscarPuntoDeEntradaEspecifico(destino,puntoDeDestino)){
                                                                    clearScreen();
                                                                    std::cout<<"Cuanto horas dura la ruta?: ";
                                                                    std::cin>>horasRuta;
                                                                    std::cin.ignore(10000,'\n');
                                                                    if(insertarArcos(origen,horasRuta,medioDeTransporte,puntoDeOrigen,puntoDeDestino,destino)){
                                                                        clearScreen();
                                                                        std::cout<<"Arco insertado con éxito, volviendo al menú..."<<std::endl;
                                                                        sleep(2);
                                                                        break;
                                                                    }
                                                                    else{
                                                                        clearScreen();
                                                                        std::cout<<"IMPOSBILE"<<std::endl;
                                                                        sleep(2);
                                                                        break;
                                                                    }
                                                                }
                                                                else{
                                                                    clearScreen();
                                                                    std::cout<<"Comando inválido, volviendo al menú..."<<std::endl;
                                                                    sleep(2);
                                                                }
                                                            }
                                                            break;
                                                        }
                                                        else{
                                                            clearScreen();
                                                            std::cout<<"El nombre del punto origen no existe..."<<std::endl;
                                                            sleep(2);

                                                        }
                                                    }
                                                    break;
                                                }
                                                else{
                                                    clearScreen();
                                                    std::cout<<"El vértice que se insertó no existe o es repetido, volviendo..."<<std::endl;
                                                    sleep(2);
                                                }
                                            }
                                            break;

                                        }
                                        else{
                                            clearScreen();
                                            std::cout<<"El vértice que se insertó no existe, volviendo..."<<std::endl;
                                            sleep(2);
                                        }

                                    }
                                    continue;
                                }
                                case 2:{ // Eliminar arco (2) //FALTA ARREGLAR BUGS
                                    string origen;
                                    string destino;
                                    int indexRuta;
                                    while(true){
                                        clearScreen();
                                        std::cout<<"Vertices totales: "<<std::endl;
                                        imprimirVertices();
                                        std::cout<<"Seleccione el vertice origen: ";
                                        getline(std::cin,origen);
                                        if(!comprobarNombreGrafo(origen)){
                                            while(true){
                                                clearScreen();
                                                std::cout<<"Vertices totales: "<<std::endl;
                                                imprimirVertices();
                                                std::cout<<"Seleccione el vertice destino: ";
                                                getline(std::cin,destino);
                                                if(!comprobarNombreGrafo(destino) && origen!=destino){
                                                    while(true){
                                                        clearScreen();
                                                        std::cout<<"Rutas disponibles entre "<<origen<<" y "<<destino<<": "<<std::endl;
                                                        imprimirRuta(origen,destino);
                                                        try{
                                                            std::cin>>indexRuta;
                                                            std::cin.ignore(10000,'\n');
                                                            clearScreen();
                                                            std::cout<<((eliminarArco(origen,destino,indexRuta)) ? "Eliminado con éxito, volviendo al menú...":"Error al borrar, la ruta dada no existe, volviendo al menú...")<<std::endl;
                                                            sleep(2);
                                                            break;
                                                        }
                                                        catch(exception a){
                                                            clearScreen();
                                                            std::cout<<"Saliendo..."<<std::endl;
                                                            sleep(2);
                                                            break;
                                                        }
                                                    }
                                                    break;
                                                }
                                                else{
                                                    clearScreen();
                                                    std::cout<<"El vértice otorgado no existe o es igual al origen, volviendo..."<<std::endl;
                                                    sleep(2);
                                                }
                                            }
                                            break;
                                        }
                                        else{
                                            clearScreen();
                                            std::cout<<"El vértice otorgado no existe, volviendo..."<<std::endl;
                                            sleep(2);
                                        }
                                    }
                                    continue;
                                }
                                case 3:{ // Modificar arco (3)
                                    string origen;
                                    string destino;
                                    int indexRuta;
                                    int nuevoTiempo;
                                    while(true){
                                        clearScreen();
                                        std::cout<<"Vertices totales: "<<std::endl;
                                        imprimirVertices();
                                        std::cout<<"Seleccione el vertice origen: ";
                                        getline(std::cin,origen);
                                        if(!comprobarNombreGrafo(origen)){
                                            while(true){
                                                clearScreen();
                                                std::cout<<"Vertices totales: "<<std::endl;
                                                imprimirVertices();
                                                std::cout<<"Seleccione el vertice destino: ";
                                                getline(std::cin,destino);
                                                if(!comprobarNombreGrafo(destino) && origen!=destino){
                                                    clearScreen();
                                                    std::cout<<"Inserte el nuevo tiempo deseado para la ruta: ";
                                                    std::cin>>nuevoTiempo;
                                                    std::cin.ignore(10000,'\n');
                                                    clearScreen();
                                                    std::cout<<"Rutas disponibles entre "<<origen<<" y "<<destino<<": "<<std::endl;
                                                    imprimirRuta(origen,destino);
                                                    try{
                                                        std::cin>>indexRuta;
                                                        std::cin.ignore(10000,'\n');
                                                        clearScreen();
                                                        std::cout<<((modificarArco(origen,destino,nuevoTiempo,indexRuta)) ? "Ruta modificada con éxito, volviendo al menú.":"Error al modificar, volviendo al menú.")<<std::endl;
                                                        sleep(2);
                                                        break;

                                                    }
                                                    catch(exception a){
                                                        clearScreen();
                                                        std::cout<<"Saliendo..."<<std::endl;
                                                        sleep(2);
                                                        break;
                                                    }
                                                }
                                                else{
                                                    clearScreen();
                                                    std::cout<<"El vértice otorgado no existe o es igual al origen, volviendo..."<<std::endl;
                                                    sleep(2);
                                                }

                                            }
                                            break;
                                        }
                                        else{
                                            clearScreen();
                                            std::cout<<"El vértice otorgado no existe, volviendo..."<<std::endl;
                                            sleep(2);
                                        }
                                    }

                                    continue;
                                }
                                case 4:{ //Salir (4)
                                    continue;
                                }
                                default:{
                                    std::cout<<"Opción inválida, volviendo al menú..."<<std::endl;
                                    sleep(2);
                                    break;
                                }

                            }


                        continue;
                    }
                    case 4:{ //Guardar el grafo actualizado en un archivo JSON (4).
                        clearScreen();
                        string comando;
                        if(grafoRutas==NULL){
                            std::cout<<"El grafo de viajes esta vacio, volviendo al menú..."<<std::endl;
                            sleep(2);
                            continue;
                        }
                        std::cout<<"Estas seguro? esto sobrescribira el archivo prueba.json (y/n)."<<std::endl;
                        getline(std::cin,comando);
                        if(comando=="y" || comando=="yes" || comando=="si"){
                            clearScreen();
                            guardarGrafoRutaJson();
                            std::cout<<"Sobrescrito con exito, volviendo al menú..."<<std::endl;
                            sleep(2);
                        }
                        else{
                            clearScreen();
                            std::cout<<"Volviendo al menú..."<<std::endl;
                            sleep(2);
                        }
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
                        salirDatos=true;
                    }
                }
            }
            continue;

        }
        case 2:{ //Reportes (2)
            int opcion3;
            bool salirReportes=false;
            while(true){
                clearScreen();
                if(salirReportes){
                    salirReportes=false;
                    break;
                }
                std::cout<< "    ==============================================    "
               <<std::endl<<"                 REPORTES DEL GRAFO                   "
               <<std::endl<<"    ==============================================    "<<std::endl<<std::endl;

                std::cout<<"Esta es la sección de reportes, por favor intruduzca la opción deseada: "<<std::endl
                    <<std::endl<<"Imprimir el grafo en amplitud (1)."
                    <<std::endl<<"Imprimir el grafo en profundidad (2)."
                    <<std::endl<<"Imprimir la lista de premios (3)."
                    <<std::endl<<"Imprimir la lista de clientes (4)."
                    <<std::endl<<"Imprimir todos los clientes con viaje reservado (5)."
                    <<std::endl<<"Imprimir todos los clientes con premios (6)."
                    <<std::endl<<"Imprimir destinos del grafo sin visita (7)."
                    <<std::endl<<"Volver al menú principal (8)."<<std::endl
                    <<std::endl<<"Inserte la opción deseada: ";
                std::cin>>opcion3;
                std::cin.ignore(10000,'\n');
                switch(opcion3){
                    case 1:{ //Imprimir el grafo en amplitud (1).
                        string salir;
                        clearScreen();
                        if(grafoRutas==NULL){
                            std::cout<<"Lo sentimos, el grafo está vacio, volviendo..."<<std::endl;
                            sleep(2);
                            continue;
                        }
                        else{
                            std::cout<<"Gafo en amplitud: "<<std::endl<<std::endl;
                            imprimirGrafoAmplitud();
                            std::cout<<std::endl<<"Digite cualquier tecla para salir: ";
                            getline(std::cin,salir);
                            continue;
                        }

                    }
                    case 2:{ //Imprimir el grafo en profundidad (2)
                        string origen;
                        while(true){
                            clearScreen();
                            if(grafoRutas==NULL){
                                std::cout<<"Grafo vacio, volviendo al menú..."<<std::endl;
                                sleep(2);
                                break;
                            }
                            imprimirVertices();
                            std::cout<<std::endl<<"Inserte el vértice en el cual desea empezar a imprimir en profundidad (nombre): ";
                            getline(std::cin,origen);
                            if(!comprobarNombreGrafo(origen)){
                                verticeOrigen*tempV=buscarVertice(origen);
                                clearScreen();
                                std::cout<<"Grafo en profundidad: "<<std::endl<<std::endl;
                                profundidad(tempV);
                                string salirCualquiera;
                                std::cout<<std::endl<<"Inserte cualquier tecla para salir: ";
                                getline(std::cin,salirCualquiera);
                                break;
                            }
                            else{
                                clearScreen();
                                std::cout<<"Nombre inválido, volviendo...."<<std::endl;
                                sleep(2);
                            }
                        }
                        continue;
                    }
                    case 3:{ //Imprimir la lista de premios (3)
                        continue;
                    }
                    case 4:{ //Imprimir la lista de clientes (4)
                        continue;
                    }
                    case 5:{ //Imprimir todos los clientes con viaje reservado (5)
                        continue;
                    }
                    case 6:{ //Imprimir todos los clientes con premios (6)
                        continue;
                    }
                    case 7:{ //Imprimir destinos del grafo sin visita (7)
                        clearScreen();
                        string salirCualquiera;
                        if(grafoRutas==NULL){
                            std::cout<<"Grafo vacio, volviendo al menú..."<<std::endl;
                            sleep(2);
                            continue;
                        }
                        std::cout<<"Destinos sin visitas: "<<std::endl<<std::endl;
                        imprimirDestinosSinVisitas();
                        std::cout<<std::endl<<"Inserte cualquier tecla para continuar: ";
                        getline(std::cin,salirCualquiera);
                        continue;
                    }
                    case 8:{ //Volver al menú principal (8)
                        salirReportes=true;
                        continue;
                    }
                    default:{
                        clearScreen();
                        std::cout<<"Opción inválida, volviendo al menú..."<<std::endl;
                        sleep(2);
                        continue;
                    }
                }

            }


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
