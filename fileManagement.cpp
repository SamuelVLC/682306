#include "fileManagement.h"
/*------------------------Record member functions------------------------------*/

void Record::printRecord(Record p)
{
	cout << "CPF:" << p.cpf << endl;
	cout << "Name:" << p.name << endl;
	cout << "Birthdate:" << p.Bday.day << "/" << p.Bday.month << "/" << p.Bday.year << endl;
	cout << "Gender:" << p.gender << endl;
	cout << "Notes:" << endl;
	cout << p.notes << endl;
}

Record::Record()
{
	cpf = 0;
	Bday.day = Bday.month = Bday.year = 0;
	gender = '-';
}

Record::~Record()
{
}

/*------------------------fileManagement member functions------------------------------*/

fileManagement::fileManagement()
{
	MainFileName = "main.txt";
	IndexFileName = "index.txt";
	DirectoryFileName = "directory.txt";
	record_size = 2500;
	hash.Buckets_MAX = 2;
	offset = 0;
	array = readDirectory();
}
fileManagement::~fileManagement()
{
	free(array);// desaloca a memoria antes de finalizar o programa
}

void fileManagement::createFiles()
{
	/*
	* Verifica se qualquer um dos arquivos estao aberto antes e fecha se estes estiverem aberto.
	*/
	if (directoryFile.is_open())
		directoryFile.close();
	if (mainFile.is_open())
		mainFile.close();
	if (indexFile.is_open())
		indexFile.close();

	/*
	* Abre/cria o sistema de arquivos.
	* trunca qualquer conteudo previamente armazenado(deleta o conteudo).
	*/

	directoryFile.open(DirectoryFileName.c_str(), ios::trunc | ios::out | ios::in);
	indexFile.open(IndexFileName.c_str(), ios::trunc | ios::out | ios::in);
	mainFile.open(MainFileName.c_str(), ios::trunc | ios::out | ios::in);

	/*
	* fecha os arquivos criados para permitir que eles sejam trabalhados e operados no sistema.
	*/
	mainFile.close();
	indexFile.close();
	directoryFile.close();

	hash.depth = 0;//atribuir valor inicial para a profundidade global igual a 0
	
	free(array);//libera o espaço em memoria primaria do vetor do diretorio
	array = NULL;//atribui uma referencia nula para o mesmo inicialmente
}

void fileManagement::setEOF(long int offset)
{
	while (offset != 0)
	{
		mainFile << '\0';
		offset--;
	}
}

