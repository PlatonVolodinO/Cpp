#include <iostream>
#include <stdlib.h>
#include <vector>
#include <stack>
#include <algorithm>
#include <fstream>
#include <string>


using namespace std;
ifstream file;

// Таблица служебных слов
// boolean нужен для проверки условий в while и if
const char *TW[] = {
    "",
    "program",
    "undefined",
    "boolean", "int",
    "real", "else", "false",
    "if", "number",
    "return", "string",
    "true",
    "while", "write", "read",
    "and", "or", "not",
    "continue", "do",
    NULL};

// Таблица ограничителей
const char *TD[] = {
    ";", ",", ":", ".", "(",
    ")", "{", "}",
    "=", "==", "<", ">", "+",
    "-", "*", "/",
    "<=", "!=", ">=",
    NULL};

enum LexType
{
    // TW 0-18
    LEX_NULL,
    LEX_PROGRAM,
    LEX_UNDEFINED,
    LEX_BOOL,
    LEX_INT,
    LEX_REAL,
    LEX_ELSE,
    LEX_FALSE,
    LEX_IF,
    LEX_NUMBER,
    LEX_RETURN,
    LEX_STRING,
    LEX_TRUE,
    LEX_WHILE,
    LEX_WRITE,
    LEX_READ,
    LEX_AND,
    LEX_OR,
    LEX_NOT,
    LEX_CONTINUE,
    LEX_DO,
    // TD 19-38
    LEX_SEMICOLON,
    LEX_COMMA,
    LEX_COLON,
    LEX_DOT,
    LEX_LPAREN,
    LEX_RPAREN,
    LEX_BEGIN,
    LEX_END,
    LEX_EQ,
    LEX_DEQ,
    LEX_LSS,
    LEX_GTR,
    LEX_PLUS,
    LEX_MINUS,
    LEX_TIMES,
    LEX_SLASH,
    LEX_LEQ,
    LEX_NEQ,
    LEX_GEQ,
    // 39-42
    LEX_ID,
    LEX_MARK,
    LEX_NUMB,
    LEX_FLOAT_NUMB,
    LEX_STR_CONST,

    // Для генерации ПОЛИЗА
    POLIZ_GO,
    POLIZ_FGO,
    POLIZ_LABEL,
    POLIZ_ADDRESS
};

// Состояния, использующиеся при считывании лексемы из файла
enum state
{
    H,
    IDENT,
    NUMB,
    COM,
    HELPCOM,
    SLSH,
    MUL_PER,
    DOUBLE_OP1,
    DOUBLE_OP2,
    PLUS,
    MINUS,
    QUOTE
};

//==========================================================================================
// Классы Ident, Scanner и Lex - лексический анализ
//==========================================================================================

// Описание Идентификатора
// Содержит в себе его имя, тип, значение (индекс в TID), был ли объявлен идентификатор
// И соответствующие функции - setter'ы и getter'ы
// А также перегрузку оператора сравнения
class Ident
{
    string id_name;
    LexType id_type;

    int id_value;
    string id_str_value;
    double id_r_value;

    bool declare;

public:
    Ident(string n) : id_name(n), declare(false) {}
    bool operator==(const string &s) const { return id_name == s; }

    LexType GetType() const { return id_type; }

    int GetValue() const { return id_value; }
    string GetStrValue() const { return id_str_value; }
    double GetRValue() const { return id_r_value; }

    string GetName() const { return id_name; }
    bool GetDeclare() const { return declare; }

    void SetType(LexType t) { id_type = t; }

    void SetStrValue(string s) { id_str_value.assign(s); }
    void SetValue(int v) { id_value = v; }
    void SetRValue(double v) { id_r_value = v; }

    void SetName(string str) { id_name = str; }
    void SetDeclare() { declare = true; }
};

// Таблица идентификаторов анализируемой программы
vector<Ident> TID;

// Добавляет идентификатор в TID
// Возвращает его индекс
int addtoTID(const string &str)
{
    vector<Ident>::iterator i;
    i = find(TID.begin(), TID.end(), str);
    if (i != TID.end())
    {
        return (i - TID.begin());
    }
    else
    {
        TID.push_back(Ident(str));
        return (TID.size() - 1);
    }
}

// См. внутри класса
// Также есть setter'ы и getter'ы, необходимые для инкапсуляции
class Lex
{
    // Тип лексемы, см. enum LexType
    LexType l_type;
    // Либо номер в TID, либо номер в TW, либо в TD
    // Или значение для типа INT
    // В случае STRING | REAL - 0
    int l_value;
    // Значение, если REAL (FLOAT_NUMB)
    double l_real;
    // Значение, если STRING
    string l_str;

public:
    Lex(LexType t = LEX_NULL, int v = 0, string str = "", double real = 0) : l_type(t), l_value(v), l_str(str), l_real(real) {}

    LexType GetType() { return l_type; }
    int GetValue() { return l_value; }
    string GetStr() const { return l_str; }
    double GetReal() const { return l_real; }

