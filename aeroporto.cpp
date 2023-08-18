
#include <mpi.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <cstdio>
#include <cstdlib>

// Inicializei as variáveis aqui porque elas precisam ser vistas pelas threads
int size, rank,
        provided;

// Criaos aqui porque precisam ser usados na classe Aeroporto
struct Voo {
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

    Voo() : codigo(0), origem(0), destino(0), hora_saida(0), hora_chegada(0), tempo_voo(0), ativo(true) {}
};

struct Mensagem {
    int lc;
    struct Voo voo;

    Mensagem(int z) {
        voo = Voo(0, 0, 0, 0);
        lc = 0;
    }
}; 

class Aeroporto {
private:
    std::vector<struct Voo> decolagens_pendentes;
    std::vector<struct Voo> decolagens_feitas;
    std::vector<struct Voo> pousos_pendentes;
    std::vector<struct Voo> pousos_feitos;
    bool loop = true;
    bool new_msg = false; 
    int id;
    int lc;

    std::mutex pousos_mutex;

    void addDecolagem(struct Voo voo) {
        decolagens_pendentes.push_back(voo);
    }

    void addPouso(struct Voo voo) {
        pousos_pendentes.push_back(voo);
    } 

    // Implementação de um pop_at que remove um item em uma determinada posicao do vetor 
    std::vector<struct Voo> remove(std::vector<struct Voo> list, int pos) {
        std::vector<struct Voo> new_list;
        for(long unsigned i = 0, j = 0; i < list.size(); i++, j++) {
            j += ((int)j != pos) ? 0 : 1;
            new_list[i] = list[j];
        }
        return new_list;
    }

    // Atualiza o estado dos voos de acordo com o relógio local
    void atualizar() {
        
        for(long unsigned i = 0; i < decolagens_pendentes.size(); i++) {
            struct Voo decolagem = decolagens_pendentes[i];
            if(lc >= decolagem.hora_saida) {
                decolagens_pendentes = remove(decolagens_pendentes, i);
                decolagens_feitas.push_back(decolagem);    
            }
        }
        
        for(long unsigned i = 0; i < pousos_pendentes.size(); i++) {
            struct Voo pouso = pousos_pendentes[i];
            if(lc >= pouso.hora_chegada) {
                pousos_pendentes = remove(pousos_pendentes, i);
                pousos_feitos.push_back(pouso);    
            }
        }        
    }