unsigned int fileManagement::writeRecord(Record r)
{
	size_t size;

	if (!mainFile.is_open())//verifica se o arquivo nao foi previamente aberto em algum momento do programa.Se nao, significa que a operaçao e uma inserçao, caso contrario ele foi aberto para ediçao.
	{
		mainFile.open(MainFileName.c_str(), ios::out | ios::binary | ios::in);//abre o arquivo para operaçoes em binario de escrita
		mainFile.seekp(0, ios::end);//posiciona o ponteiro para fazer a inserçao no final do arquivo
	}


	if (mainFile.is_open() && mainFile.good())
	{
		unsigned int address = mainFile.tellp();//salva o endereço do registro a ser inserido

		/*
		* Como o nome e as notas estao sendo trabalhados como uma string, precisamos saber se os mesmos
		podem ser inseridos dentro do espaço disponivel para eles, pois as strings tem tamanhos variaveis.
		Sendo assim, é necessario ser feito a seguinte comparação onde:
		r.record.size = tamanho do registro
		espaço disponivel = tamanho do registro - (tamanho fixo de variveis do registro + tamanho das duas strings)
		variaveis de tamanhos fixo = cpf(unsigned int), data de nascimento(conjunto de 3 inteiros sem sinais), gênero(um char), tamanho das duas strings(um unsigned int)

		Nota:
		É importante ressaltar que,como estamos trabalhando como strings para armazenar o nome e as notas,é necessario armazenar o tamanho destas no arquivo mestre
		para que a mesma possa ser lida quando uma operaçao de leitura do registro for solicitada

		Caso as duas strings caimbam juntas dentro do espaço disponivel, a condiçao if a seguir é executada.
		*/

		if (r.name.size() + r.notes.size() <= (r.record_size - (sizeof(r.Bday) + sizeof(r.cpf) + sizeof(r.gender) + 2 * sizeof(size))))
		{
			mainFile.write((char*)&r.cpf, sizeof(r.cpf));
			size = r.name.size();//obtem o tamanho da string nome
			mainFile.write((char*)&size, sizeof(size_t));//salva o tamanho da string nome
			mainFile.write(r.name.c_str(), r.name.size());//insere totalmente a string nome
			mainFile.write((char*)&r.gender, sizeof(r.gender));//insere o caracter que define o genero
			mainFile.write((char*)&r.Bday, sizeof(Bday));//insere a data de nascimento
			size = r.notes.size();// obtem o tamanho da string nota
			mainFile.write((char*)&size, sizeof(size_t));// salva o tamanho da string notas
			mainFile.write(r.notes.c_str(), r.notes.size());// insere totalmente a string notas

			offset = record_size - (sizeof(r.Bday) + sizeof(r.cpf) + sizeof(r.gender) + r.name.size() + r.notes.size() + 2 * sizeof(size));//calcula o tamanho do offset

			/*
			Nota:
			o tamanho do offset procura saber quantos bytes sao necessarios deslocar o EOF para que o registro final tenho o tamanho correto definido.
			*/

			setEOF(offset);//seta o EOF do arquivo deslocando a quantidade de bytes necessaria para atingir o tamanho do registro

		}
		else
		{
			/*
			Caso não há espaço para inserir as duas strings de forma completa, estas serão inseridas até que não haja mais espaço para inserir os caracteres.
			Sendo assim, a inserção aqui se da de caracter a caracter,ou seja, cada caracter sera inserido individualmente ate que atinja o tamanho maximo do registro.
			Uma consequencia disso é que havera informação incompleta do campo de notas (julgando-se que o nome nunca sera grande suficiente a ponto de nao caber no registro)
			*/

			mainFile.write((char*)&r.cpf, sizeof(r.cpf));
			size = r.name.size();
			mainFile.write((char*)&size, sizeof(size_t));// salva o tamanho da string nome

			int availableSpace = (r.record_size - (sizeof(r.Bday) + sizeof(r.cpf) + sizeof(r.gender) + sizeof(size)));//calcula o espaço disponivel em bytes para a inserçao das duas strings
			int i;
			for (i = 0; i < r.name.length() && i < availableSpace; i = i + 1)
			{
				mainFile.write((char*)&r.name.at(i), sizeof(char));//insere a string nome caracter a caracter
				availableSpace = availableSpace - 1;// a cada inserçao de caracter subtrai um byte do espaço disponivel
			}


			mainFile.write((char*)&r.gender, sizeof(r.gender));
			mainFile.write((char*)&r.Bday, sizeof(Bday));

			availableSpace = availableSpace - sizeof(size);
			size = availableSpace;//o tamanho salve da string notas sera o espaço restante disponivel para a escrita.
			mainFile.write((char*)&size, sizeof(size_t));// escreve o tamanho da string nota

			for (i = 0; i < availableSpace; i = i + 1)
			{
				mainFile.write((char*)&r.notes.at(i), sizeof(char));//escreve caracter a caracter a string nota ate que nao haja mais espaço disponivel
			}

			cout << "Error: record's size limit has been reached. there might be an imcomplete information" << endl;
		}

		mainFile.close();

		return address;//retorna o endereço onde o registro foi escrito
	}
	else
	{
		cout << "could not open the file or there might be some problem with it" << endl;

		mainFile.close();

		return 1;//retorna um caso nao consiga ser escrito
	}
}
bool fileManagement::addRecord(Record r)
{
	start_t = clock();

	unsigned int address = writeRecord(r);

	if(address != 1)
	LinkDirectoryToIndex(r.cpf, address);//chama a funçao responsavel por manipular o diretorio e que trabalha em conjunto com a funçao responsavel por manipular os indices.
	else
	{
		return false;
	}

	/*
	Observaçao: é passado como argumento para a funçao acima o endereço do registro que acabou de ser adicionado e seu respectivo cpf para que
	as duas funçoes principais responsáveis pela hash dinâmica trabalhem para adicionar esse endereço e cpf numa pagina do arquivo de indices.
	*/

	end_t = clock();

	/*
	*-obs: essa parte abaixo atrapalha a simulação, pois ela printa o tempo toda vez que insere um registro.

	cout.precision(5);
	cout.setf(ios::fixed, ios::floatfield);

	cout << (float)(end_t - start_t) / CLOCKS_PER_SEC << "s for writing" << endl;
	cout.unsetf(ios::floatfield);

	cout << "press ENTER to continue..." << endl;
	getchar();
	*/

}

Record fileManagement::readRecord(unsigned int offset)
{
	Record tmp;
	
	/*
	Verifica se o arquivo nao esta aberto, se nao estiver aberto, abre o arquivo para realizar a leitura do registro
	*/
	if (!mainFile.is_open())
	{
		mainFile.open(MainFileName.c_str(), ios::in | ios::binary);
		mainFile.seekg(offset, ios::beg);//desloca o ponteiro para a posiçao onde o registro se encontra antes de fazer a leitura
	}

	if (mainFile.is_open())
	{
		size_t size;//variavel para guardar o tamanho das strings nome e notas na leitura

		mainFile.read((char*)&tmp.cpf, sizeof(tmp.cpf));//le o cpf
		mainFile.read((char*)&size, sizeof(size));//le o tamanho da string nome
		tmp.name.resize(size);//redimensiona o tamanho da string nome
		mainFile.read((char*)tmp.name.c_str(), size);//le a string nome do arquivo
		mainFile.read((char*)&tmp.gender, sizeof(tmp.gender));//le o genero
		mainFile.read((char*)&tmp.Bday, sizeof(Bday));//le a data de nascimento
		mainFile.read((char*)&size, sizeof(size));//le o tamanho da string notas
		tmp.notes.resize(size);//redimensiona o tamanho da string notas
		mainFile.read((char*)tmp.notes.c_str(), size);//le a string notas do arquivo
	}

	return tmp;// retorna o registro lido
}

