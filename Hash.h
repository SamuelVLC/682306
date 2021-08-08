#pragma once
#include <math.h>


class Bucket
{
public:
	unsigned int cpf;
	unsigned int Address;

	Bucket();
	~Bucket();
};

class Page : public Bucket
{	
private:
	size_t pageSize;
public:
	unsigned int nBuckets_MAX;//quantidade maxima de buckets por pagina.
	unsigned int bucketDepth;//profundidade da pagina
	unsigned int nBuckets;//quantidade de buckets da pagina
	Bucket *b;//ponteiro de bucket.

	Page();
	Page(unsigned int size);
	~Page();
	void createPage(const unsigned int length);//aloca dinamicamente um espaço temporario para o ponteiro de buckets sendo length a quantidade de buckets a ser alocado para determinada pagina.
	void deletePage();//deleta a pagina alocada
	size_t GetPageSize();//retorna o tamanho da pagina
};

class Hash
{
public:
	unsigned int Buckets_MAX;//usado para definir a quantidade maxima de buckets por pagina
	unsigned int depth;//profundidade global da hash
	bool is_linked;// flag de auxilio na manipulaçao dos dados e arranjos do diretorio
	Page page;//pagina a ser escrita no arquivo de indices

	unsigned int hashFunction(unsigned int key);//funçao hash. retorna um valor dado o cpf.
	Hash();
	~Hash();
};

