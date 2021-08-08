#pragma once

#include <fstream>
#include <iostream>
#include "Hash.h"
#include <stdlib.h>
#include <string>
#include <time.h>
#include <cstring>

using namespace std;

typedef struct B_day
{
	unsigned short int day;
	unsigned short int month;
	unsigned short int year;
};

class Record
{
public:
	short record_size;//tamanho de cada registro
	unsigned int cpf;
	string name;
	B_day Bday;
	char gender;
	string notes;

	Record();

	~Record();

	void printRecord(Record p);//printa um registro
};



class mainManagement
{
public:
	string MainFileName;
	fstream mainFile;
protected:
	long int offset;

};

class indexManagement
{
public:
	string IndexFileName;
	string DirectoryFileName;
	Hash hash;
	unsigned int* array;//vetor que armazena o diretorio em memoria primaria. o diretorio contem os endereços das paginas.
	unsigned int search(unsigned int address, unsigned int cpf);//dado o endereço da pagina e o cpf do registro procura-se o bucket correspondente ao cpf.
protected:
	fstream indexFile;
	fstream directoryFile;
	void writePage(Page p);//escreve uma pagina em memoria secundaria
	Page readPage();//le uma pagina do arquivo em memoria secundaria
	void printPage(Page p);//printa uma pagina
	unsigned int* readDirectory();//le o diretorio, armazena a profundidade do diretorio no membro depth do objeto hash e retorna um ponteiro para um array que contem referencias para as paginas dos indices
	bool deleteCpf(unsigned int address, unsigned int cpf);// dado o endereço de uma pagina e um respectivo cpf, deleta logicamente o registro zerando o cpf
	bool compareAddresses(const int i, const int j);// compara dois endereços de paginas. retorna true se forem iguais, falso caso contrario.
	unsigned int nBits(unsigned int n);//calcula quantos bits no minimo sao necessario para representar um numero decimal
	bool is_linked(unsigned int root, unsigned int nbits, const unsigned int cpf, const unsigned int NewBucketDepth);//verifica se o resultado do rehash com determinado cpf ira permitir a mudança de pagina do bucket referente ao cpf para a nova pagina a ser criada
	void updateArray();//atualiza o vetor do diretorio em memoria primaria

};

class fileManagement : public mainManagement, public indexManagement, public Record
{
private:
	clock_t start_t, end_t;
	void setEOF(long int offset);//seta um offset baseado no tamanho de cada registro ao final da inserçao.
    Bucket LinkIndexToMain(unsigned int pos, Bucket b);//gerencia o arquivo de indices
	void LinkDirectoryToIndex(unsigned int cpf, unsigned int address);//gerencia o arquivo diretorio
    void RearrangePointers(unsigned int root, unsigned int nbits, unsigned int address);// rearranja os ponteiros do diretorio sempre que preciso
	void printDirectoryFile();//printa o diretorio
	void printIndexFile();//printa o arquivo de indices

public:
	bool addRecord(Record r);//funçao de chamada no main para adicionar um registro
	unsigned int writeRecord(Record r);//funçao para escrever o registro no arquivo mestre
	Record readRecord(unsigned int offset);//le um registro do arquivo mestre dado o seu endereço(offset a partir do inicio)
	bool edit(unsigned int cpf, string notes);//edita determinado registro dado o cpf e as notas

	void printMainFile();//printa o arquivo mestre

	bool remove(unsigned int cpf);//funçao que da acesso ao delete.Remove um registro

	void printFiles();//printa todos os arquivos

	fileManagement();
	~fileManagement();

	void createFiles();//cria um sistema de tres arquivos e reseta o conteudo anterior destes.
};