    void SetType(LexType t) { l_type = t; }
    void SetValue(int v) { l_value = v; }
    void SetString(string s) { l_str = s; }

    friend ostream &operator<<(ostream &out, Lex l);
};

// Перегрузка оператора вывода
// Для понимания того, что происходит при анализе
ostream &operator<<(ostream &out, Lex l)
{
    string type, type_of_table;
    // Служебное слово
    if (l.l_type <= LEX_DO)
    {
        type = (string)TW[l.l_type];
        type_of_table = "TW: ";
    }
    // Ограничитель
    else if ((l.l_type <= LEX_GEQ) && (l.l_type >= LEX_SEMICOLON))
    {
        type = (string)TD[l.l_type - LEX_SEMICOLON];
        type_of_table = "TD: ";
    }
    // Числовое выражение
    else if (l.l_type == LEX_NUMB)
    {
        type = "NUM";
        type_of_table = "";
    }
    else if (l.l_type == LEX_FLOAT_NUMB)
    {
        type = "REAL";
        type_of_table = "";
    }
    // Идентификатор
    if (l.l_type == LEX_ID)
    {
        type = TID[l.l_value].GetName();
        out << " < " << type << " | "
            << "TID: " << l.l_value << " > "
            << "\n";
    }
    // Строковое выражение
    else if (l.l_type == LEX_STR_CONST)
    {
        type = "STR";
        out << " < " << type << " | " << type_of_table << l.l_str << " > "
            << "\n";
    }
    else if (l.l_type == LEX_FLOAT_NUMB) 
    {
        out << " < " << type << " | " << type_of_table << l.l_real << " > "
            << "\n";
    }
    else
    {
        out << " < " << type << " | " << type_of_table << l.l_value << " > "
            << "\n";
    }
    return out;
}

class Scanner
{
    char c;
    char gc()
    {
        cin.read(&c, 1);
        return (c);
    }
    int find(string s, const char **table)
    {
        int i = 0;
        while (table[i] != NULL)
        {
            if (s == table[i])
                return (i);
            i++;
        }
        return (0);
    }

public:
    // Продолжаем ли считывание файла
    static bool flag;
    Scanner() {}
    // Конструктор от имени файла, закидывает его содержимое в буффер std::cin
    Scanner(const char *name)
    {
        file.open(name);
        if (!file)
        {
            cout << "File not found\n";
            exit(1);
        }
        cin.rdbuf(file.rdbuf());
    }

    // Считывание лексемы
    Lex GetLex()
    {
        // Для работы с унарными операциями
        bool is_unary_minus = false;
        // Для типа REAL
        bool is_float = false;
        double f_dig;
        int f_dig_n = 10;

        // Все остальное
        int dig, j;
        state CS = H;
        string str;
        do
        {
            if (flag)
                gc();
            else
                flag = true;
            switch (CS)
            {
            case H:
                if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
                    ;
                else if (isalpha(c))
                {
                    str.push_back(c);
                    CS = IDENT;
                }
                else if (isdigit(c))
                {
                    dig = c - '0';
                    CS = NUMB;
                }
                else if (c == '+')
                {
                    str.push_back(c);
                    CS = PLUS;
                }
                else if (c == '-')
                {
                    str.push_back(c);
                    CS = MINUS;
                }
                else if (c == '/')
                {
                    str.push_back(c);
                    CS = SLSH;
                }
                else if (c == '*')
                {
                    str.push_back(c);
                    CS = MUL_PER;
                }
                else if (c == '!' || c == '=')
                {
                    str.push_back(c);
                    CS = DOUBLE_OP1;
                }
                else if (c == '<' || c == '>')
                {
                    str.push_back(c);
                    CS = DOUBLE_OP2;
                }
                else if (c == '"')
                {
                    CS = QUOTE;
                }
                else
                {
                    str.push_back(c);
                    j = find(str, TD);
                    return (Lex((LexType)(j + (int)LEX_SEMICOLON), j));
                }
                break;
            // Идентификатор
            case IDENT:
                if (isalpha(c) || isdigit(c))
                {
                    str.push_back(c);
                }
                else
                {
                    flag = false;
                    if ((j = find(str, TW)))
                    {
                        return Lex((LexType)j, j);
                    }
                    else
                    {
                        j = addtoTID(str);
                        return Lex(LEX_ID, j);
                    }
                }
                break;
            // REAL | INT
            case NUMB:
                if (isdigit(c))
                {
                    if (is_float)
                    {
                        f_dig += static_cast<double>(c - '0') / f_dig_n;
                        f_dig_n *= 10;
                    }
                    else
                    {
                        dig = 10 * dig + (c - '0');
                    }
                }
                else if (isalpha(c))
                {
                    throw c;
                }
                else if (c == '.')
                {
                    f_dig = dig;
                    is_float = true;
                }
                else
                {
                    flag = false;
                    if (is_float)
                    {
                        return Lex(LEX_FLOAT_NUMB, 0, "", is_unary_minus ? -f_dig : f_dig);
                    }
                    return Lex(LEX_NUMB, is_unary_minus ? -dig : dig);
                }
                break;
            case PLUS:
            case MINUS:
                if (isalpha(c))
                {
                    is_unary_minus = true;
                    str.pop_back();
                    str.push_back(c);
                    CS = IDENT;
                    continue;
                }
                if (isdigit(c))
                {
                    is_unary_minus = true;
                    str.pop_back();
                    str.push_back(c);
                    CS = NUMB;
                    continue;
                }
                flag = false;
                j = find(str, TD);
                return Lex((LexType)(j + (int)LEX_SEMICOLON), j);

                break;
            // Обработка *
            case MUL_PER:
                j = find(str, TD);
                return Lex((LexType)(j + (int)LEX_SEMICOLON), j);
                break;
            // Считывание LEX_STR_CONST
            case QUOTE:
                if (c == '"')
                {
                    string quoted_str = "";
                    quoted_str += str;
                    return Lex(LEX_STR_CONST, 0, quoted_str);
                }
                str.push_back(c);
                break;
            // /
            case SLSH:
                // Если комментарий: /*
                if (c == '*')
                {
                    str.pop_back();
                    CS = COM;
                }
                else
                {
                    flag = false;
                    j = find(str, TD);
                    return Lex((LexType)(j + (int)LEX_SEMICOLON), j);
                }
                break;
            // Обработка комментария, не выйдем, пока не встретим *
            case COM:
                if (c == '*')
                    CS = HELPCOM;
                break;
            // Конечное состояние обработки комментария, ждем появления /
            case HELPCOM:
                if (c == '/')
                    CS = H;
                else
                    CS = COM;
                break;
            // Обработка == | !=
            case DOUBLE_OP1:
                // Если == | !=
                if (c == '=')
                {
                    str.push_back(c);
                    j = find(str, TD);
                    return Lex((LexType)(j + (int)LEX_SEMICOLON), j);
                }
                // Если просто =
                else
                {
                    flag = false;
                    j = find(str, TD);
                    return Lex((LexType)(j + (int)LEX_SEMICOLON), j);
                }
                break;
            // Обработка <= | >=
            case DOUBLE_OP2:
                // Если <= | >=
                if (c == '=')
                {
                    str.push_back(c);
                    j = find(str, TD);
                    return Lex((LexType)(j + (int)LEX_SEMICOLON), j);
                }
                // Если просто =
                else
                {
                    flag = false;
                    j = find(str, TD);
                    return Lex((LexType)(j + (int)LEX_SEMICOLON), j);
                }
                break;
            }
        } while (1);
    }
};

