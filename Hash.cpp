#include "Hash.h"
/*------------------------Bucket member functions------------------------------*/

Bucket::Bucket()
{
	/*
	* os valores abaixo sao os valores que indicam um bucket quando ele esta vazio
	*/
	cpf = 0;
	Address = 1;
}
Bucket::~Bucket() {};

/*------------------------Page member functions------------------------------*/

Page::Page()
{
	nBuckets_MAX = 4;
	bucketDepth = nBuckets = 0;
}

Page::Page(unsigned int size)// construtor que define a quantidade maxima de buckets por pagina
{
	nBuckets_MAX = size;
	bucketDepth = nBuckets = 0;
}

Page::~Page() {}


void Page::createPage(const unsigned int length)
{
	this->b = new Bucket[length];
}

void Page::deletePage()
{
	delete[] this->b;
}

size_t Page::GetPageSize()
{
	this->pageSize = (sizeof(Bucket) * nBuckets_MAX) + sizeof(nBuckets) + sizeof(bucketDepth);

	return this->pageSize;
}

/*------------------------Hash member functions------------------------------*/
unsigned int Hash::hashFunction(unsigned int key)
{
	return key % (unsigned int)pow(2, this->depth);
}

Hash::Hash()
{
	depth = 0;
	Buckets_MAX = 2;
}
Hash::~Hash(){}