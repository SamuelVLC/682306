#include "simulation.h"

simulation::simulation() {}
simulation::~simulation() {}

bool simulation::runSimulation()
{
	short op;
	float writing_time, retrieval_time;
	string pendrivePath;
	string mainFile;
	string directoryFile;
	string indexFile;

	file.MainFileName = "main.txt";
	file.DirectoryFileName = "directory.txt";
	file.IndexFileName = "index.txt";

	cout << "Options:" << endl;
	cout << "1 - Record on pen-drive" << endl;
	cout << "2 - Record on local disk" << endl;

	cin >> op;
	cin.ignore();

	if (op == 1)
	{
		cout << "Type the drive letter of your pendrive" << endl;
		cout << "Example: C" << endl;
		getline(cin, pendrivePath);

		/*
		Define o caminho para escrita do arquivo no pendrive
		*/

		mainFile = pendrivePath + ":\\" + file.MainFileName;
		file.MainFileName = mainFile;

		directoryFile = pendrivePath + ":\\" + file.DirectoryFileName;
		file.DirectoryFileName = directoryFile;

		indexFile = pendrivePath + ":\\" + file.IndexFileName;
		file.IndexFileName = indexFile;
	}
	else
	{
		if (op == 2)
		{
			/*
			* define o nome do arquivos para serem gravados em disco
			*/

			pendrivePath = "simulationFile_";
			mainFile = pendrivePath + file.MainFileName;
			file.MainFileName = mainFile;

			directoryFile = pendrivePath + file.DirectoryFileName;
			file.DirectoryFileName = directoryFile;

			indexFile = pendrivePath + file.IndexFileName;
			file.IndexFileName = indexFile;
		}
		else
		{
			cout << "no valid option selected" << endl;
			cout << "press ENTER to go back to the main menu" << endl;
			getchar();
			return false;
		}
	}

	file.record_size = m;//atribui o tamanho do registro
	file.hash.Buckets_MAX = n;//atribui a quantidade maxima de buckets por pagina a hash
	file.createFiles();

	cout.setf(ios::fixed, ios::floatfield);
	cout.precision(2);

	writing_time = insertBlockOfRecords();
	retrieval_time = getBlockOfRecords();


	cout << writing_time << "s for writing" << endl;
	cout << retrieval_time << "s for retrieving" << endl;

	cout.unsetf(ios::floatfield);
	free(keysArray);

	return true;
}

void simulation::generateRandomCPFS()
{

	keysArray = (unsigned int*)malloc(k * sizeof(unsigned int));//reserva o espaço para as chaves a serem geradas

	for (unsigned int i = 1; i <= k; i++)
	{
		keysArray[i - 1] = i;//gera os valores para as chaves a principio sequencialmente
	}

	shuffleCPFS(0, k/10);//chamada para embaralhar as chaves no vetor de chaves
}

void simulation::shuffleCPFS(unsigned int start_position, unsigned int end_position)
{
	unsigned int temp;// variavel temporaria para fazer o swap
	unsigned int pos;//variavel para armazenar a posiçao aleatoria gerada pelo random para vetor de cpfs/chaves;
	unsigned int pos2;// mesma funçao do pos

	srand((unsigned)time(NULL));

	while (end_position <= k)
	{

		for (unsigned int i = 0; i < 10000000 + 1; i++)// por medida de eficiencia para embaralhar, foi colocado um valor bem alto
		{
			pos = rand() % (end_position - start_position) + start_position;//pega dois valores aleatorios entre a posiçao inicial e final que tem inicialmente os valores passados pelo argumento
			pos2 = rand() % (end_position - start_position) + start_position;

			// a seguir um swap e feito no vetor e troca dois valores de duas posiçoes aleatorias do vetor de chaves
			temp = keysArray[pos];
			keysArray[pos] = keysArray[pos2];
			keysArray[pos2] = temp;
		}

		/*
		As linhas abaixo atribuem o novo valor para a posiçao inicial e final de forma a tentar
		que os elementos sejam trocados entre posiçoes relativamente distantes uma da outra.
		Eventualmente com valores de chaves muito grande isso nao se torna efetivo
		*/

		start_position = end_position;
		end_position = end_position + k/10;
	}

	/*
	* embaralha uma ultima vez todo o conjunto de uma vez
	*/
	for (unsigned int i = 0; i < k; i++)
	{
		pos = rand() % k;
		pos2 = rand() % k;

		temp = keysArray[pos];
		keysArray[pos] = keysArray[pos2];
		keysArray[pos2] = temp;
	}
}

float simulation::insertBlockOfRecords()
{
	Record r;
	ostringstream name;
	clock_t start_time, end_time;

	generateRandomCPFS();//gera o bloco de cpfs pseudoaleatorio

	start_time = clock();

	for (unsigned int i = 1; i <= k; i++)
	{
		r.cpf = keysArray[i - 1];

		name << "name " << i;
		r.name = name.str();

		if (!file.addRecord(r))
			file.addRecord(r);//tenta escrever o elemento uma segunda vez caso a primeira falhe

		name.str("");
		name.clear();
	}

	end_time = clock();

	return (float)(end_time - start_time) / CLOCKS_PER_SEC;//retorna o tempo que foi gasto para escrever o bloco

}

float simulation::getBlockOfRecords()
{
	unsigned int offset;
	Record r;
	clock_t start_time, end_time;

	if (file.mainFile.is_open())
		file.mainFile.close();

	file.mainFile.open(file.MainFileName.c_str(), ios::in | ios::binary);//abre o arquivo mestre para leitura

	start_time = clock();

	for (unsigned int i = 0; i < k; i++)
	{
		offset = file.search(file.array[file.hash.hashFunction(keysArray[i])], keysArray[i]);//procura pelo cpf dado o endereço da pagina e o cpf
		
		if (offset != 1)
		{
			file.mainFile.seekg(offset, ios::beg);

			r = file.readRecord(offset);//faz a leitura do registro

			if (r.cpf == 0)//se o registro tiver cpf = 0 entao e um registro invalido 
			{
				cout << "invalid record. press ENTER to continue" << endl;
				getchar();
			}
			else
			{
				file.printRecord(r);
			}
		}
		else
		{
			cout << "could not find record.Press ENTER to continue search" << endl;
			getchar();
		}
	}

	end_time = clock();

	file.mainFile.close();

	return (float)(end_time - start_time) / CLOCKS_PER_SEC;// retorna o tempo de busca do bloco de chaves

}