bool fileManagement::edit(unsigned int cpf, string notes)
{
	mainFile.open(MainFileName.c_str(), ios::out | ios::in | ios::binary);//abre o arquivo para ediçao
	
	start_t = clock();// marca o inicio do tempo para calcular o tempo de ediçao

	/*
	*verifica se o array do diretorio e nulo. se este for nulo, fecha arquivo e retorna falso indicando que nao pode ser feito a ediçao
	*/

	if (array == NULL)
	{
		mainFile.close();
		return false;
	}

	unsigned int indexAddress = array[hash.hashFunction(cpf)];// pega qual o endereço da pagina onde o registro de chave/cpf pode ser encontrado
	unsigned int mainAddress = search(indexAddress, cpf);//usado o endereço da pagina adquirido acima do vetor do diretorio para procurar pela chave e seu endereço no arquivo mestre

	/*
	Caso a funçao search retorne 1, isso quer dizer que nao foi possivel achar a chave no arquivo de indices.
	Portanto, fecha-se o arquivo e retorne falso indicando que a ediçao nao pôde ser feita.

	obs:A funçao search retorna o endereço da chave caso ela seja bem sucedida na pesquisa.
	*/

	if (mainAddress == 1)
	{
		mainFile.close();
		return false;
	}

	Record r;

	mainFile.seekg(mainAddress, ios::beg);//posiciona o ponteiro do arquivo mestre para a posiçao do registro a ser editado
	r = readRecord(mainAddress);//le o registro

	end_t = clock();// marca o fim do tempo para calcular o tempo de ediçao

	cout.precision(5);//seta uma precisao de 5 casas decimais para printar o tempo de pesquisa
	cout.setf(ios::fixed, ios::floatfield);//seta o campo floatfield para o modo fixed para que todo output de numero float seja printado nesse modo

	cout << (float)(end_t - start_t) / CLOCKS_PER_SEC << "s to retrieve the record" << endl;//printa o tempo
	cout.unsetf(ios::floatfield);//reseta o campo floatfield
	cout << "press ENTER to continue..." << endl;
	getchar();

	r.notes = notes;// atribui a string notas do registro a ediçao a ser feita.

	mainFile.seekp(mainAddress, ios::beg);//reposiciona o ponteiro para a posiçao original de onde o registro foi lido

	writeRecord(r);//reescreve o registro lido

	return true;
}

bool fileManagement::remove(unsigned int cpf)
{
	if (array == NULL)
	{
		return false;
	}

	unsigned int indexAddress = array[hash.hashFunction(cpf)];// pega qual o endereço da pagina onde o registro de chave/cpf pode ser encontrado para ser deletado/removido logicamente

	return deleteCpf(indexAddress,cpf);//chama a funçao responsavel por fazer a remoçao logica
}

void fileManagement::printMainFile()
{
	Record tmp;
	int offset = 0;

	mainFile.open(MainFileName.c_str(), ios::in | ios::binary);//abre o arquivo para leitura

	if (mainFile.is_open())
	{
		if (mainFile.peek() != EOF)//verifica se o arquivo fa vazio
		{
			while (mainFile.peek() != EOF)//faz a leitura enquanto o fim do arquivo nao e encontrado
			{
				mainFile.seekg(offset, ios::beg);//posiciona o ponteiro para fazer a leitura do registro
				if (mainFile.peek() != EOF)//faz a leitura se diferente do fim do arquivo
				{
					tmp = readRecord(offset);

					if (search(array[hash.hashFunction(tmp.cpf)], tmp.cpf) != 1)//verifica se é um registro valido que esta no indice
					{
						printRecord(tmp);//printa o registro
						cout << endl;
					}
				}

				offset = offset + record_size;// desloca o offset para o registro seguinte
			}
		}
		else
		{
			cout << "file is empty" << endl;
		}
	}
	else
	{
		cout << "Couldn't print file" << endl;
	}

	mainFile.close();
}

void fileManagement::printDirectoryFile()
{

	if (array == NULL)
	{
		cout << "file is empty" << endl;
	}
	else
	{
		for (unsigned int i = 0; i < (unsigned int)pow(2, hash.depth); i++)
		{
			cout << "[" << i << "] = ";

			cout.setf(ios::hex, ios::basefield);//seta o campo base para hexa para que os valores sejam printados em hexadecimal
			cout.setf(ios::showbase);

			cout << array[i] << endl;//printa o vetor do diretorio

			cout.unsetf(ios::hex | ios::showbase);
		
		}
	}
}

void fileManagement::printIndexFile()
{
	Page p;
	int i = 1;
	if (indexFile.is_open())
		indexFile.close();

	indexFile.open(IndexFileName.c_str(), ios::in | ios::binary);

	if (indexFile.is_open())
	{
		if (indexFile.peek() != EOF)
		{
			while (indexFile.peek() != EOF)
			{
				cout << "Page " << i << endl;
				p = readPage();//le uma pagina
				printPage(p);//printa a pagina

				p.deletePage();
				i++;
			}
		}
		else
		{
			cout << "file is empty" << endl;
		}
	}
	else
	{
		cout << "Couldn't print file" << endl;
	}

	indexFile.close();
}

void fileManagement::printFiles()
{

	/*
	* essa funçao so tem o papel de printar todos os arquivos juntos.
	*/

	cout << "Directory file:" << endl;
	printDirectoryFile();

	cout << "\n";

	cout << "Index file:" << endl;
	printIndexFile();

	cout << "\n";

	cout << "Main file:" << endl;
	printMainFile();
}

