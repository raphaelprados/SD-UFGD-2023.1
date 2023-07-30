
#include <mpi.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <cstdio>

typedef struct Voo {
    int codigo,
        origem,
        destino,
        hora_saida,
        hora_chegada,
        tempo_voo,
        ativo;

    Voo(int ori, int dest, int h_said, int h_cheg) {
        origem = ori;
        destino = dest;
        hora_saida = h_said;
        hora_chegada = h_cheg;
        tempo_voo = hora_chegada - hora_saida;
        ativo = true;
    }

    void incremPouso() {
        hora_chegada++;
        tempo_voo++;
    }

    void incremDecolagem() {
        hora_saida++;
        hora_chegada++;
    }

    std::string toString() {
        std::string str = "|" + std::to_string(codigo) + "   " + 
                          "|" + std::to_string(destino) + "  " +
                          "|" + std::to_string(hora_chegada) + "     " +
                          "|" + std::to_string(tempo_voo) + "    " +
                          "|\n";         
        return str;
    }
} Voo;

typedef struct Mensagem {
    int lc;
    struct Voo voo;
} Mensagem; 

class Aeroporto {
private:
    std::vector<struct Voo> decolagens_pendentes;
    std::vector<struct Voo> decolagens_feitas;
    std::vector<struct Voo> pousos_pendentes;
    std::vector<struct Voo> pousos_feitos;
    int id;
    int lc;
public:
    Aeroporto(int c_id) {
        id = c_id;
        lc = 0;
    }

    void addDecolagem(struct Voo voo) {
        decolagens_pendentes.push_back(voo);
    }

    void addPouso(struct Voo voo) {
        pousos_pendentes.push_back(voo);
    } 

    std::vector<struct Voo> remove(std::vector<struct Voo> list, int pos) {
        std::vector<struct Voo> new_list;
        for(int i = 0, j = 0; i < list.size(); i++, j++) {
            j += j != pos ? 0 : 1;
            new_list[i] = list[j];
        }
        return new_list;
    }

    void atualizar() {
        for(int i = 0; i < decolagens_pendentes.size(); i++) {
            struct Voo decolagem = decolagens_pendentes[i];
            if(lc >= decolagem.hora_saida) {
                decolagens_pendentes = remove(decolagens_pendentes, i);
                decolagens_feitas.push_back(decolagem);    
            }
        }
        for(int i = 0; i < pousos_pendentes.size(); i++) {
            struct Voo pouso = pousos_pendentes[i];
            if(lc >= pouso.hora_chegada) {
                pousos_pendentes = remove(pousos_pendentes, i);
                pousos_feitos.push_back(pouso);    
            }
        }
    }

    void prioridade(struct Voo &v1, struct Voo &v2) {
        bool simDecs = v1.origem == v2.origem,
             simPous = v1.destino == v2.destino;

        // Duas decolagens ou dois pousos simultaneos
        if(simDecs || simPous) {
            if(v1.tempo_voo > v2.tempo_voo) {
                if(simDecs)
                    v2.incremDecolagem();
                else
                    v2.incremPouso();
            } else {
                if(simDecs)
                    v1.incremDecolagem();
                else
                    v1.incremPouso();
            }
        // Um pouco e uma decolagem simultâneos
        } else {
            if(v1.destino == id)
                v2.incremDecolagem();
            else
                v1.incremDecolagem();
        }
    }    

    void corrigeConflitos(struct Voo &voo) {
        for(int i = 0; i < decolagens_pendentes.size(); i++) 
            prioridade(voo, decolagens_pendentes[i]);
        for(int i = 0; i < pousos_pendentes.size(); i++) 
            prioridade(voo, pousos_pendentes[i]);
    }
}; 

int main(int argc, char *argv[]) {

    int errs = 0;
    int provided, flag, claimed;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

      MPI_Is_thread_main( &flag );
    if (!flag) {
        errs++;
        printf( "This thread called init_thread but Is_thread_main gave false\n" );fflush(stdout);
    }
    MPI_Query_thread( &claimed );
    if (claimed != provided) {
        errs++;
        printf( "Query thread gave thread level %d but Init_thread gave %d\n", claimed, provided );fflush(stdout);
    }

    MPI_Finalize();

    return 0;
}