//==========================================================================================
// Класс Poliz
//==========================================================================================

class Poliz
{
    Lex *p;
    int size;
    int free;

public:
    Poliz(int max_size)
    {
        p = new Lex[size = max_size];
        free = 0;
    };

    void put_lex(Lex l)
    {
        p[free] = l;
        ++free;
    };

    void put_lex(Lex l, int place) { p[place] = l; };
    void blank() { ++free; };

    int get_free() { return free; };

    Lex &operator[](int index)
    {
        if (index > size)
            throw "POLIZ:out of array";
        else if (index > free)
            throw "POLIZ:indefinite element of array";
        else
            return p[index];
    };

    void print()
    {
        for (int i = 0; i < free; ++i)
            cout << p[i];
    };

    ~Poliz() { delete[] p; };
};

//==========================================================================================
// Класс Parser - синтаксический и семантический анализ
//==========================================================================================

// Шаблонная функция работы со стеком
template <class T, class T_EL>
void from_st(T &st, T_EL &i)
{
    i = st.top();
    st.pop();
}

class Parser
{
    LexType def_type;
    // Проверка для оператора continue;
    static bool is_while_context;
    int loop_start_index;

    // Характеристики последней считанной лексемы
    Lex curr_lex;
    LexType c_type;
    int c_val;
    double c_real;
    string c_str;
    Scanner scan;

    // Стек значений и лексем
    stack<int> st_int;
    stack<LexType> st_lex;

    // начальное состояние
    void S();
    // работа с содержимым между LEX_BEGIN и LEX_END
    void B();
    // работа с описанием переменных
    void D();
    // вызывает рекурсивно T() и F()
    void E();
    void T();
    void F();
    // работа с read
    void RD();
    // работа с write
    void WR();

    // проверка переменной на повторное описание
    void dec(LexType type, int i);
    // проверка переменной на факт описания в программе
    void check_id();
    // проверка оператора ввода
    void check_id_in_read();
    // проверка соответствия типов операндов для двуместной операции
    void check_op();
    // проверка not
    void check_not();
    // проверка на равенство типов при операторе присваивания
    void eq_type(LexType &);
    // проверка на то, что в условном операторе и операторе цикла находится логическое выражение
    void eq_bool(LexType &);