void fileManagement::LinkDirectoryToIndex(unsigned int cpf, unsigned int address)
{
	if (directoryFile.is_open())
		directoryFile.close();

	directoryFile.open(DirectoryFileName.c_str(), ios::in | ios::binary);

	Bucket bucket;

	/*
	Armazena o cpf e o endereço do registro do arquivo mestre no objeto b da class bucket.
	*/
	Bucket b;
	b.cpf = cpf;
	b.Address = address;

	if (directoryFile.peek() == EOF)//verifica se o arquivo esta vazio
	{
		/*
		OBS:Esse if so roda uma vez a cada sistema novo de arquivo criado,pois ele so roda quando o arquivo do diretorio esta vazio
		*/

		directoryFile.close();
		directoryFile.open(DirectoryFileName.c_str(), ios::out | ios::binary | ios::app);
		if (directoryFile.is_open())
		{
			directoryFile.write((char*)&hash.depth, sizeof(hash.depth));//escreve a profundidade inicial do diretorio que é 0
			bucket = LinkIndexToMain(0, b);//chama a funçao que manipula o arquivo de indices para inserir o novo bucket na pagina

			/*
			A funçao linkIndexToMain retorna um bucket que é usado como flag para realizçao de algumas operaçoes nesta funçao(LinkDirectoryToIndex).
			Alem disso, quando sucedido em realizar o link entre o main e o diretorio, a funçao retorna o endereço da pagina onde o bucket foi inserido
			atraves do membro endereço da class bucket.
			*/

			directoryFile.write((char*)&bucket.Address, sizeof(unsigned int));//salva o endereço da primeira pagina
			
			directoryFile.close();

			updateArray();// atualiza o vetor do diretorio em memoria primaria

		}	
	}
	else
	{
		directoryFile.close();

		/*
		A linha de codigo logo a seguir chama a funçao responsavel por armazenar o novo bucket no indice e fazer o link entre o arquivo mestre e o indice.
		Ela passa como argumento o endereço da pagina o bucket deve ser inserido e o bucket que deve ser inserido.
		*/

		bucket = LinkIndexToMain(array[hash.hashFunction(cpf)], b);

		/*
		Internamente a funçao LinkIndexToMain modifica a flag is_linked da class hash. Caso essa flag seja falsa, quer dizer que o bucket nao pôde ser inserido,
		caso contrario o bucket foi inserido com sucesso.
		*/

		if (!hash.is_linked)
		{
			/*
			Caso a funçao retorne um objeto do tipo bucket cujo o membro endereço e igual a 1, isso quer dizer que é necessario fazer uma expansao do diretorio
			*/

			if (bucket.Address == 1)
			{
				unsigned int* indicesArray;//ponteiro temporario para geraçao do novo bloco de memoria contendo o diretorio
				int i = 0;

				free(array); // reseta o array do diretorio antes de fazer a alocaçao do novo bloco de memoria do diretorio
				array = NULL;

				directoryFile.open(DirectoryFileName.c_str(), ios::in | ios::binary);

				hash.depth++;//aumenta a profundidade global da hash

				indicesArray = (unsigned int*)malloc(pow(2, hash.depth) * sizeof(unsigned int));//aloca o novo bloco de memorias do diretorio

				if (indicesArray == NULL)
				{
					cout << "error: could not allocate memory for directory array" << endl;
					exit(0);
				}

				directoryFile.seekg(4, ios::beg);//define a posiçao inicial do ponteiro para ler o conteudo antigo do diretorio excluindo a profundidade

				while (directoryFile.peek() != EOF)// le o diretorio enquanto nao for igual ao fim do arquivo
				{
					directoryFile.read((char*)&indicesArray[i], sizeof(unsigned int));//le um por um os elementos do diretorio para dentro do vetor temporario 
					i++;//incrementa a variavel de controle do vetor
				}

				directoryFile.close();

				while (i < (int)pow(2, hash.depth))
				{
					/*
					Aponta cada posiçao nova gerada na expansao do diretorio para a posiçao correspondente.
					O apontamento acontece da seguinte forma:

					Posiçao nova = (primeira posiçao nova) - ((2^profundidade_global)/2)

					*/

					indicesArray[i] = indicesArray[i - (int)(pow(2, hash.depth) / 2)];
					i++;
				}

				directoryFile.open(DirectoryFileName.c_str(), ios::out | ios::binary);

				directoryFile.write((char*)&hash.depth, sizeof(hash.depth));//escreve a nova profundidade global
				directoryFile.write((char*)indicesArray, sizeof(unsigned int) * pow(2, hash.depth));//escreve o novo diretorio

				directoryFile.close();
				free(indicesArray);
				updateArray();//atualiza o vetor do diretorio em memoria primaria

				LinkDirectoryToIndex(cpf, address);// tenta inserir o bucket denovo chamando esta funçao recursivamente

			}
			else
			{
				/*
				Caso a funçao retorne diferente de 1, isso quer dizer que ela retornou um endereço de uma nova pagina que foi criada,mas ainda nao foi
				possivel inserir o bucket, pois todos os elementos cairam na nova pagina dps do rehash.
				Sendo assim é necessario fazer o rearranjo novo dos ponteiros por conta da nova pagina criada
				*/

				RearrangePointers(bucket.cpf, nBits(bucket.cpf) + 1, bucket.Address);//faz o rearranjo dos ponteiros do diretorio

				/*
				OBS:A funçao linkIndexToMain atribui ao cpf nesse caso uma posiçao do vetor do diretorio o qual o endereço da nova pagina deve ser atribuido
				*/

				array[bucket.cpf] = bucket.Address;//atribui determinada posiçao do vetor do diretorio o endereço da nova pagina

				directoryFile.open(DirectoryFileName.c_str(), ios::out | ios::binary);

				directoryFile.write((char*)&hash.depth, sizeof(hash.depth));//escreve a profundidade global
				directoryFile.write((char*)array, sizeof(unsigned int) * pow(2, hash.depth));//escreve o novo vetor do diretorio

				directoryFile.close();

				LinkDirectoryToIndex(cpf, address);//tenta inserir o bucket novamente de forma recursiva
			}
		}
		else
		{
			/*
			Caso o is_linked seja verdadeiro, é necessario fazer um rearranjo dos ponteiros caso o endereço retornado tenha sido diferente de 1, pois,
			se ele foi diferente de um, quer dizer que uma nova pagina foi criada para inserir o novo bucket fazendo-se então necessário um rearranjo
			dos ponteiros do vetor do diretorio para comportar a nova pagina.
			*/

			if (bucket.Address != 1)
			{
				
				RearrangePointers(bucket.cpf, nBits(bucket.cpf) + 1, bucket.Address);

				array[bucket.cpf] = bucket.Address;

				directoryFile.open(DirectoryFileName.c_str(), ios::out | ios::binary);

				directoryFile.write((char*)&hash.depth, sizeof(hash.depth));
				directoryFile.write((char*)array, sizeof(unsigned int) * pow(2, hash.depth));

				directoryFile.close();
			}
		}
	}
}

 Bucket fileManagement::LinkIndexToMain(unsigned int pos, Bucket b)
{
	 if (indexFile.is_open())
		 indexFile.close();

	Page temp(hash.Buckets_MAX);//cria uma pagina nova com tamanho maximo de buckets definido pelo membro do objeto da class hash
	Bucket returnBucket;

	temp.createPage(temp.nBuckets_MAX);//reserva dinamicamente uma quantidade de espaços para os buckets serem inseridos nas paginas
	indexFile.open(IndexFileName.c_str(), ios::in | ios::binary);

	if (indexFile.peek() == EOF)
	{
		/*
		Notas:Assim como na funçao linkDirectoryToIndex, esse if so roda uma vez parada cada sistema de arquivos novos.
		*/

		indexFile.close();
		indexFile.open(IndexFileName.c_str(), ios::out | ios::binary);
		returnBucket.Address = indexFile.tellp();//armazena o endereço da pagina criada e a ser escrita
		returnBucket.cpf = 0;
		
		temp.b[temp.nBuckets] = b;//insere o bucket na primeira posiçao
		temp.nBuckets++;//incrementa a quantidade de buckets e permiti que um proximo bucket seja inserido na posiçao seguinte
		hash.page = temp;//atribui a hash a pagina a ser inserida
		writePage(hash.page);//escreve a pagina no arquivo

		temp.deletePage();//deleta a pagina temporaria criada para inserçao
		indexFile.close();

		hash.is_linked = true;//muda a flag para true indicando que foi possivel inserir com sucesso o novo bucket

		return returnBucket;//retorna um bucket como flag

	}
	else
	{
		indexFile.close();
		indexFile.open(IndexFileName.c_str(), ios::in | ios::binary);
		Page temp2;

		indexFile.seekg(pos, ios::beg);// posiciona o ponteiro para a posiçao da pagina onde o bucket dever ser inserido
		temp2 = readPage();// le a pagina

		indexFile.close();
		indexFile.open(IndexFileName.c_str(), ios::out | ios::in | ios::binary);

		if (temp2.nBuckets != temp2.nBuckets_MAX)// verifica se a pagina ja esta cheia.se nao estiver,executa o if abaixo
		{
			indexFile.seekp(pos, ios::beg);//posiciona o ponteiro para a posiçao da pagina onde o bucket deve ser inserido
			returnBucket.Address = 1;//retorna 1 indicando que nenhuma pagina foi criada
			returnBucket.cpf = 0;
			
			temp2.b[temp2.nBuckets] = b;//adiciona o bucket a pagina
			temp2.nBuckets++;//aumenta o contador de buckets por pagina
			hash.page = temp2;

			writePage(hash.page);//reescreve a pagina no arquivo de indices novamente

			temp.deletePage();
			temp2.deletePage();
			indexFile.close();
			
			hash.is_linked = true;

			return returnBucket;
		}
		else
		{
			if (temp2.bucketDepth == hash.depth)//verifica se a profundidade da pagina lida e igual a profundidade global da hash
			{
				temp.deletePage();
				temp2.deletePage();
				indexFile.close();

				returnBucket.Address = 1;//retorna 1 indicando que nenhuma pagina foi criada
				returnBucket.cpf = 0;

				hash.is_linked = false;//muda a flag global para falso indicando que nao foi possivel inserir o bucket

				return returnBucket;
			}
			else
			{	
				/*
				Caso a profundidade local da pagina nao seja igual a profundidade global da hash, nao e necessario indicar a funçao
				linkDirectoryToIndex a necessidade de fazer uma expansao. Sendo assim, a medida a tomar e criar uma nova pagina e tentar 
				inserir o novo bucket e retornar o endereço da nova pagina criada para possibilitar o rearranjo dos ponteiros no diretorio.
				*/

				Page temp3(hash.Buckets_MAX);
				temp3.createPage(temp3.nBuckets_MAX);

				unsigned int key = hash.hashFunction(b.cpf);//pega por qual ponteiro(numero de uma posiçao do vetor do diretorio) o bucket de cpf dado sera apontado INICIALMENTE.
				unsigned int NumberOfBits = nBits(key);//calcula quants bits sao necessarios para representar aquela posiçao

				int i;//variavel de controle do loop

				/*
				O if abaixo se faz necessário, pois em grandes escalas se a posiça do diretorio for 0 OU qualquer posiçao que ja tenha sido atribuida um valor de endereço
				de uma pagina criada anteriormente é necessario que o proximo ponteiro diretamente relacionado a posiçao no vetor do diretorio calculado acima seja usado
				para referenciar a nova pagina.
				OBS: O proximo ponteiro diretamente relacionado nao pode estar referenciando a mesma pagina. Isso seria exemplificado no relatorio.
				*/

				if (key == 0 || !compareAddresses(key, key - (unsigned int)(pow(2, NumberOfBits) / 2)))
				{

					if (key != 0)
						i = NumberOfBits + 1;
					else
						i = NumberOfBits;

					while (i <= hash.depth)
					{
						/*
						Procura o proximo ponteiro que se relaciona com o ponteiro INICIALMENTE calculado anteriormente e que nao referencia a mesma pagina
						*/
						if (compareAddresses(key, key + (unsigned int)(pow(2, i) / 2)))
						{
							key = key + (unsigned int)(pow(2, i) / 2);
							break;
						}

						i++;
					}

					NumberOfBits = nBits(key);//recalcula a nova quantidade de bits

					for (i = 0; i < temp2.nBuckets_MAX; i++)
					{
						if (temp2.b[i].cpf % (unsigned int)pow(2, temp2.bucketDepth + 1) == key)// faz o rehash com a nova profundidade da pagina
						{
							temp.b[temp.nBuckets] = temp2.b[i];//passa o bucket da pagina antiga para a pagina nova caso o rehash possibilite
							temp2.nBuckets--;
							temp.nBuckets++;//aumenta o contador de buckets da pagina nova
						}
						else
						{
							/*
							Faz o rehash com todos os elementos conectados a raiz. A raiz é a posiçao no vetor do diretório que ira referenciar a nova pagina a ser inserida.
							os elementos conectados/referenciados a raiz pode ser obtido da seguinte forma:
							elemento = raiz + pow(2,quantidade de bits)/2 
							
							Sendo que a quantidade de bits esta no seguinte intervalo:

							quantidade de bits da raiz < quantidade de bits <= profundidade global

							OBS: para calcular todos os elementos e necessario que a quantidade de bits assuma todos os valores possiveis dentro daquele intervalo.
							Isso faz parte da funçao is_linked e sera detalhado por meio de um simples diagrama como exemplo no relatorio.
							*/

							if (is_linked(key, NumberOfBits + 1, temp2.b[i].cpf, temp2.bucketDepth + 1))
							{
								temp.b[temp.nBuckets] = temp2.b[i];
								temp2.nBuckets--;
								temp.nBuckets++;
							}
							else
							{
								/*
								Caso o rehash nao e bem sucedido, isso significa que o bucket deve permanecer na pagina antiga.
								A pagina que assumira o papel de "pagina antiga" sera a pagina temporaria temp3. Em outras palavras,
								o bucket que nao teve o cpf bem sucedido no rehash sera transferido para o temp3 e posteriormente escrito no lugar
								da pagina antiga.
								*/

								temp3.b[temp3.nBuckets] = temp2.b[i];
								temp3.nBuckets++;//aumenta o contador de buckets
							}
						}
					}

					temp3.bucketDepth = temp2.bucketDepth + 1;//define a profundidade nova da pagina antiga
					temp.bucketDepth = NumberOfBits;//define a profundidade inicial da pagina nova a ser inserida

					if (key != 0)
						i = NumberOfBits + 1;
					else
						i = NumberOfBits;


					/*
					O loop while abaixo e responsavel por tentar recalcular a profundidade da nova pagina a ser inserida.
					Isso se faz necessario, pois a profundidade local da nova pagina a ser inserida depende diretamente de quantos ponteiros estarao referenciando ela.
					Em outras palavras, inicialmente se tem que a profundidade da nova pagina e o numero de bits necessario para representar a posiçao no vetor do diretorio
					que ira referenciar a nova pagina. Sendo assim, se essa quantidade de bits for menor que a profundidade local isso quer dizer que a nova pagina pode ter
					mais um ponteiro apontando para ela alem do qual ja foi definido que ira apontar. Entretanto, esse ponteiro a mais,que é a posiçao no vetor do diretorio,
					ja pôde ter sido atribuido o endereço de uma nova pagina anteriormente criada.Sendo este o caso, essa pagina nao sera apontada por esses ponteiros que ja tiverem
					sido atribuidos o endereço de uma nova pagina.

					OBS: Os ponteiros "extras" que podem ou nao apontar para a nova pagina a ser criada sao aqueles que se relacionam/referenciam ao ponteiro raiz inicialmente
					definido para abrigar o endereço da nova pagina.

					*/
					while (i <= hash.depth)
					{

						if (!compareAddresses(key, key + (unsigned int)(pow(2, i) / 2)))
						{
							temp.bucketDepth++;
						}

						i++;
					} 

					if (temp3.nBuckets != temp3.nBuckets_MAX)//insere o novo bucket se a a nova pagina nao estiver cheia
					{
						temp3.b[temp3.nBuckets] = b;//adiciona o bucket na pagina 
						temp3.nBuckets++;

						indexFile.seekp(pos, ios::beg);
						hash.page = temp3;
						writePage(hash.page);//escreve a pagina antiga com o bucket adicionado

						indexFile.seekp(0, ios::end);
						returnBucket.Address = indexFile.tellp();//armazena o endereço da nova pagina a ser inserida no bucket de retorno
						returnBucket.cpf = key;//retorna a posiçao no vetor do diretorio que deve referenciar a nova pagina

						hash.page = temp;
						writePage(hash.page);//escreve a pagina nova com os buckets rearranjados nela

						temp.deletePage();
						temp2.deletePage();
						temp3.deletePage();
						indexFile.close();

						hash.is_linked = true;

						return returnBucket;
					}
					else
					{
						indexFile.seekp(pos, ios::beg);
						hash.page = temp3;
						writePage(hash.page);

						indexFile.seekp(0, ios::end);
						returnBucket.Address = indexFile.tellp();
						returnBucket.cpf = key;

						hash.page = temp;
						writePage(hash.page);

						temp.deletePage();
						temp2.deletePage();
						temp3.deletePage();
						indexFile.close();

						hash.is_linked = false;

						return returnBucket;
					}
				}
				else
				{
					
					for (i = 0; i < temp2.nBuckets_MAX; i++)
					{
						if (temp2.b[i].cpf % (unsigned int)pow(2, temp2.bucketDepth + 1) == key)
						{
							temp.b[temp.nBuckets] = temp2.b[i];
							temp2.nBuckets--;
							temp.nBuckets++;
						}
						else
						{
							if (is_linked(key, NumberOfBits + 1, temp2.b[i].cpf, temp2.bucketDepth + 1))
							{
								temp.b[temp.nBuckets] = temp2.b[i];
								temp2.nBuckets--;
								temp.nBuckets++;
							}
							else
							{
								temp3.b[temp3.nBuckets] = temp2.b[i];
								temp3.nBuckets++;
							}
						}
					}

					temp3.bucketDepth = temp2.bucketDepth + 1;
					temp.bucketDepth = NumberOfBits;

					if (key != 0)
						i = NumberOfBits + 1;
					else
						i = NumberOfBits;

					while (i <= hash.depth)
					{

						if (!compareAddresses(key, key + (unsigned int)(pow(2, i) / 2)))
						{
							temp.bucketDepth++;
						}

						i++;
					}

					if (temp.nBuckets != temp.nBuckets_MAX)
					{
						temp.b[temp.nBuckets] = b;
						temp.nBuckets++;

						indexFile.seekp(pos, ios::beg);
						hash.page = temp3;
						writePage(hash.page);

						indexFile.seekp(0, ios::end);
						returnBucket.Address = indexFile.tellp();
						returnBucket.cpf = key;

						hash.page = temp;
						writePage(hash.page);

						temp.deletePage();
						temp2.deletePage();
						temp3.deletePage();
						indexFile.close();

						hash.is_linked = true;

						return returnBucket;
					}
					else
					{
						indexFile.seekp(pos, ios::beg);
						hash.page = temp3;
						writePage(hash.page);

						indexFile.seekp(0, ios::end);
						returnBucket.Address = indexFile.tellp();
						returnBucket.cpf = key;

						hash.page = temp;
						writePage(hash.page);

						temp.deletePage();
						temp2.deletePage();
						temp3.deletePage();
						indexFile.close();

						hash.is_linked = false;

						return returnBucket;
					}
				}
			}
		}
	}
}

