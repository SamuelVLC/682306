/*
Author:Samuel Vieira L. Chiodi
Bruno Kelsen
Dayane Cordeiro

date:27/09/2020
*/

#include <stdbool.h>
#include "fileManagement.h"
#include "simulation.h"

using namespace std;

void menu()
{
    cout << "Options:" << endl;
    cout << "0 - Exit" << endl;
    cout << "1 - Create file system" << endl;
    cout << "2 - Add" << endl;
    cout << "3 - Edit" << endl;
    cout << "4 - Remove" << endl;
    cout << "5 - Print file's contents" << endl;
    cout << "6 - Run a simulation" << endl;
    
}


int main()
{ 

    short op;
    Record record;
    fileManagement file;
    simulation s;

    do
    {
        menu();
        cin >> op;
        cin.ignore();
        switch (op)
        {
        case 0:
            cout << "Exiting..." << endl;
            getchar();
            break;

        case 1:
 
            file.createFiles();

            break;

        case 2:

            cout << "cpf:";
            cin >> record.cpf;
            cout << endl;
            cin.ignore();
            cout << "name:";
            getline(cin, record.name);
            cout << endl;
            cout << "birthdate:" << endl;
            cout << "day:";
            cin >> record.Bday.day;
            cout << endl << "month:";
            cin >> record.Bday.month;
            cout << endl << "year:";
            cin >> record.Bday.year;
            cout << endl;
            cout << "gender:";
            cin >> record.gender;
            cout << endl;
            
            if (file.addRecord(record))
                cout << "successfully written!" << endl;

            break;

        case 3:
            
            cout << "digite o cpf do paciente para edicao de suas notas:";
            cin >> record.cpf;
            cout << endl;
            cin.ignore();
            cout << "Agora digite as notas:";
            getline(cin, record.notes);
            cout << endl;
       
            if (file.edit(record.cpf, record.notes))
                cout << "changes saved successfully!" << endl;
            else
                cout << "Record not found" << endl;
            
            break;

        case 4:

            cout << "digite o cpf do paciente para sua remocao" << endl;
            cin >> record.cpf;
            cin.ignore();
            cout << endl;

            if (file.remove(record.cpf))
                cout << "removal done successfully!" << endl;
            else
                cout << "could not remove data from cpf:" << record.cpf << endl;

            break;

        case 5:

            file.printFiles();

            break;

        case 6:

            cout << "digite o tamanho do registro: ";
            cin >> s.m;
            cout << endl;
            cout << "digite a quantidade de buckets por pagina: ";
            cin >> s.n;
            cout << endl;
            cout << "digite quantos registros a serem inseridos: ";
            cin >> s.k;
            cout << endl;

            s.runSimulation();

            break;

        default:
            cout << "please select a valid option" << endl;
            break;
        }

        cout << "press ENTER to continue" << endl;
        getchar();
        system("cls");

    } while (op != 0);

    return 0;

}
