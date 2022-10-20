#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <omp.h>
#include <regex>
#include <cstdio>
#include <iomanip>
using std::setw;
using namespace std;

#pragma region Structures

struct SelectedDataStructure {
    string StringProperty;
    int LinePosition;
};
struct SelectedDataStructureWithComputedValue {
    string DecryptedText;
    string EncryptedText;
    int LinePosition;
};
#pragma endregion

#pragma region Methods
void ReadData();
void WriteData();
void InsertionSort(vector<SelectedDataStructureWithComputedValue>& data, SelectedDataStructureWithComputedValue n);
void PopulateSubVectors(vector<vector<SelectedDataStructure>>& subvectors);
void Encrypt(vector<SelectedDataStructure> data);
string EncryptOneDataPoint(string string_line, bool EncryptedOne);
string ConvertToString(char* a, int size);
bool containsOnlyLetters(std::string const& str);

bool SortByNumber(SelectedDataStructureWithComputedValue& a, SelectedDataStructureWithComputedValue& b) {
    return a.LinePosition < b.LinePosition;
}
#pragma endregion

const string Input = "IFF01_VoronkeviciusS_L1_dat_1.txt";
const string Output = "IFF01_VoronkeviciusS_L1_rez.txt";
vector<SelectedDataStructure> selectedDataList;
vector<SelectedDataStructureWithComputedValue> selectedResultList;
int WorkerCount;

int main()
{
    //1. Nuskaito duomenų failą į lokalų masyvą, sąrašą ar kitą duomenų struktūrą;
    ReadData();

    //2. Padaliname vieną sarašą į daug atitinkamo dydžio sąrašų gijoms apdoroti
    WorkerCount = (selectedDataList.size() / 4) < 2 ? 2 : selectedDataList.size() / 4;
    vector<vector<SelectedDataStructure>> subvectors(WorkerCount);
    PopulateSubVectors(subvectors);

    //3. Paleidžia pasirinktą kiekį darbininkių gijų 2 ≤ x ≤ n/4 (n — duomenų kiekis faile).
    omp_set_num_threads(WorkerCount);
    int sum = 0;
#pragma omp parallel reduction(+:sum)
    {
        auto total_threads = omp_get_num_threads();
        vector<SelectedDataStructure> sub_vec;
#pragma omp critical
        {
            sub_vec = subvectors.front();
            subvectors.erase(subvectors.begin());
            sum += total_threads;
        }
        Encrypt(sub_vec);
    }

    //4. Rezultatus išvedame į tekstinio failo lentele.
    WriteData();
}

void PopulateSubVectors(vector<vector<SelectedDataStructure>>& subvectors)
{
    int subpart_of_vector = selectedDataList.size() / WorkerCount;
    int module_of_vector = selectedDataList.size() % WorkerCount;
    int counter = 0;
    for (vector<SelectedDataStructure>& sub_vec : subvectors)
    {
        for (auto i = 0; i < subpart_of_vector; i++)
        {
            sub_vec.push_back(selectedDataList.at(counter));
            counter++;
        }
        if (module_of_vector > 0)
        {
            sub_vec.push_back(selectedDataList.at(counter));
            counter++;
            module_of_vector--;
        }
    }
}

void Encrypt(vector<SelectedDataStructure> data)
{
    for (SelectedDataStructure item : data)
    {
        string EncText = EncryptOneDataPoint(item.StringProperty, true);
        string DecText = EncryptOneDataPoint(item.StringProperty, false);

        SelectedDataStructureWithComputedValue value;
        value.DecryptedText = DecText;
        value.EncryptedText = EncText;
        value.LinePosition = item.LinePosition;
        if (containsOnlyLetters(DecText))
        {
#pragma omp critical
            {
                //selectedResultList.push_back(value);
                //std::sort(selectedResultList.begin(), selectedResultList.end(), SortByNumber);
                InsertionSort(selectedResultList, value);
            }
        }
    }
}
void InsertionSort(vector<SelectedDataStructureWithComputedValue>& data, SelectedDataStructureWithComputedValue n)
{
    if (data.empty())
    {
        data.push_back(n);
        return;
    }
    if (data.front().LinePosition >= n.LinePosition)
    {
        data.insert(data.begin() + 0, n);
        return;
    }
    if (data.back().LinePosition <= n.LinePosition)
    {
        data.insert(data.end(), n);
        return;
    }

    for (int i = 0; i < data.size(); i++)
    {
        if (data[i].LinePosition <= n.LinePosition && data[i+1].LinePosition >= n.LinePosition )
        {
            data.insert(data.begin() + i+1, n);
            return;
        }
    }
}
string EncryptOneDataPoint(string string_line, bool EncryptedOne)
{
    //string string_line = "ashard";
    int count = string_line.size();
    int i;
    char str[100];
    char enc[100];
    char dec[100];

    for (auto i = 0; i < string_line.size(); i++)
    {
        str[i] = string_line[i];
    }

    for (i = 0; (i < 100 && str[i] != '\0'); i++)
        enc[i] = str[i] + 2; //the key for encryption is 3 that is added to ASCII value

    //cout << "\nEncrypted string: " << str << endl;

    for (i = 0; (i < 100 && str[i] != '\0'); i++)
        dec[i] = enc[i] - 2; //the key for encryption is 3 that is subtracted to ASCII value

    //cout << "\nDecrypted string: " << str << endl;

    string Encrypted = ConvertToString(enc, count);
    string Decrypted = ConvertToString(dec, count);
    return EncryptedOne == true ? Encrypted : Decrypted;
}

string ConvertToString(char* a, int size)
{
    int i;
    string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

bool containsOnlyLetters(std::string const& str) {
    return str.find_first_not_of(" abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") ==
        std::string::npos;
}

#pragma region IO

void ReadData()
{
    string myText;
    ifstream MyReadFile(Input);
    int count = 0;
    while (getline(MyReadFile, myText)) {
        SelectedDataStructure data;
        data.StringProperty = myText;
        data.LinePosition = count;
        selectedDataList.push_back(data);
        count++;
    }
    MyReadFile.close();
}

void WriteData()
{
    ofstream myfile;
    myfile.open(Output);
    for (size_t i = 0; i < 97; i++)
        myfile << "-";
    myfile << "\n|" << setw(35) << "Encrypted Text" << " |" << setw(35) << "Decrypted Text" << " |" << setw(20) << "Position" << " |" << endl;
    for (size_t i = 0; i < 97; i++)
        myfile << "-";
    for (SelectedDataStructureWithComputedValue item : selectedResultList)
    {
        myfile << "\n|" << setw(35) << item.EncryptedText << " |" << setw(35) << item.DecryptedText << " |" << setw(20) << item.LinePosition << " |";
    }
    myfile << "\n";
    for (size_t i = 0; i < 97; i++)
        myfile << "-";
    myfile.close();
}

#pragma endregion