/*
A funçao membro abaixo sera detalhada com um diagrama exemplo no relatorio
*/

void fileManagement::RearrangePointers(unsigned int root, unsigned int nbits, unsigned int address)
{
	if (nbits < hash.depth)
	{
		RearrangePointers(root, nbits + 1, address);
		RearrangePointers(root + (unsigned int)(pow(2, nbits) / 2), nbits + 1, address);
	}

	if (nbits <= hash.depth)
	{
		unsigned int index = root + unsigned int(pow(2, nbits) / 2);
		unsigned int bitsNumber = nBits(index);

		if (compareAddresses(index, index - unsigned int(pow(2, bitsNumber) / 2)))
		{
			array[index] = address;
		}
	}
}

/*------------------------indexManagement member functions------------------------------*/

void indexManagement::writePage(Page p)
{
	indexFile.write((char*)&p.nBuckets, sizeof(p.nBuckets));
	indexFile.write((char*)&p.bucketDepth, sizeof(p.bucketDepth));
	indexFile.write((char*)p.b, sizeof(Bucket)*p.nBuckets_MAX);
}

Page indexManagement::readPage()
{
	Page p(hash.Buckets_MAX);
	p.createPage(p.nBuckets_MAX);

	indexFile.read((char*)&p.nBuckets, sizeof(p.nBuckets));
	indexFile.read((char*)&p.bucketDepth, sizeof(p.bucketDepth));
	indexFile.read((char*)p.b, sizeof(Bucket)*p.nBuckets_MAX);

	return p;
}

