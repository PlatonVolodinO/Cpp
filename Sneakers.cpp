#include <iostream>
#include <math.h>
#include <cstring>
using namespace std;

// Задание: Продажа кроссовок (российских, европейских)
// Идея программы: абстрактный базовый класс — класс "кроссовки";
// производные классы — российские и европейские кроссовки, для которых по-разному рассчитывается оптовая цена через розничную.

class Sneakers
{
    char   *size;            // размер
    double retail_price;        // розничная цена
    double wholesale_discount;  // скидка на оптовую покупку (скидка от розничной цены)
public:

    char*  GetSneakSize()  const { return size; }
    double GetRetPrice() const { return retail_price; }
    double GetDiscount() const { return wholesale_discount; }

    void SetSneakSize(const char *ss)
    {
        delete [] size;
        size = new char [strlen(ss) + 1];
        strcpy (size, ss);
    }  
    void SetRetPrice(double rp) { retail_price = rp; }
    void SetDiscount(double wd) { wholesale_discount = wd; }

    static double import_duty;  // статическая переменная - пошлина на ввоз европейских кроссовок
    static double euro_rate;    // статическая переменная - курс евро

    Sneakers(const char *ss, double rp, double wd)
    {
        size = new char [strlen(ss) + 1];
        strcpy (size, ss);
        retail_price = rp;
        wholesale_discount = wd;
    }

    Sneakers(const Sneakers &obj)  // конструктор копирования
    {
        size = new char [strlen(obj.GetSneakSize())+1];
        strcpy(size, obj.GetSneakSize());
        retail_price = obj.GetRetPrice();  
        wholesale_discount = obj.GetDiscount();  
    }
    
    virtual ~Sneakers() { delete [] size; }  // деструктор

    virtual double RubPrice() const = 0;  // чистая виртуальная функция для расчёта цены оптовой покупки кроссовок в производных классах

};

class RusSneakers : public Sneakers
{
public:
    RusSneakers(const char *ss, double rp, double wd) : Sneakers(ss, rp, wd) {}
    RusSneakers(const RusSneakers &obj) : Sneakers(obj.GetSneakSize(), obj.GetRetPrice(), obj.GetDiscount()) {}

    RusSneakers& operator=(const RusSneakers &obj)
    {
        this->SetSneakSize(obj.GetSneakSize());
        this->SetRetPrice(obj.GetRetPrice());
        this->SetDiscount(obj.GetDiscount());

        return *this;
    }

    double RubPrice() const
    {
        return (this->GetRetPrice()*(1-this->GetDiscount()));
    }

    void operator()(double new_price)  // перегрузка операции () для изменения розничной цены существующего объекта
    {
        this->SetRetPrice(new_price);
    }

    friend ostream &operator<<(ostream &s, const RusSneakers &obj);
};

class EurSneakers : public Sneakers
{
    char *brand;
public:

    char* GetBrand() const { return brand; }
    void SetBrand(const char *br)
    {
        delete [] brand;
        brand = new char [strlen(br) + 1];
        strcpy (brand, br);
    }

    EurSneakers(const char *ss, double rp, double wd, const char *br) : Sneakers(ss, rp, wd) 
    {
        brand = new char [strlen(br) + 1];
        strcpy (brand, br);
    }

    EurSneakers(const EurSneakers &obj) : Sneakers(obj.GetSneakSize(), obj.GetRetPrice(), obj.GetDiscount())
    {
        brand = new char [strlen(obj.GetBrand())+1];
        strcpy(brand, obj.GetBrand());
    }

    ~EurSneakers() { delete [] brand; }

    EurSneakers& operator=(const EurSneakers &obj)
    {
        this->SetSneakSize(obj.GetSneakSize());
        this->SetRetPrice(obj.GetRetPrice());
        this->SetDiscount(obj.GetDiscount());
        this->SetBrand(obj.GetBrand());

        return *this;
    }

    double RubPrice() const
    {
        return (this->GetRetPrice()*Sneakers::euro_rate*(1-this->GetDiscount())*(1+Sneakers::import_duty));
    }

    void operator()(double new_price)  // перегрузка операции () для изменения розничной цены существующего объекта
    {
        this->SetRetPrice(new_price);
    }

    friend ostream &operator<<(ostream &s, const EurSneakers &obj);
};

ostream &operator<<(ostream &s, const RusSneakers &obj)
{
    double discount = obj.GetDiscount() * 100;

    s << "Russian sneakers:" << endl;
    s << "Sneakers size: " << obj.GetSneakSize() << endl;
    s << "Retail price: " << obj.GetRetPrice() << "₽" << endl;
    s << "Wholesale discount: " << discount << "%" << endl;
    s << "Final price: " << obj.RubPrice() << "₽" << endl;
    return s;
}

ostream &operator<<(ostream &s, const EurSneakers &obj)
{
    double discount = obj.GetDiscount() * 100;
    double duty = Sneakers::import_duty * 100;

    s << "European sneakers:" << endl;
    s << "Brand: " << obj.GetBrand() << endl;
    s << "Sneakers size: " << obj.GetSneakSize() << endl;
    s << "Retail price: " << obj.GetRetPrice() << "€"  << endl;
    s << "Wholesale discount: " << discount << "%" << endl;
    s << "Current euro rate: " << Sneakers::euro_rate << "₽"  << endl;
    s << "Import duty: " << duty << "%" << endl;
    s << "Final price: " << obj.RubPrice() << "₽" << endl;   
    return s;
}

double Sneakers::import_duty = 0.2;
double Sneakers::euro_rate = 80;

int main()
{
    cout << endl; 

    RusSneakers Sneakers1("RU 29", 5000, 0.1);
    cout << Sneakers1 << endl;

    EurSneakers Sneakers2("EU 45", 200, 0.1, "Adidas");
    cout << Sneakers2 << endl;

    cout << "Checking the assignment operation. Let's create a new sneakers: " << endl;
    RusSneakers Sneakers3("Ru 28", 8000, 0.12);
    cout << Sneakers3 << endl;
    cout << "Let's assign a Sneakers1 to the Sneakers3 and print an info about the Sneakers3:" << endl;
    Sneakers3 = Sneakers1;
    cout << Sneakers3;
    cout << "As we can see, now the Sneakers3 is a Sneakers1!" << endl;
    cout << endl; 

    cout << "Checking the '()' operation. Let's change the cost of the Sneakers1 by 7500₽ and print it out: " << endl;
    Sneakers1(7500);
    cout << Sneakers1 << endl;
 
    return 0;
}