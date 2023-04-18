#include <iostream>
#include <string>
#include <cstdio>
#include <ctype.h>
#include <cstdlib>
#include <vector>
#include <stack>
#include <algorithm>
using namespace std;

enum type_of_lex
{

    LEX_NULL,

    LEX_AND,
    LEX_BEGIN,
    LEX_DO,
    LEX_ELSE,
    LEX_END,
    LEX_IF,
    LEX_FALSE,
    LEX_INT,
    LEX_REAL,
    LEX_STR,
    LEX_CONST,

    LEX_NOT,
    LEX_OR,
    LEX_PROGRAM,
    LEX_READ,
    LEX_THEN,
    LEX_TRUE,
    LEX_VAR,
    LEX_WHILE,
    LEX_WRITE,

    LEX_FIN,

    LEX_SEMICOLON,
    LEX_COMMA,
    LEX_COLON,
    LEX_QUOT,
    LEX_ASSIGN,
    LEX_LPAREN,
    LEX_RPAREN,
    LEX_EQ,
    LEX_LES,

    LEX_GRT,
    LEX_PLUS,
    LEX_MINUS,
    LEX_MUL,
    LEX_SLASH,
    LEX_LEQ,
    LEX_NEQ,
    LEX_GEQ,

    LEX_IDT,

    LEX_NUM,

};

class Lex
{

    type_of_lex t_lex;

    string v_lex;

public:
    Lex(type_of_lex t = LEX_NULL, string v = "") : t_lex(t), v_lex(v) {}

    type_of_lex get_type() const
    {

        return t_lex;
    }

    string get_value() const
    {

        return v_lex;
    }

    friend ostream &operator<<(ostream &s, Lex l);
};

class Ident
{

    string name;

    bool declare;

    type_of_lex type;

    bool assign;

    int value;

public:
    Ident()
    {

        declare = false;

        assign = false;
    }

    bool operator==(const string &s) const
    {

        return name == s;
    }

    Ident(const string n)
    {

        name = n;

        declare = false;

        assign = false;
    }

    string get_name() const
    {

        return name;
    }

    bool get_declare() const
    {

        return declare;
    }

    void put_declare()
    {

        declare = true;
    }

    type_of_lex get_type() const
    {

        return type;
    }

    void put_type(type_of_lex t)
    {

        type = t;
    }

    bool get_assign() const
    {

        return assign;
    }

    void put_assign()
    {

        assign = true;
    }

    int get_value() const
    {

        return value;
    }

    void put_value(int v)
    {

        value = v;
    }
};

vector<Ident> TID;

int put(const string &buf)
{

    vector<Ident>::iterator k;

    if ((k = find(TID.begin(), TID.end(), buf)) != TID.end())

        return k - TID.begin();

    TID.push_back(Ident(buf));

    return TID.size() - 1;
}

class Scanner
{

    FILE *cur_file;

    char c;

    int look(const string buf, const char **arr)
    {

        int i = 0;

        while (arr[i])
        {

            if (buf == arr[i])

                return i;

            ++i;
        }

        return 0;
    }

    void gc()
    {

        c = fgetc(cur_file);
    }

public:
    static const char *TW[], *TD[];

    Scanner(const char *program)
    {

        if (!(cur_file = fopen(program, "r")))

            throw "can’t open file";
    }

    Lex get_lex();
};

const char *

    Scanner::TW[] = {"", "if", "else", "and", "or", "not", "bool", "int", "string", "program", "true", "false", "read",

                     "write", "break", "const", "while", NULL};

const char *

    Scanner::TD[] = {"@", "+", "-", "*", "/", "=", ";", ",", "==", "!=", ">", "<", ">=", "<=", "%", "(", ")", "{", "}", "\"", "\"", NULL};