void indexManagement::printPage(Page p)
{
	cout << "quantidade de buckets = " << p.nBuckets << "\t";
	cout << "p = " << p.bucketDepth << endl;
	
	for (int i = 0; i < p.nBuckets_MAX; i++)
	{
		cout << "cpf = " << p.b[i].cpf << endl;

		cout.setf(ios::hex, ios::basefield);
		cout.setf(ios::showbase);

		cout << "address = " << p.b[i].Address << "\n\n";

		cout.unsetf(ios::hex | ios::showbase);
	}
}

unsigned int indexManagement::nBits(unsigned int n)
{
	if (n == 0)
		return 1;

	return log2(n) + 1;
}

/*
A funçao membro abaixo sera detalhada com um diagrama exemplo no relatorio
*/

bool indexManagement::is_linked(unsigned int root, unsigned int nbits, const unsigned int number, const unsigned int NewBucketDepth)
{
	if (nbits < hash.depth)
	{
		return (number % (int)pow(2, NewBucketDepth) == root + (int)(pow(2, nbits) / 2)) || is_linked(root, nbits + 1, number, NewBucketDepth) || 
			is_linked(root + (int)(pow(2, nbits) / 2), nbits + 1, number, NewBucketDepth);
	}
	else
	{
		return (number % (int)pow(2, NewBucketDepth) == root + (int)(pow(2, nbits) / 2));
	}
}

