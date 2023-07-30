
#include <mpi.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <cstdio>

std::mutex pousos_mutex;
std::mutex decolagens_mutex;

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
        std::lock_guard<std::mutex> guard(pousos_mutex);
        decolagens_pendentes.push_back(voo);
    }

    void addPouso(struct Voo voo) {
        std::lock_guard<std::mutex> guard(decolagens_mutex);
        pousos_pendentes.push_back(voo);
    } 

    std::vector<struct Voo> remove(std::vector<struct Voo> list, int pos) {
        std::vector<struct Voo> new_list;
        for(long unsigned i = 0, j = 0; i < list.size(); i++, j++) {
            j += ((int)j != pos) ? 0 : 1;
            new_list[i] = list[j];
        }
        return new_list;
    }

    void atualizar() {
        
        {
            std::lock_guard<std::mutex> guard(decolagens_mutex);
            for(long unsigned i = 0; i < decolagens_pendentes.size(); i++) {
                struct Voo decolagem = decolagens_pendentes[i];
                if(lc >= decolagem.hora_saida) {
                    decolagens_pendentes = remove(decolagens_pendentes, i);
                    decolagens_feitas.push_back(decolagem);    
                }
            }
        }
        {
            std::lock_guard<std::mutex> guard(pousos_mutex);
            for(long unsigned i = 0; i < pousos_pendentes.size(); i++) {
                struct Voo pouso = pousos_pendentes[i];
                if(lc >= pouso.hora_chegada) {
                    pousos_pendentes = remove(pousos_pendentes, i);
                    pousos_feitos.push_back(pouso);    
                }
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
        {
            std::lock_guard<std::mutex> guard(decolagens_mutex);
            for(long unsigned i = 0; i < decolagens_pendentes.size(); i++) 
                prioridade(voo, decolagens_pendentes[i]);
        }
        {
            std::lock_guard<std::mutex> guard(pousos_mutex);
            for(long unsigned i = 0; i < pousos_pendentes.size(); i++) 
                prioridade(voo, pousos_pendentes[i]);
        }
    }

    void display() {
        // Garante que os dados sejam acessados somente após alguma modificação
        std::lock_guard<std::mutex> guard(decolagens_mutex);
        std::lock_guard<std::mutex> guard(pousos_mutex);
        // Impressão dos voos
        int total_pousos = pousos_feitos.size() + pousos_pendentes.size();
        int total_decolagens = decolagens_feitas.size() + decolagens_pendentes.size();
        std::cout << "|Codigo |           " << id << "          |\n" <<
                     "|Pousos | " << total_pousos << " | Decolagens: " << total_decolagens << " |\n" <<
                     "|Pousos | Origem | Chegada | T. Voo |\n";
        for(int i = 0; i < (int)pousos_feitos.size(); i++)
            std::cout << "| " << pousos_feitos[i].codigo << " | " << pousos_feitos[i].origem <<
                        " | " << pousos_feitos[i].hora_chegada << " | " << pousos_feitos[i].tempo_voo << " |\n";
        for(int i = 0; i < (int)pousos_pendentes.size(); i++)
            std::cout << "| " << pousos_pendentes[i].codigo << " | " << pousos_pendentes[i].origem <<
                        " | " << pousos_pendentes[i].hora_chegada << " | " << pousos_pendentes[i].tempo_voo << " |\n";
        std::cout << "|Decolagens | Destino | Partida | T. Voo |\n";
        for(int i = 0; i < (int)decolagens_decolagens_feitas.size(); i++)
            std::cout << "| " <<decolagens_feitas[i].codigo << " | " <<decolagens_feitas[i].destino <<
                        " | " <<decolagens_feitas[i].hora_saida << " | " <<decolagens_feitas[i].tempo_voo << " |\n";
        for(int i = 0; i < (int)decolagens_pendentes.size(); i++)
            std::cout << "| " << decolagens_pendentes[i].codigo << " | " << decolagens_pendentes[i].destino <<
                        " | " << decolagens_pendentes[i].hora_saida << " | " << decolagens_pendentes[i].tempo_voo << " |\n";
    }

    Voo menuAddVoo() {
        int destino, saida, chegada;
        Voo v;
        // Input das informações suficientes para criação do voo
        std::cout << "Destino: ";
        std::cin >> destino;
        std::cout << "Saida: ";
        std::cin >> saida;
        std::cout << "Chegada: ";
        std::cin >> chegada;
        // Voo(int ori, int dest, int h_said, int h_cheg)
        v = Voo(id, destino, saida, chegada);
        std::lock_guard<std::mutex> guard(decolagens_mutex);
        v.codigo = id * 100 + (int)(decolagens_feitas.size() + decolagens_pendentes.size());
        return v;
    }

    bool commVoo(Voo v) {
        MPI_Request = req;
        Mensagem msg = {lc, v};
        MPI_Isend(&v, sizeof(Mensagem), MPI_BYTES, destino - 1, 0, MPI_COMM_WORLD, &req);
        // Passo 2 do algoritmo da descrição do trabalho
        lc++;
        // Adiciona o voo à lista de decolagens
        addDecolagem(v);
    }

    bool recvVoo() {
        MPI_Request req;
        MPI_Status sts;
        Mensagem msg;
        MPI_Irecv(&msg, sizeof(Mensagem), MPI_BYTES, origem - 1, 0, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &sta);
        // Passo 3 do algoritmo da descrição do trabalho
        lc = (lc < msg.lc ? msg.lc + 1 : lc + 1);
        // Adiciona o pouso à lista de pousos
        addPouso(msg.voo);
        return true;
    }

    void receiver() {
        while(recvVoo()) {
            atualizar();
            corrigeConflitos();
        }
    }

    void menu() {
        char opt = '0';

        display();
        do {
            std::cout << "-----------------------------\n" <<
                         "|1. Adicionar voo           |\n" <<
                         "|2. Sair                    |\n" <<
                         "-----------------------------\n";
            std::cout << ">";
            std::cin >> opt;
        } while(opt != '1' && opt != '2');
        switch(opt) {
            case '1':
                Voo temp_v = menuAddVoo();
                atualizar();
                corrigeConflitos();
                break;
            case '2':
                return;
        }
    }

    void init() {
        std::thread t_main(menu);
        std::thread t_recv(receiver);
    }
}; 

// Criado como global porque as threads precisam acessar os dados dos aeroportos
std::vector<Aeroporto> aeroportos;

int main(int argc, char *argv[]) {

    int errs = 0;
    int provided, flag, claimed;
    int size, rank;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    aeroportos.reserve(size);
    // Cria e inicializa os aeroportos em cada máquina
    aeroportos[rank] = Aeroporto(rank+1)
    aeroportos[rank].init();

    MPI_Finalize();

    return 0;
}