    void gl()
    {
        curr_lex = scan.GetLex();
        c_type = curr_lex.GetType();
        c_val = curr_lex.GetValue();
        c_str = curr_lex.GetStr();
        c_real = curr_lex.GetReal();
        cout << curr_lex;
    }

public:
    Poliz prog;
    Parser() : scan(), prog(1000) {}
    Parser(const char *name) : scan(name), prog(1000) {}
    void analyze();
};

bool Parser::is_while_context = false;

// Обертка над S() для вывода о конце анализа
void Parser::analyze()
{
    gl();
    S();
    cout << endl
         << "Your program is syntactically and semantically correct!" << endl;
}

// Базовое состояние парсера, считывает лексемы внутри {} и лексему program
void Parser::S()
{
    if (c_type == LEX_PROGRAM)
    {
        gl();
        B();
    }
    else if (c_type == LEX_IF)
    {
        loop_start_index = prog.get_free();
        LexType new_val;
        gl();
        if (c_type != LEX_LPAREN)
        {
            throw curr_lex;
        }
        else
        {
            gl();
            E();
            eq_bool(new_val);

            // после условия
            int fgoIndex = prog.get_free();
            prog.blank();
            prog.put_lex(Lex(POLIZ_FGO));

            if (c_type == LEX_RPAREN)
            {
                gl();
                B();
                int goIndex = prog.get_free();
                prog.blank();
                prog.put_lex(Lex(POLIZ_GO));
                prog.put_lex(Lex(POLIZ_LABEL, prog.get_free()), fgoIndex);

                if (c_type == LEX_ELSE)
                {
                    gl();
                    B();
                }
                prog.put_lex(Lex(POLIZ_LABEL, prog.get_free()), goIndex);
            }
            else
            {
                throw curr_lex;
            }
        }
    }
    else if (c_type == LEX_WHILE)
    {
        loop_start_index = prog.get_free();
        int fgoIndex;
        int startIndex = prog.get_free();

        LexType new_val;
        gl();
        if (c_type != LEX_LPAREN)
        {
            throw curr_lex;
        }
        else
        {
            gl();
            E();
            eq_bool(new_val);

            fgoIndex = prog.get_free();
            prog.blank();
            prog.put_lex(Lex(POLIZ_FGO));

            if (c_type == LEX_RPAREN)
            {
                gl();
                is_while_context = true;
                B();
                is_while_context = false;
            }
            else
                throw curr_lex;
        }
        prog.put_lex(Lex(POLIZ_LABEL, startIndex));
        prog.put_lex(Lex(POLIZ_GO));
        prog.put_lex(Lex(POLIZ_LABEL, prog.get_free()), fgoIndex);
    }
    else if (c_type == LEX_DO)
    {
        loop_start_index = prog.get_free();
        LexType new_val;
        gl();

        is_while_context = true;
        B();
        is_while_context = false;

        if (c_type != LEX_WHILE)
        {
            throw curr_lex;
        }
        gl();
        if (c_type != LEX_LPAREN)
        {
            throw curr_lex;
        }
        gl();

        E();
        eq_bool(new_val);

        if (c_type != LEX_RPAREN)
        {
            throw curr_lex;
        }
        gl();
        if (c_type != LEX_SEMICOLON)
        {
            throw curr_lex;
        }

        prog.put_lex(Lex(POLIZ_LABEL, prog.get_free() + 4));
        prog.put_lex(Lex(POLIZ_FGO));
        prog.put_lex(Lex(POLIZ_LABEL, loop_start_index));
        prog.put_lex(Lex(POLIZ_GO));

        gl();
    }
    else if (c_type == LEX_ID)
    {
        int l_v_index = curr_lex.GetValue();
        prog.put_lex(Lex(POLIZ_ADDRESS, l_v_index));

        LexType new_val;
        check_id();
        gl();
        if (c_type == LEX_EQ)
        {
            gl();
            E();
            eq_type(new_val);
            prog.put_lex(Lex(LEX_EQ));
            TID[l_v_index].SetType(new_val);
            if (c_type == LEX_SEMICOLON)
            {
                gl();
            }
            else if (cin.eof())
            {
                return;
            }
            else
            {
                throw curr_lex;
            }
        }
        else if (c_type == LEX_COLON)
        {
            gl();
        }
        else
        {
            throw curr_lex;
        }
    }
    else if (c_type == LEX_INT || c_type == LEX_STRING || c_type == LEX_REAL)
    {
        if (c_type == LEX_INT)
            def_type = LEX_NUMB;
        else if (c_type == LEX_STRING)
            def_type = LEX_STR_CONST;
        else
            def_type = LEX_FLOAT_NUMB;
        gl();
        D();
    }
    else if (c_type == LEX_NUMB || c_type == LEX_STR_CONST || c_type == LEX_FLOAT_NUMB)
    {
        st_lex.push(c_type);
        prog.put_lex(Lex(c_type, c_val, c_str, c_real));
        E();
        gl();
    }
    else if (cin.eof())
    {
        return;
    }
    else if (c_type == LEX_END)
    {
        return;
    }
    else if (c_type == LEX_READ)
    {
        RD();
        check_id_in_read();
        gl();
    }
    else if (c_type == LEX_WRITE)
    {
        WR();
        gl();
    }
    else if (c_type == LEX_CONTINUE)
    {
        prog.put_lex(Lex(POLIZ_LABEL, loop_start_index));
        prog.put_lex(Lex(POLIZ_GO));

        if (!is_while_context)
            throw curr_lex;
        gl();
        if (c_type != LEX_SEMICOLON)
            throw curr_lex;
        gl();
    }
    S();
}

