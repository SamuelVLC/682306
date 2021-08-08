#pragma once
#include "fileManagement.h"
#include <sstream>

using namespace std;

class simulation
{
private:
	unsigned int* keysArray;//ponteiro para um array de cpfs a serem gerados
	fileManagement file;//objeto que ira lidar com a simulaçao
public:
	unsigned int k;//numero de chaves
	unsigned int n;//numero de buckets por pagina
	unsigned int m;//tamanho do registro

	simulation();
	~simulation();

	bool runSimulation();//chamada para a inicializaçao da simulaçao
	float insertBlockOfRecords();//insere o bloco de registros
	float getBlockOfRecords();//le e escreve o bloco de registros inseridos
	void shuffleCPFS(unsigned int start_position, unsigned int end_position);//embaralha os cpfs do vetor trocando eles de posiçoes aleatoriamente
	void generateRandomCPFS();//gera a quantidade sequencial de cpfs de 1 ate o numero maximo k de chaves e armazena no vetor alocado dinamicamente chamado keysArray
};