unsigned int* indexManagement::readDirectory()
{
	unsigned int* array = NULL;
	if (directoryFile.is_open())
		directoryFile.close();

	directoryFile.open(DirectoryFileName.c_str(), ios::in | ios::binary);

	if (directoryFile.is_open())
	{
		if (directoryFile.peek() != EOF)
		{
			directoryFile.read((char*)&hash.depth, sizeof(hash.depth));//le a profundidade global
			array = (unsigned int*)malloc(pow(2, hash.depth) * sizeof(unsigned int));//reserva um espaço para vetor que ira conter os valores de endereços dos indices apontados pelo diretorio.

			directoryFile.read((char*)array, pow(2, hash.depth) * sizeof(unsigned int));//le o diretorio para dentro do vetor
		}
		else
		{
			directoryFile.close();
			return array;
		}
	}

	directoryFile.close();

	return array;//retorna uma referencia do vetor criado
}

unsigned int indexManagement::search(unsigned int address, unsigned int cpf)
{
	Page p;
	unsigned int addressReturn;

	if (indexFile.is_open())
		indexFile.close();

	indexFile.open(IndexFileName.c_str(), ios::in | ios::binary);

	if (indexFile.peek() != EOF)
	{
		indexFile.seekg(address, ios::beg);
		p = readPage();
		
		indexFile.close();
	}
	else
	{
		indexFile.close();

		return 1;
	}

	/*
	Tentar achar o bucket que contem o cpf informado.Caso o bucket e achado, retorna o endereço do arquivo mestre referente ao cpf passado como parametro.
	Caso contrario, retorna 1.
	*/
	for (int i = 0; i < p.nBuckets; i++)
	{
		if (p.b[i].cpf == cpf)
		{
			addressReturn = p.b[i].Address;

			p.deletePage();
			return addressReturn;
		}
	}

	p.deletePage();

	return 1;
}