// Состояние для контроля парности фигурных скобок
void Parser::B()
{
    if (c_type == LEX_BEGIN)
    {
        gl();
        S();
        if (c_type == LEX_END)
        {
            gl();
        }
        else
        {
            throw curr_lex;
        }
    }
    else
    {
        throw curr_lex;
    }
}

// Работа с определением переменных
void Parser::D()
{
    if (c_type != LEX_ID)
    {
        throw curr_lex;
    }
    else
    {
        st_int.push(c_val);
        int l_v_index = c_val;
        gl();
        if (c_type == LEX_EQ)
        {
            prog.put_lex(Lex(POLIZ_ADDRESS, l_v_index));
            LexType i;
            gl();
            E();
            prog.put_lex(Lex(LEX_EQ));
            from_st(st_lex, i);
            if (i != def_type)
                throw "Wrong type: waited: " + to_string(def_type) + ", got: " + to_string(i);
            dec(i, l_v_index);
        }
        else
        {
            dec(def_type, l_v_index);
        }

        while (c_type == LEX_COMMA)
        {
            gl();
            if (c_type != LEX_ID)
            {
                throw curr_lex;
            }
            else
            {
                st_int.push(c_val);
                int l_v_index = c_val;
                gl();
                if (c_type == LEX_EQ)
                {
                    prog.put_lex(Lex(POLIZ_ADDRESS, l_v_index));
                    LexType i;
                    gl();
                    E();
                    prog.put_lex(Lex(LEX_EQ));
                    from_st(st_lex, i);
                    if (i != def_type)
                        throw "Wrong type: waited: " + to_string(def_type) + ", got: " + to_string(i);
                    dec(i, l_v_index);
                }
                else
                {
                    dec(def_type, l_v_index);
                }
            }
        }
        if (c_type != LEX_SEMICOLON && (!cin.eof()))
        {
            throw curr_lex;
        }
        else if (c_type == LEX_SEMICOLON)
        {
            gl();
        }
    }
}

// Первое из трех состояний для разбора выражение (E, T и F)
// Работает с OR AND NOT и = между выражениями
// Т.е. (выражение: обработалось в T и F) AND (выражение: обработалось в T и F)
void Parser::E()
{
    T();
    while (c_type == LEX_OR || c_type == LEX_AND || c_type == LEX_NOT || c_type == LEX_EQ)
    {
        st_lex.push(c_type);
        gl();
        T();
        check_op();
    }
}

// Работает с + - / и тд.
// Сами операнды обрабатывает F
void Parser::T()
{
    F();
    while (c_type == LEX_PLUS || c_type == LEX_MINUS || c_type == LEX_TIMES || c_type == LEX_SLASH ||
           c_type == LEX_EQ || c_type == LEX_LSS || c_type == LEX_GTR || c_type == LEX_LEQ ||
           c_type == LEX_GEQ || c_type == LEX_NEQ || c_type == LEX_DEQ)
    {
        st_lex.push(c_type);
        gl();
        F();
        check_op();
    }
}

// Собственно, обрабатывает операнды
void Parser::F()
{
    if (c_type == LEX_ID)
    {
        prog.put_lex(Lex(LEX_ID, c_val));
        check_id();
        gl();
    }
    else if (c_type == LEX_NUMB || c_type == LEX_STR_CONST || c_type == LEX_FLOAT_NUMB)
    {
        st_lex.push(c_type);
        prog.put_lex(Lex(c_type, c_val, c_str, c_real));
        gl();
    }
    else if (c_type == LEX_NOT)
    {
        gl();
        F();
        check_not();
    }
    else if (c_type == LEX_LPAREN)
    {
        gl();
        E();
        if (c_type == LEX_RPAREN)
        {
            gl();
        }
        else
        {
            throw curr_lex;
        }
    }
    else
    {
        cout << c_type;
        throw curr_lex;
    }
}

// Работа со служебным словом READ (выделено т.к. необходимо обработать круглые скобки)
void Parser::RD()
{
    gl();
    if (c_type != LEX_LPAREN)
    {
        throw curr_lex;
    }
    else
    {
        gl();
        E();
        if (c_type != LEX_RPAREN)
        {
            throw curr_lex;
        }
        else
        {
            gl();
        }
    }
    prog.put_lex(Lex(LEX_READ));
}

// Работа со служебным словом WRITE (выделено т.к. необходимо обработать круглые скобки)
void Parser::WR()
{
    gl();
    if (c_type != LEX_LPAREN)
    {
        throw curr_lex;
    }
    else
    {
        gl();
        E();
        while (c_type == LEX_COMMA)
        {
            gl();
            E();
        }
        if (c_type != LEX_RPAREN)
        {
            throw curr_lex;
        }
        else
        {
            gl();
        }
    }
    prog.put_lex(Lex(LEX_WRITE));
}