Lex Scanner::get_lex()
{

    enum state
    {
        H,
        IDENT,
        NUMB,
        ALE,
        NEQ,
        STR
    };

    int j;
    int flg = 0;

    string buf;

    state FL = H;

    do
    {

        gc();

        switch (FL)
        {

        case H:

            if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
                ;

            else if (isalpha(c))
            {

                buf.push_back(c);

                FL = IDENT;
            }

            else if (isdigit(c))
            {

                buf.push_back(c);

                FL = NUMB;
            }

            else if (c == '"')
            {

                buf.push_back(c);

                FL = STR;
            }

            else if (c == '=' || c == '<' || c == '>')
            {

                buf.push_back(c);

                FL = ALE;
            }

            else if (c == EOF)

                return Lex(LEX_FIN);

            else if (c == '!')
            {

                buf.push_back(c);

                FL = NEQ;
            }

            else
            {

                buf.push_back(c);

                if ((j = look(buf, TD)))
                {

                    string str = to_string(j);

                    return Lex((type_of_lex)(j + (int)LEX_FIN), str);
                }

                else

                    throw c;
            }

            break;

        case IDENT:

            if (isalpha(c) || isdigit(c))
            {

                buf.push_back(c);
            }

            else
            {

                ungetc(c, cur_file);

                if ((j = look(buf, TW)))
                {

                    string str = to_string(j);

                    return Lex((type_of_lex)j, str);
                }

                else
                {

                    j = put(buf);

                    return Lex(LEX_IDT, to_string(j));
                }
            }

            break;

        case NUMB:

            if (isdigit(c))
            {

                buf.push_back(c);
            }

            else if (c == '.')
            {

                flg = 1;

                buf.push_back(c);
            }

            else
            {

                ungetc(c, cur_file);

                if (flg == 0)

                    return Lex(LEX_NUM, buf);

                else

                    return Lex(LEX_REAL, buf);
            }

            break;

        case ALE:

            if (c == '=')
            {

                buf.push_back(c);

                j = look(buf, TD);

                string str = to_string(j);

                return Lex((type_of_lex)(j + (int)LEX_FIN), str);
            }

            else
            {

                ungetc(c, cur_file);

                j = look(buf, TD);

                string str = to_string(j);

                return Lex((type_of_lex)(j + (int)LEX_FIN), str);
            }

            break;

        case NEQ:

            if (c == '=')
            {

                buf.push_back(c);

                j = look(buf, TD);

                string str = to_string(j);

                return Lex(LEX_NEQ, str);
            }

            else

                throw '!';

            break;

        case STR:

            if (c == '"')

                return Lex(LEX_STR, buf);

            else

                buf.push_back(c);
        }

    } while (true);
}

ostream &operator<<(ostream &s, Lex l)
{

    string t;

    if (l.t_lex <= LEX_WRITE)

        t = Scanner::TW[l.t_lex];

    else if (l.t_lex >= LEX_FIN && l.t_lex <= LEX_GEQ)

        t = Scanner::TD[l.t_lex - LEX_FIN];

    else if (l.t_lex == LEX_NUM)

        t = "NUMB";

    else if (l.t_lex == LEX_REAL)

        t = "REAL";

    else if (l.t_lex == LEX_IDT)

        t = TID[stoi(l.v_lex)].get_name();

    else

        throw l;

    s << '(' << t << ',' << l.v_lex << ");" << endl;

    return s;
}

int main()

{

    try
    {

        const char *program;

        program = "lexems.txt";

        Scanner S(program);

        Lex lexem;

        lexem = S.get_lex();

        while (lexem.get_type() != LEX_FIN)

        {

            cout << lexem;

            lexem = S.get_lex();
        }

        return 0;
    }

    catch (char c)

    {

        cout << "Unexpected symbol: " << c << endl;

        return 1;
    }

    catch (Lex l)

    {

        cout << "Unexpected lexeme: " << l << endl;

        return 1;
    }

    catch (...)

    {

        cout << "Unidentified error " << endl;

        return 1;
    }
}