bool indexManagement::deleteCpf(unsigned int address, unsigned int cpf)
{
	Page p;

	if (indexFile.is_open())
		indexFile.close();

	indexFile.open(IndexFileName.c_str(), ios::in | ios::out | ios::binary);

	if (indexFile.peek() != EOF)
	{
		indexFile.seekg(address, ios::beg);//posiciona o ponteiro para ler a pagina
		p = readPage();
	}
	else
	{
		indexFile.close();

		return false;
	}

	/*
	Procura pelo bucket que contem o cpf informado. Se achar este bucket, zera o cpf e retorna true. caso nao ache, o loop for
	sera encerrado e um false sera retornado
	*/

	for (int i = 0; i < p.nBuckets; i++)
	{
		if (p.b[i].cpf == cpf)
		{
			p.b[i].cpf = 0;

			indexFile.seekp(address, ios::beg);//reposiciona o ponteiro para reescrever a pagina

			writePage(p);//reescreve a pagina apos deletar logicamente o bucket
			
			indexFile.close();
			p.deletePage();

			return true;
		}
	}

	indexFile.close();
	p.deletePage();

	return false;
}

void indexManagement::updateArray()
{
	free(array); // libera o espaço antigo de memoria

	array = readDirectory();//atualiza o array utilizando-se de um novo espaço de memoria
}

bool indexManagement::compareAddresses(const int i, const int j)
{
	return (array[i] == array[j]) ? true : false;
}