// Вспомогательная функция для Parser:D(), проверяющая двойное объявление
void Parser::dec(LexType type, int i)
{
    if (TID[i].GetDeclare())
    {
        throw "twice";
    }
    else
    {
        TID[i].SetDeclare();
        TID[i].SetType(type);
    }
}
// Вспомогательная функция для E, проверяющая корректность идентификатора (был ли объявлен)
void Parser::check_id()
{
    if (TID[c_val].GetDeclare())
    {
        st_lex.push(TID[c_val].GetType());
    }
    else
    {
        throw "not declared";
    }
}

// Вспомогательная функция для E, проверяющая корректность идентификатора (был ли объявлен)
// Выделена для read отдельно, т.к. в стек лексем ничего записывать не надо
void Parser::check_id_in_read()
{
    if (!TID[c_val].GetDeclare())
        throw "not declared";
}

// Проверка корректности бинарных операций
// По типу
void Parser::check_op()
{
    // EDITED!
    LexType t1, t2, op;

    from_st(st_lex, t2);
    from_st(st_lex, op);
    from_st(st_lex, t1);

    const char ERROR_MSG[] = "wrong types in last operation";

    if (t1 != t2)
        throw ERROR_MSG;

    if (op == LEX_EQ)
    {
        st_lex.push(t1);
    }
    else if (t1 != LEX_BOOL && (op == LEX_LSS || op == LEX_GTR || op == LEX_LEQ || op == LEX_GEQ))
    {
        st_lex.push(LEX_BOOL);
    }
    else if (op == LEX_DEQ || op == LEX_NEQ)
    {
        st_lex.push(LEX_BOOL);
    }
    else if (t1 == LEX_BOOL && (op == LEX_OR || op == LEX_AND))
    {
        st_lex.push(LEX_BOOL);
    }
    else if ((t1 == LEX_NUMB || t1 == LEX_FLOAT_NUMB) && (op == LEX_PLUS || op == LEX_MINUS || op == LEX_TIMES || op == LEX_SLASH))
    {
        st_lex.push(t1);
    }
    else if (t1 == LEX_STR_CONST && op == LEX_PLUS)
    {
        st_lex.push(LEX_STR_CONST);
    }
    else
    {
        throw ERROR_MSG;
    }

    prog.put_lex(Lex(op));
}

// Проверка корректности для not (ждем тип BOOL)
void Parser::check_not()
{
    LexType t;
    from_st(st_lex, t);
    if (t == LEX_BOOL)
    {
        st_lex.push(LEX_BOOL);
    }
    else
        throw "wrong type is in not";

    prog.put_lex(Lex(LEX_NOT));
}

// Если встретили справа от = объявленный, но не инициализированный операнд => ошибка
void Parser::eq_type(LexType &new_val)
{
    from_st(st_lex, new_val);
    if (new_val == LEX_UNDEFINED)
    {
        throw "wrong types in '=' operation";
    }
}

// Проверка на BOOl для IF & WHILE & DO {} WHILE();
void Parser::eq_bool(LexType &new_val)
{
    from_st(st_lex, new_val);
    if (new_val != LEX_BOOL)
        throw "expression is not boolean";
}

//==========================================================================================
// Класс Translator - трансляция
//==========================================================================================

class Translator
{
    Parser p;

public:
    Translator() : p() {}
    Translator(const char *name) : p(name){};
    void Translate()
    {
        p.analyze();

        cout << "Syntax analysator ends!\n"
             << endl;

        printPolize();

        cout << endl
             << "POLIZ END!" << endl;

        cout << endl
             << "PROGRAMM OUTPUT:" << endl;

        execute();

        cout << endl
             << "PROGRAMM EXECUTED CORRECTLY" << endl;
    }

    void printPolize()
    {
        cout << endl
             << "POLIZ:" << endl;
        for (int i = 0; i < p.prog.get_free(); ++i)
        {
            int l_type = p.prog[i].GetType();
            if (l_type == POLIZ_ADDRESS)
            {
                cout << i << ":\t <ident: " << TID[p.prog[i].GetValue()].GetName() << '>' << endl;
            }
            else if (l_type == POLIZ_LABEL)
            {
                cout << i << ":\t <label to: " << p.prog[i].GetValue() << '>' << endl;
            }
            else if (l_type == POLIZ_GO)
            {
                cout << i << ":\t <GO>" << endl;
            }
            else if (l_type == POLIZ_FGO)
            {
                cout << i << ":\t <FGO>" << endl;
            }
            else if (l_type == LEX_FLOAT_NUMB)
            {
                cout << i << ":\t <REAL | " << p.prog[i].GetReal() << " >" << endl;
            }
            else if (l_type == LEX_STR_CONST)
            {

                cout << i << ":\t <STR | '" << p.prog[i].GetStr() << "' >" << endl;
            }
            else if (l_type == LEX_NUMB)
            {

                cout << i << ":\t <INT | " << p.prog[i].GetValue() << " >" << endl;
            }
            else
            {
                cout << i << ":\t" << p.prog[i];
            }
        }
    }