    // Define qual voo tem maior prioridade dentre os pendentes (pousos e decolagens)
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
        // Um pouso e uma decolagem simultâneos
        } else {
            if(v1.destino == id)
                v2.incremDecolagem();
            else
                v1.incremDecolagem();
        }
    }    

    // Faz chamadas para a funcao prioridade para atualizar o valor dos voos com menor prioridade e conflito
    void corrigeConflitos(struct Voo &voo) {
        for(long unsigned i = 0; i < decolagens_pendentes.size(); i++) 
            prioridade(voo, decolagens_pendentes[i]);
        
        for(long unsigned i = 0; i < pousos_pendentes.size(); i++) 
            prioridade(voo, pousos_pendentes[i]);
    }

    // Imprime os dados do aeroporto
    void display() {
        // Garante que os dados sejam acessados somente após alguma modificação
        // Impressão dos voos
        system("clear");
        int total_pousos = pousos_feitos.size() + pousos_pendentes.size();
        int total_decolagens = decolagens_feitas.size() + decolagens_pendentes.size();
        std::cout << "|Hora   | " << lc << "                    |\n" <<
                     "|Codigo |           " << id << "          |\n" <<
                     "|Pousos | " << total_pousos << " | Decolagens: " << total_decolagens << " |\n" <<
                     "|Pousos | Origem | Chegada | T. Voo |\n";
        for(int i = 0; i < (int)pousos_feitos.size(); i++)
            std::cout << "| " << pousos_feitos[i].codigo << " | " << pousos_feitos[i].origem <<
                        " | " << pousos_feitos[i].hora_chegada << " | " << pousos_feitos[i].tempo_voo << " |\n";
        for(int i = 0; i < (int)pousos_pendentes.size(); i++)
            std::cout << "| " << pousos_pendentes[i].codigo << " | " << pousos_pendentes[i].origem <<
                        " | " << pousos_pendentes[i].hora_chegada << " | " << pousos_pendentes[i].tempo_voo << " |\n";
        std::cout << "|Decolagens | Destino | Partida | T. Voo |\n";
        for(int i = 0; i < (int)decolagens_feitas.size(); i++)
            std::cout << "| " <<decolagens_feitas[i].codigo << " | " <<decolagens_feitas[i].destino <<
                        " | " <<decolagens_feitas[i].hora_saida << " | " <<decolagens_feitas[i].tempo_voo << " |\n";
        for(int i = 0; i < (int)decolagens_pendentes.size(); i++)
            std::cout << "| " << decolagens_pendentes[i].codigo << " | " << decolagens_pendentes[i].destino <<
                        " | " << decolagens_pendentes[i].hora_saida << " | " << decolagens_pendentes[i].tempo_voo << " |\n";
    }

    // Inputs da criacao de um voo
    Voo menuAddVoo() {
        int destino, saida, chegada;
        bool condition;
        // Input das informações suficientes para criação do voo

        // Controle de input para o destino no intervalo [1, size[
        do {
            std::cout << "Destino: ";
            std::cin >> destino;
            condition = destino < 1 || destino > size;
            if(condition)
                std::cout << "Digite um valor no intervalo [1, n_aeroportos[" << std::endl;
        } while(condition);

        // Controle de saída (precisa ser maior ou igual que o local clock)
        do {
            std::cout << "Saida: ";
            std::cin >> saida;
            condition = saida < lc;
            if(condition) 
                std::cout << "Digite um valor maior ou igual que o horario local" << std::endl;
        } while(condition);

        // Controle de chegada (precisa ser maior que a saida)
        do {
            std::cout << "Chegada: ";
            std::cin >> chegada;
            condition = chegada <= saida;
            if(condition)
                std::cout << "Digite um valor maior que o horario de saida do voo" << std::endl;
        } while(condition);

        // Voo(int ori, int dest, int h_said, int h_cheg)
        struct Voo v = Voo(id, destino, saida, chegada);
        v.codigo = id * 100 + (int)(decolagens_feitas.size() + decolagens_pendentes.size());
        return v;
    }

    // Função de envio de um voo para outro aeroporto
    bool commVoo(Voo v) {
        MPI_Request req;
        Mensagem msg = Mensagem(0);
        msg.voo = v;
        MPI_Isend(&msg, sizeof(Mensagem), MPI_BYTE, v.destino - 1, 0, MPI_COMM_WORLD, &req);
        // Passo 2 do algoritmo da descrição do trabalho
        MPI_Wait(&req, MPI_STATUS_IGNORE);
        lc++;
        // Adiciona o voo à lista de decolagens
        addDecolagem(v);
        return true;
    }

    // Função de recebimento de um voo de outro aeroporto
    bool recvVoo() {
        MPI_Request req;
        MPI_Status sta;
        Mensagem msg = Mensagem(lc);
        MPI_Irecv(&msg, sizeof(Mensagem), MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &sta);
        // Passo 3 do algoritmo da descrição do trabalho
        lc = (lc < msg.lc ? msg.lc + 1 : lc + 1);
        // Adiciona o pouso à lista de pousos
        pousos_mutex.lock();
        addPouso(msg.voo);
        atualizar();
        corrigeConflitos(msg.voo);
        new_msg = true;
        pousos_mutex.unlock();

        return true;
    }
public:
    Aeroporto(int c_id) {
        id = c_id;
        lc = 0;
    }

    void rcv() {
        while(loop) {
            recvVoo();
        }
    }

    void menu() {
        char opt = '0';
        bool brk = false;

        while(loop) {
            display();
            do {
                std::cout << "-----------------------------\n" <<
                            "|1. Adicionar voo           |\n" <<
                            "|2. Sair                    |\n" <<
                            "-----------------------------\n";
                std::cout << ">";
                for(int i = 0; i < 5; i++) {
                    if(std::cin >> opt) {
                        brk = true;
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            } while(opt != '1' && opt != '2' && !brk);
            
            brk = false;
            switch(opt) {
                case '1': {
                    struct Voo temp_v = menuAddVoo();
                    pousos_mutex.lock();
                    commVoo(temp_v);
                    atualizar();
                    corrigeConflitos(temp_v);
                    pousos_mutex.unlock();
                    system("clear");
                    } 
                    break;
                case '2':
                    loop = false;
                    break;
                default:
                    break;
            }
        }
    }

    void init() {
        std::vector<std::thread> threads;

        std::thread t_main(&Aeroporto::menu, this);           // controle de ordenação e envios
        std::thread t_recv(&Aeroporto::rcv, this);       // controle de recebimentos
        
        t_main.join();
        t_recv.join();
    }
}; 

// Criado como global porque as threads precisam acessar os dados dos aeroportos

int main(int argc, char *argv[]) {

    std::vector<Aeroporto> aeroportos;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Cria e inicializa os aeroportos em cada máquina
    Aeroporto aeroporto = Aeroporto(rank+1);
    aeroporto.init();

    MPI_Finalize();

    return 0;
}
