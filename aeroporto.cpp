
// #include <mpi.h>
#include <iostream>

struct Voo {

    int origem,
        destino,
        hora_saida,
        hora_chegada,
        tempo_voo;

    Voo(int ori, int dest, int h_said, int h_cheg) {
        origem = ori;
        destino = dest;
        hora_saida = h_said;
        hora_chegada = h_cheg;
        tempo_voo = hora_chegada - hora_saida;
    }

    void toString() {
        
    }
};

class Aeroporto {
private:
public:
}; 

int main(int argc, char *argv[]) {

    //MPI_Init(&argc, &argv);



    //MPI_Finalize();

    return 0;
}

