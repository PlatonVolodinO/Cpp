#include <iostream>
#include <string>
using namespace std;

class Array
{
    string *arr;
    int size;
    static int Length; // длина строки

public:
    friend ostream& operator<<(ostream &output, const Array &mas);

    Array(int n, bool st){
        size = n;
        arr = new string[size+1]; 
        string s; // дополнительная переменная-строка
        char buf[Length]; // буфер для ввода строк
        cout << "Enter strings:" << endl;
        for(int i = 0; i <= size; i++){
            cin.getline(buf, Length, '\n');
            s = buf;
            arr[i] = s;
        }
    }

    Array(int n){
        size = n;
        arr = new string[size+1];
    }
 
    // конструктор копирования
    Array(const Array &Src){
        size = Src.size;
        arr = new string[size+1]; 
        for(int i = 0; i <= size; i++){
            arr[i] = Src.arr[i];
        }
    }

    // перегрузка оператора []
    string operator[](const int index) const{ 
        if (index >= size || index < 0) { 
        cout << "index not in range" << endl;
        abort();
        }
        return arr[index]; 
    }


    bool is_contain (const string str) const { // проверка наличия строки
    bool answ = false;
    for (int i = 0; i <= size; ++i) {
      answ = str == arr[i];
      if (answ) {
        break;
      }
    }
    return answ;
    }

    Array Merger(const Array& a) const{ // слияние массивов
        Array NewAr(a.size + size);
        int i, j;
        for (i = 1, j = 0; i <= size; ++i) {
        if (!NewAr.is_contain(arr[i])) {
            NewAr.arr[j] = arr[i];
            ++j;
        }
        else  NewAr.size--;
        }
        for(i = 1; i <= a.size; ++i) {
            if (!NewAr.is_contain(a.arr[i])) {
                NewAr.arr[j] = a.arr[i];
                ++j;
            }
            else  NewAr.size--;
        }
    return NewAr;
    }

    Array Concat(const Array& a) const{ // объединение массивов
        Array NewAr(a.size + size);
        int i, j = 1;
        for (i = 0; i <= size; i++){
            NewAr.arr[i] = arr[i];
        }
        for (; i <= NewAr.size; i++, j++){
            NewAr.arr[i] = a.arr[j];
        }
        return NewAr;
    }

    void PString(int k) const{
        cout << arr[k] << endl;
    }

    // деструктор
    ~Array() { 
        delete [] arr;
    }
};

// перегрузка операции вывода
ostream& operator<<(ostream &output, const Array &mas){
    output << endl;
    output << "Array contains " << mas.size << " strings \n";
    for(int i = 0; i <= mas.size; i++)
        output << mas.arr[i] << endl;
    return output;
}

int Array::Length = 8;

int main(){   
    int choice;
    cout << "You can choose one of the operations: " << endl;
    cout << "1 Enter array" << endl;
    cout << "2 Connect two arrays" << endl;
    cout << "3 Merge two arrays with no repeats" << endl;
    cout << "4 Print string by index" << endl;
    cout << "5 Print whole array" << endl;
    cout << "6 End session" << endl;
    cout << "Enter command" << endl;
    cout << "=> ";
    
    cin >> choice;
    while (choice != 6){
        switch(choice){
            case 1:{
                int sz;
                cout << "Enter size of array:" << endl;
                cin >> sz;
                Array mas(sz, true);
                cout << mas;
                break;
            }
            case 2:{
                int sz1, sz2;
                cout << "Enter size of the first array" << endl;
                cin >> sz1;
                Array mas1(sz1, true);
                cout << "Enter size of the second array" << endl;
                cin >> sz2;
                Array mas2(sz2, true);
                Array res_mas = mas1.Concat(mas2);
                cout << res_mas;
                break;
            }
            case 3:{
                int sz1, sz2;
                cout << "Enter size of the first array" << endl;
                cin >> sz1;
                Array mas1(sz1, true);
                cout << "Enter size of the second array" << endl;
                cin >> sz2;
                Array mas2(sz2, true);
                Array res_mas = mas1.Merger(mas2);
                cout << res_mas;
                break;
            }
            case 4:{
                int sz, index;
                cout << "Enter size of array:" << endl;
                cin >> sz;
                Array mas(sz, true);
                cout << "Enter index of the string:" << endl;
                cin >> index;
                mas.PString(index);
                break;
            }
            case 5:{
                int sz;
                cout << "Enter size of array:" << endl;
                cin >> sz;
                Array mas(sz, true);
                cout << mas;
                break;
            }
            case 6:{
                exit(0);
            }
            default: 
                cout << "Wrong input!" << endl;
                abort();
                exit(1);
            }
            cout << "Enter new command" << endl;

            cin >> choice;
        }
    return 0;
}