    void execute()
    {
        Lex curr_el;
        stack<Lex> args;

        int index{}, size = p.prog.get_free();
        Lex first_op, second_op;

        auto getValueViaAddr = [](Lex &operand)
        {
            Ident id = TID[operand.GetValue()];
            switch (id.GetType())
            {
            case LEX_NUMB:
                return Lex(LEX_NUMB, id.GetValue());
            case LEX_FLOAT_NUMB:
                return Lex(LEX_FLOAT_NUMB, 0, "", id.GetRValue());
            default:
                return Lex(LEX_STR_CONST, 0, id.GetStrValue());
            }
        };

        auto setSingleOperand = [&first_op, &args, &getValueViaAddr]()
        {
            first_op = args.top();
            args.pop();

            if (first_op.GetType() == POLIZ_ADDRESS)
                first_op = getValueViaAddr(first_op);
        };

        auto setTwoOperands = [&first_op, &second_op, &args, &getValueViaAddr]()
        {
            first_op = args.top();
            args.pop();
            second_op = args.top();
            args.pop();

            if (first_op.GetType() == POLIZ_ADDRESS)
                first_op = getValueViaAddr(first_op);
            if (second_op.GetType() == POLIZ_ADDRESS)
                second_op = getValueViaAddr(second_op);
        };

        while (index < size)
        {
            curr_el = p.prog[index];
            switch (curr_el.GetType())
            {
            case LEX_NUMB:
            case LEX_REAL:
            case LEX_STR_CONST:
            case POLIZ_ADDRESS:
            case POLIZ_LABEL:
                args.push(curr_el);
                break;
            case LEX_ID:
                args.push(Lex(POLIZ_ADDRESS, curr_el.GetValue()));
                break;
            case LEX_NOT:
                setSingleOperand();
                args.push(Lex(LEX_BOOL, !first_op.GetValue()));
                break;
            case LEX_OR:
                setTwoOperands();
                args.push(Lex(LEX_BOOL, first_op.GetValue() || second_op.GetValue()));
                break;
            case LEX_AND:
                setTwoOperands();
                args.push(Lex(LEX_BOOL, first_op.GetValue() && second_op.GetValue()));
                break;
            case POLIZ_GO:
                setSingleOperand();
                index = first_op.GetValue() - 1;
                break;
            case POLIZ_FGO:
                setTwoOperands();
                if (!second_op.GetValue())
                    index = first_op.GetValue() - 1;
                break;
            case LEX_WRITE:
                setSingleOperand();
                switch (first_op.GetType())
                {
                case LEX_NUMB:
                    cout << first_op.GetValue() << endl;
                    break;
                case LEX_STR_CONST:
                    cout << first_op.GetStr() << endl;
                    break;
                default:
                    cout << first_op.GetReal() << endl;
                    break;
                }
                break;
            case LEX_READ:
                first_op = args.top();
                args.pop();

                if (first_op.GetType() == LEX_NUMB)
                {
                    int value;
                    cout << "Input int value for " << TID[first_op.GetValue()].GetName() << endl;
                    cin >> value;
                    TID[first_op.GetValue()].SetValue(value);
                }
                if (first_op.GetType() == LEX_FLOAT_NUMB)
                {
                    double value;
                    cout << "Input real value for " << TID[first_op.GetValue()].GetName() << endl;
                    cin >> value;
                    TID[first_op.GetValue()].SetRValue(value);
                }
                if (first_op.GetType() == LEX_STR_CONST)
                {
                    string value;
                    cout << "Input string value for " << TID[first_op.GetValue()].GetName() << endl;
                    cin >> value;
                    TID[first_op.GetValue()].SetStrValue(value);
                }
                break;
            case LEX_PLUS:
                setTwoOperands();
                if (first_op.GetType() == LEX_NUMB)
                {
                    args.push(Lex(LEX_NUMB, first_op.GetValue() + second_op.GetValue()));
                }
                if (first_op.GetType() == LEX_FLOAT_NUMB)
                {
                    args.push(Lex(LEX_FLOAT_NUMB, 0, "", first_op.GetReal() + second_op.GetReal()));
                }
                if (first_op.GetType() == LEX_STR_CONST)
                {
                    args.push(Lex(LEX_STR_CONST, 0, second_op.GetStr() + first_op.GetStr()));
                }
                break;
            case LEX_MINUS:
                setTwoOperands();

                if (first_op.GetType() == LEX_NUMB)
                {
                    args.push(Lex(LEX_NUMB, -first_op.GetValue() + second_op.GetValue()));
                }
                if (first_op.GetType() == LEX_FLOAT_NUMB)
                {
                    args.push(Lex(LEX_FLOAT_NUMB, 0, "", -first_op.GetReal() + second_op.GetReal()));
                }
                break;
            case LEX_TIMES:
                setTwoOperands();

                if (first_op.GetType() == LEX_NUMB)
                {
                    args.push(Lex(LEX_NUMB, first_op.GetValue() * second_op.GetValue()));
                }
                if (first_op.GetType() == LEX_FLOAT_NUMB)
                {
                    args.push(Lex(LEX_FLOAT_NUMB, 0, "", first_op.GetReal() * second_op.GetReal()));
                }
                break;
            case LEX_SLASH:
                setTwoOperands();

                if (first_op.GetType() == LEX_NUMB)
                {
                    args.push(Lex(LEX_NUMB, second_op.GetValue() / first_op.GetValue()));
                }
                if (first_op.GetType() == LEX_FLOAT_NUMB)
                {
                    args.push(Lex(LEX_FLOAT_NUMB, 0, "", second_op.GetReal() / first_op.GetReal()));
                }
                break;
            case LEX_LSS:
                setTwoOperands();
                if (second_op.GetType() == LEX_NUMB)
                    args.push(Lex(LEX_BOOL, second_op.GetValue() < first_op.GetValue()));
                else if (second_op.GetType() == LEX_FLOAT_NUMB)
                    args.push(Lex(LEX_BOOL, second_op.GetReal() < first_op.GetReal()));
                else 
                    args.push(Lex(LEX_BOOL, second_op.GetStr() < first_op.GetStr()));
                break;
            case LEX_GTR:
                setTwoOperands();
                if (second_op.GetType() == LEX_NUMB)
                    args.push(Lex(LEX_BOOL, second_op.GetValue() > first_op.GetValue()));
                else if (second_op.GetType() == LEX_FLOAT_NUMB)
                    args.push(Lex(LEX_BOOL, second_op.GetReal() > first_op.GetReal()));
                else 
                    args.push(Lex(LEX_BOOL, second_op.GetStr() > first_op.GetStr()));
                break;
            case LEX_LEQ:
                setTwoOperands();
                if (second_op.GetType() == LEX_NUMB)
                    args.push(Lex(LEX_BOOL, second_op.GetValue() <= first_op.GetValue()));
                else if (second_op.GetType() == LEX_FLOAT_NUMB)
                    args.push(Lex(LEX_BOOL, second_op.GetReal() <= first_op.GetReal()));
                else 
                    args.push(Lex(LEX_BOOL, second_op.GetStr() <= first_op.GetStr()));
                break;
            case LEX_GEQ:
                setTwoOperands();
                if (second_op.GetType() == LEX_NUMB)
                    args.push(Lex(LEX_BOOL, second_op.GetValue() >= first_op.GetValue()));
                else if (second_op.GetType() == LEX_FLOAT_NUMB)
                    args.push(Lex(LEX_BOOL, second_op.GetReal() >= first_op.GetReal()));
                else 
                    args.push(Lex(LEX_BOOL, second_op.GetStr() >= first_op.GetStr()));
                break;
            case LEX_NEQ:
                setTwoOperands();
                if (second_op.GetType() == LEX_NUMB)
                    args.push(Lex(LEX_BOOL, second_op.GetValue() != first_op.GetValue()));
                else if (second_op.GetType() == LEX_FLOAT_NUMB)
                    args.push(Lex(LEX_BOOL, second_op.GetReal() != first_op.GetReal()));
                else 
                    args.push(Lex(LEX_BOOL, second_op.GetStr() != first_op.GetStr()));
                break;
            case LEX_EQ:
                setSingleOperand();
                second_op = args.top();
                args.pop();

                switch ((TID[second_op.GetValue()].GetType()))
                {
                case LEX_FLOAT_NUMB:
                    TID[second_op.GetValue()].SetRValue(first_op.GetReal());
                    break;
                case LEX_NUMB:
                    TID[second_op.GetValue()].SetValue(first_op.GetValue());
                    break;
                default:
                    TID[second_op.GetValue()].SetStrValue(first_op.GetStr());
                    break;
                }
                break;
            default:
                break;
            }

            ++index;
        }
    }
};

//==========================================================================================
bool Scanner::flag = true;

// Обработка ошибок, для корректного вывода об ошибках анализируемой программы
int main(int argc, char **argv)
{
    cout << "Syntax analysator starts: " << endl;

    Translator t;
    if (argc == 2)
        t = Translator(argv[1]);
    else
    {
        printf("Error with argc\n");
        return 1;
    }

    try
    {
        t.Translate();
    }
    catch (char c)
    {
        cout << "ERROR: wrong NUM, extra character: " << c << "\n";
        return 2;
    }
    catch (LexType type)
    {
        cout << "ERROR: wrong type: " << to_string(type) << "\n";
        return 3;
    }
    catch (Lex l)
    {
        cout << "ERROR: wrong lexem:" << l << "\n";
        return 4;
    }
    catch (const char *str)
    {
        cout << "ERROR: " << str << "\n";
        return 5;
    }
    return 0;
}