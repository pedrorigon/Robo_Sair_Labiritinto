
#include <iostream>
#include <math.h>

#include "libs/EdubotLib.hpp"

#define SPEED 0.1        // velocidade
#define SLEEPMOVE 50     // sleep de movimento
#define SLEEPROTATE 2000 // sleep de rotacao

int main()
{
    double distancia = 0.073; // variavel para distancia minima para o sensor
    bool check = true;        //
    double last_pos_x = 0;    // ultimo x que tinha parede da direita
    double last_pos_y = 0;    // ultimo y que tinha parede da direita
    bool parede = false;      // se achou parede para seguir
    bool tem_direita = false; // se tem parede na direita
    bool travado = false;     // se bateu no bumper
    int curva = 0;            // contador para variar angulo de correcao apos bumper
    EdubotLib *edubotLib = new EdubotLib();

    //try to connect on robot
    if (edubotLib->connect())
    {

        edubotLib->move(SPEED); // para iniciar andando para frente todas as vezes(IDE variava sem isso)
        edubotLib->sleepMilliseconds(SLEEPMOVE);

        while (edubotLib->isConnected())
        {
            if (edubotLib->getSonar(3) > distancia) // condicao de andar para frente se nao tiver obstaculo
            {
                edubotLib->move(SPEED);
                edubotLib->sleepMilliseconds(SLEEPMOVE);
            }
            if (edubotLib->getSonar(3) < 0.07 && !parede) // se chegar numa parede e nao tiver seguindo uma parede
            {
                edubotLib->stop();

                edubotLib->rotate(-90); // vira esquerda
                edubotLib->sleepMilliseconds(SLEEPROTATE);
                parede = true; // diz que agora esta seguindo uma parede da direita
            }

            if (parede) // condicao para outros casos se tiver uma parede para seguir
            {
                if (edubotLib->getSonar(6) < distancia) // se tiver uma parede na direita
                {
                    edubotLib->move(SPEED); // anda pra frente
                    edubotLib->sleepMilliseconds(SLEEPMOVE);
                    last_pos_x = edubotLib->getX(); // pega pos x e y enquanto estiver a parede, se nao tiver mais, eh a ultima pos que tinha
                    last_pos_y = edubotLib->getY();
                    check = true;       // condicao para nao entrar em loop quando virar a direita vira true
                    tem_direita = true; // diz que parede da direita eh true
                    travado = false;    // diz que nao esta travado com bumpers
                }
                else // se nao tiver parede da direita
                {
                    tem_direita = false; // diz que nao tem parede da direita

                    /*condicao de distancia para dar espaco, se a valor absoluto da ultima pos da direita - pos atual
                    for maior que a metade do robo, assim tem espaco para dobrar a direita
                    */
                    if (fabs(last_pos_x - edubotLib->getX()) > 0.17 || fabs(last_pos_y - edubotLib->getY()) > 0.17)
                    {
                        if (check) // se tem condicao para dobrar a direita sem entrar em loop
                        {
                            edubotLib->stop();
                            edubotLib->sleepMilliseconds(SLEEPMOVE);
                            edubotLib->rotate(90); // vira a direita
                            edubotLib->sleepMilliseconds(SLEEPROTATE);
                            check = false; // diz que nao pode virar a direita de novo
                        }
                    }
                }

                if (tem_direita || travado) // se tiver parede da direita ou bateu no bumper recentemente
                {
                    /* condicao para nao travar se tiver em um canto com parede fronta e esquerda, assim toma o rumo 
                    preferencial de achar uma parede da direita dobrar a esquerda*/
                    if (edubotLib->getSonar(3) < 0.08 && edubotLib->getSonar(0) < edubotLib->getSonar(6))
                    {
                        edubotLib->stop();

                        edubotLib->rotate(-90);
                        edubotLib->sleepMilliseconds(SLEEPROTATE);
                    }
                }

                // se parede frontal e direita estiverem proximas, dobra a esquerda
                if (edubotLib->getSonar(3) < 0.08 && edubotLib->getSonar(6) < 0.1)
                {
                    edubotLib->stop();
                    edubotLib->sleepMilliseconds(SLEEPMOVE);
                    edubotLib->rotate(-90);
                    edubotLib->sleepMilliseconds(SLEEPROTATE);
                }
            }

            // caso bater corrige angulo indo pra tras se for bumper frontal

            if (edubotLib->getBumper(0) || edubotLib->getBumper(1)) // se bater bumpers frontais
            {
                edubotLib->move(-0.2); // vai para tras
                edubotLib->sleepMilliseconds(100);
                edubotLib->stop();

                edubotLib->rotate(edubotLib->getTheta() - 90 * curva); // corrige angulo variando angulo final
                edubotLib->sleepMilliseconds(1000);

                travado = true; // diz que bateu recentemente para achar uma nova parede para seguir
                parede = false; // diz que nao tem parede a seguir
                curva++;        // incrementa contador para variar angulo final de correcao
            }

            // caso bater corrige angulo indo pra frente se for bumper traseiro
            if (edubotLib->getBumper(2) || edubotLib->getBumper(3)) // se bater bumpers traseiros
            {
                edubotLib->move(0.2); // vai para frente
                edubotLib->sleepMilliseconds(100);
                edubotLib->stop();

                edubotLib->rotate(edubotLib->getTheta() - 90 * curva); // corrige angulo variando angulo final
                edubotLib->sleepMilliseconds(1000);
                travado = true; // diz que bateu recentemente para achar uma nova parede para seguir
                parede = false; // diz que nao tem parede a seguir
                curva++;        // incrementa contador para variar angulo final de correcao
            }
            if (curva == 4) // se contador checar a 4, volta para 0 , assim sempre estara entre 0 e 360
                curva = 0;
        }
        edubotLib->disconnect();
    }

    else
    {
        std::cout << "Could not connect on robot!" << std::endl;
    }

    return 0;
}

/*
anotacoes

	-vai reto ate achar uma parede, vira esquerda e segue usando sensor da direita
    -se direita estiver disponivel gira direita
    -se direita e frente estiverem proximas, vira esquerda
    -se bate corrige angulo variando cada vez que bate
    -se bate achar nova parede para seguir pela direita
    

		*/