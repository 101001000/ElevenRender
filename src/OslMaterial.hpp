#pragma once
#include <iostream>
#include "Vector.h"
#include "sycl.h"

template <typename T1, typename T2>
struct Union {
    enum class Type {
        NUL, T1_TYPE, T2_TYPE
    };

    Type type;
    T1 d1;
    T2 d2;

    Union(T1 val) {
        d1 = val;
        type = Type::T1_TYPE;
    }

    Union(T2 val) {
        d2 = val;
        type = Type::T2;
    }

    template<>
    T1 val<T1>() {
        return d1;
    }

    template<>
    T2 val<T2>() {
        return d2;
    }
};



template <typename T>
struct Stack {
#define STACK_MAX_SIZE 64

    int idx = -1;
    T arr[STACK_MAX_SIZE];

    void push(T val) {
        arr[++idx] = val;
    }

    T pop() {
        return arr[idx--];
    }

    T top() {
        return arr[idx];
    }

    bool empty() {
        return idx == -1;
    }

};

struct Var {

    enum class Type {
        NUL, UND, FLO, VEC
    };

    Type type;
    float f;
    Vector3 v;

    Var(Vector3 _v) {
        v = _v;
        type = Type::VEC;
    }

    Var(float _f) {
        f = _f;
        type = Type::FLO;
    }

    Var(Type _type) {
        type = _type;
    }

    Var() {
        type = Type::NUL;
    }

};

struct Map {
#define HASH_TABLE_SIZE 1024

public:

    Var vals[HASH_TABLE_SIZE];

    Var& operator [] (const char* str) {
        return vals[hash_string(str)];
    }

    void put(const char* key, Var var) {
        vals[hash_string(key)] = var;
    }

    Var get(const char* key) {
        return vals[hash_string(key)];
    }

    uint32_t hash_string(const char* s)
    {
        uint32_t hash = 0;

        for (; *s; ++s)
        {
            hash += *s;
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }

        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);

        return hash % HASH_TABLE_SIZE;
    }

    void print(sycl::stream sycl_stream) {
        for (int i = 0; i < HASH_TABLE_SIZE; i++) {
            if (vals[i].type == Var::Type::UND)
                sycl_stream << "var " << i << " - " << "keys[i]" << "... (und) \n";
            if (vals[i].type == Var::Type::FLO)
                sycl_stream << "var " << i << " - " << "keys[i]" << "... (float): " << vals[i].f << "\n";
            if (vals[i].type == Var::Type::VEC)
                sycl_stream << "var " << i << " - " << "keys[i]" << "... (vector): (" << vals[i].v[0] << ", " << vals[i].v[1] << ", " << vals[i].v[2] << ")" << "\n";

        }
    }

    void print(std::ostream stream) {
        for (int i = 0; i < HASH_TABLE_SIZE; i++) {
            if (vals[i].type == Var::Type::FLO)
                stream << "var " << i << " - " << "keys[i]" << "... (float): " << vals[i].f << "\n";
            if (vals[i].type == Var::Type::VEC)
                stream << "var " << i << " - " << "keys[i]" << "... (vector): (" << vals[i].v[0] << ", " << vals[i].v[1] << ", " << vals[i].v[2] << ")" << "\n";

        }
    }
};

struct Exp {

    enum class Type {
        NUL, NUM, VEC, VAR, SUM
    };

    Type type;

    int idx = -1;
    float n;
    char x[16];

    Exp* e1 = NULL;
    Exp* e2 = NULL;
    Exp* e3 = NULL;

    Var Vtemp_val;
    //Var Atemp_val;

    //bool Avisited = false;
    bool Vvisited = false;
    //bool Pvisited = false;

    //bool At_visited = false;
    bool Vt_visited = false;
    //bool Pt_visited = false;

    Exp() {
        type = Type::NUL;
    }

    Exp(Type _type, float _n) {
        n = _n;
        type = _type;
    }
    Exp(Type _type, char* _x) {
        strcpy(x, _x);
        type = _type;
    }
    Exp(Type _type, Exp* _e1, Exp* _e2) {
        e1 = _e1;
        e2 = _e2;
        type = _type;
    }
    Exp(Type _type, Exp* _e1, Exp* _e2, Exp* _e3) {
        e1 = _e1;
        e2 = _e2;
        e3 = _e3;
        type = _type;
    }

    int rec_setidx(int n) {

        if (e1 != NULL)
            n = e1->rec_setidx(n) + 1;
        if (e2 != NULL)
            n = e2->rec_setidx(n) + 1;       
        if (e3 != NULL)
            n = e3->rec_setidx(n) + 1;
        idx = n;
        return n;
    }

    void process_A_base(Exp* node, Map& vars, Var* temp_vals) {
        if (node->type == Type::NUM) {
            temp_vals[node->idx] = Var(node->n);
        }
        else if (node->type == Type::VEC) {
            temp_vals[node->idx] = Var(Var::Type::UND);
        }
        else if (node->type == Type::VAR) {
            if (vars[node->x].type == Var::Type::FLO) {
                temp_vals[node->idx] = Var(vars[node->x].f);
            }
            else {
                temp_vals[node->idx] = Var(Var::Type::UND);
            }
        }
        else if (node->type == Type::SUM) {
            if (temp_vals[node->e1->idx].type == Var::Type::FLO && temp_vals[node->e2->idx].type == Var::Type::FLO) {
                temp_vals[node->idx] = Var(temp_vals[node->e1->idx].f + temp_vals[node->e2->idx].f);
            }
            else {
                temp_vals[node->idx] = Var(Var::Type::UND);
            }
        }
    }

    Var A(Map& vars, sycl::stream& out) {

#define A_TEMP_MAX_SIZE 64
        Var val_temp[A_TEMP_MAX_SIZE];

        Stack<Exp*> S;

        S.push(this);
        Exp* prev = NULL;
        while (!S.empty()) {
            Exp* current = S.top();
            if (prev == NULL || prev->e1 == current
                || prev->e2 == current) {
                if (current->e1 && current->type == Type::SUM)
                    S.push(current->e1);
                else if (current->e2 && current->type == Type::SUM)
                    S.push(current->e2);
                else {
                    S.pop();
                    process_A_base(current, vars, val_temp);
                }
            }
            else if (current->e1 == prev) {
                if (current->e2 && current->type == Type::SUM)
                    S.push(current->e2);
                else {
                    S.pop();
                    process_A_base(current, vars, val_temp);
                }
            }
            else if (current->e2 == prev) {
                S.pop();
                process_A_base(current, vars, val_temp);
            }
            prev = current;
        }
        return val_temp[idx];
    }

    /*
    Var A_1(Map& vars, sycl::stream& out) {

        Exp* temp = this;

        while (temp && temp->Avisited == At_visited) {

            if (temp->e1 && temp->e1->Avisited == At_visited && temp->type == SUM)
                temp = temp->e1;

            else if (temp->e2 && temp->e2->Avisited == At_visited && temp->type == SUM)
                temp = temp->e2;

            else if (temp->e3 && temp->e3->Avisited == At_visited && temp->type == SUM)
                temp = temp->e3;

            else {

                if (temp->type == NUM) {
                    temp->Atemp_val = Var(temp->n);
                } else if (temp->type == VEC) {
                    temp->Atemp_val = Var(Var::Type::UND);
                } else if (temp->type == VAR) {
                    if (vars[temp->x].type == Var::Type::FLO) {
                        temp->Atemp_val = Var(vars[temp->x].f);
                    }
                    else {
                        temp->Atemp_val = Var(Var::Type::UND);
                    }
                } else if (temp->type == SUM) {
                    if (temp->e1->Atemp_val.type == Var::Type::FLO && temp->e2->Atemp_val.type == Var::Type::FLO) {
                        temp->Atemp_val = Var(temp->e1->Atemp_val.f + temp->e2->Atemp_val.f);
                    }
                    else {
                        temp->Atemp_val = Var(Var::Type::UND);
                    }
                }
                temp->Avisited = !At_visited;
                temp = this;
            }
        }
        At_visited = !At_visited;
        return Atemp_val;
    }*/

    Var V(Map& vars, sycl::stream& out) {

        Exp* temp = this;

        while (temp && temp->Vvisited == Vt_visited) {

            if (temp->e1 && temp->e1->Vvisited == Vt_visited && temp->type == Type::SUM)
                temp = temp->e1;

            else if (temp->e2 && temp->e2->Vvisited == Vt_visited && temp->type == Type::SUM)
                temp = temp->e2;

            else {

                if (temp->type == Type::NUM) {
                    temp->Vtemp_val = Var(Vector3(temp->n, temp->n, temp->n));
                } else if (temp->type == Type::VEC) {
                    Var Ae1 = temp->e1->A(vars, out);
                    Var Ae2 = temp->e2->A(vars, out);
                    Var Ae3 = temp->e3->A(vars, out);
                    if (Ae1.type == Var::Type::FLO && Ae2.type == Var::Type::FLO && Ae3.type == Var::Type::FLO) {
                        temp->Vtemp_val = Var(Vector3(Ae1.f, Ae2.f, Ae3.f));
                    }
                    else {
                        temp->Vtemp_val = Var(Var::Type::UND);
                    }
                } else if (temp->type == Type::VAR) {
                    if (vars[temp->x].type == Var::Type::VEC) {
                        temp->Vtemp_val = Var(vars[temp->x].v);
                    }
                    else if (vars[temp->x].type == Var::Type::FLO) {
                        temp->Vtemp_val = Var(Vector3(vars[temp->x].f, vars[temp->x].f, vars[temp->x].f));
                    }
                    else {
                        temp->Vtemp_val = Var(Var::Type::UND);
                    }
                } else if (temp->type == Type::SUM) {
                    Var Ve1 = temp->e1->Vtemp_val;
                    Var Ve2 = temp->e2->Vtemp_val;
                    if (Ve1.type == Var::Type::VEC && Ve2.type == Var::Type::VEC) {
                        temp->Vtemp_val = Var(Ve1.v + Ve2.v);
                    }
                    else {
                        temp->Vtemp_val = Var(Var::Type::UND);
                    }
                }

                temp->Vvisited = !Vt_visited;
                temp = this;
            }
        }
        Vt_visited = !Vt_visited;
        return Vtemp_val;
    }

    /*

    void print(sycl::stream& out) {

        Exp* temp = this;

        while (temp && temp->Pvisited == Pt_visited) {

            if (temp->e1 && temp->e1->Pvisited == Pt_visited)
                temp = temp->e1;

            else if (temp->e2 && temp->e2->Pvisited == Pt_visited)
                temp = temp->e2;

            else if (temp->e3 && temp->e3->Pvisited == Pt_visited)
                temp = temp->e3;

            else {
                if (temp->Atemp_val.type == Var::Type::NUL && temp->Vtemp_val.type == Var::Type::NUL) {
                    switch (temp->type) {
                    case NUM:
                        out << "NUM(" << temp->n << ") ";
                        break;
                    case VEC:
                        out << "VEC(?,?,?) ";
                        break;
                    case VAR:
                        out << "VAR(" << temp->x << " = ?) ";
                        break;
                    case SUM:
                        out << "SUM(? + ?) ";
                        break;
                    }
                }
                else if (temp->Atemp_val.type == Var::Type::UND & temp->Vtemp_val.type == Var::Type::UND) {
                    switch (temp->type) {
                    case NUM:
                        out << "NUM(und) ";
                        break;
                    case VEC:
                        out << "VEC(und) ";
                        break;
                    case VAR:
                        out << "VAR(und) ";
                        break;
                    case SUM:
                        out << "SUM(und) ";
                        break;
                    }
                }
                else if (temp->Atemp_val.type == Var::Type::FLO) {
                    switch (temp->type) {
                    case NUM:
                        out << "NUM(" << temp->Atemp_val.f << ") ";
                        break;
                    case VEC:
                        //out << "VEC(" << temp->temp_val.v[0] << "," << temp->temp_val.v[1] << "," << temp->temp_val.v[2] << ") ";
                        break;
                    case VAR:
                        out << "VAR(" << temp->x << "=" << temp->Atemp_val.f << ") ";
                        break;
                    case SUM:
                        out << "SUM(" << temp->e1->Atemp_val.f << "+" << temp->e2->Atemp_val.f << "=" << temp->Atemp_val.f << ") ";
                        break;
                    }
                }
                else if (Vtemp_val.type == Var::Type::VEC) {
                    switch (temp->type) {
                    case NUM:
                        //out << "NUM(" << temp->temp_val.f << ") ";
                        break;
                    case VEC:
                        out << "VEC(" << temp->Vtemp_val.v[0] << "," << temp->Vtemp_val.v[1] << "," << temp->Vtemp_val.v[2] << ") ";
                        break;
                    case VAR:
                        out << "VAR(" << temp->x << "=(" << temp->Vtemp_val.v[0] << "," << temp->Vtemp_val.v[1] << "," << temp->Vtemp_val.v[2] << ")) ";
                        break;
                    case SUM:
                        out << "SUM(" << "(" << temp->e1->Vtemp_val.v[0] << "," << temp->e1->Vtemp_val.v[1] << "," << temp->e1->Vtemp_val.v[2] << ")"
                            << " + " << "(" << temp->e2->Vtemp_val.v[0] << "," << temp->e2->Vtemp_val.v[1] << "," << temp->e2->Vtemp_val.v[2] << ")"
                            << " = " << "(" << temp->Vtemp_val.v[0] << "," << temp->Vtemp_val.v[1] << "," << temp->Vtemp_val.v[2] << ")) ";
                        break;
                    }
                }

                temp->Pvisited = !Pt_visited;
                temp = this;
            }
        }
        Pt_visited = !Pt_visited;
    }*/
};


struct Statement {

    enum class Type {
        NUL, SEQ, ASS, SKIP, IF
    };

    Type type;

    Statement* s1 = NULL;
    Statement* s2 = NULL;

    char x[16];
    Exp* e;

    bool visited = false;
    bool t_visited = false;

    Statement(Type _type) {
        type = _type;
    }
    Statement(Type _type, char* _x, Exp* _e) {
        type = _type;
        strcpy(x, _x);
        e = _e;
    }
    Statement(Type _type, Statement* _s1, Statement* _s2) {
        type = _type;
        s1 = _s1;
        s2 = _s2;
    }
    Statement(Type _type, Exp* _e, Statement* _s1, Statement* _s2) {
        type = _type;
        e = _e;
        s1 = _s1;
        s2 = _s2;
    }

    void print(sycl::stream& out) {

        Statement* temp = this;

        while (temp && temp->visited == t_visited) {

            if (temp->s1 && temp->s1->visited == t_visited)
                temp = temp->s1;

            else if (temp->s2 && temp->s2->visited == t_visited)
                temp = temp->s2;

            else {
                switch (temp->type) {
                case Statement::Type::NUL:
                    out << "NUL ";
                    break;
                case Statement::Type::SKIP:
                    out << "SKIP ";
                    break;
                case Statement::Type::SEQ:
                    out << "SEQ ";
                    break;
                case Statement::Type::ASS:
                    out << "ASS(" << temp->x << "=";
                    //temp->e->print(out);
                    out << ") ";
                    break;
                case Statement::Type::IF:
                    out << "IF ";
                    break;
                }

                temp->visited = !t_visited;
                temp = this;
            }
        }
        t_visited = !t_visited;
    }

    void execute(Map& vars, sycl::stream& out) {

        Statement* temp = this;

        bool if_result = true;

        while (temp && temp->visited == t_visited) {

            if (temp->type == Type::IF)
                if_result = (temp->e->A(vars, out).f != 0);

            if (temp->s1 && temp->s1->visited == t_visited && (!(temp->type == Type::IF) || if_result))
                temp = temp->s1;

            else if (temp->s2 && temp->s2->visited == t_visited && (!(temp->type == Type::IF) || !if_result))
                temp = temp->s2;

            else {
                switch (temp->type) {
                case Statement::Type::NUL:
                    break;
                case Statement::Type::SKIP:
                    break;
                case Statement::Type::SEQ:
                    break;
                case Statement::Type::ASS:

                    if (temp->e->A(vars, out).type != Var::Type::UND) {
                        vars[temp->x] = temp->e->A(vars, out);
                    }
                    else if(temp->e->V(vars, out).type != Var::Type::UND){
                        vars[temp->x] = temp->e->V(vars, out);
                    }
                    else {
                        vars[temp->x] = Var(Var::Type::UND);
                    }
                    break;
                case Statement::Type::IF:
                    break;
                }

                temp->visited = !t_visited;
                temp = this;
            }
        }
        t_visited = !t_visited;
    }
};


struct OslMaterial {

    Map vars;
    Statement* program;

    void set_global_vars(Vector3 P,
                         Vector3 I,
                         Vector3 N,
                         Vector3 Ng,
                         float u,
                         float v){

        vars["P"] = Var(P);
        vars["I"] = Var(I);
        vars["N"] = Var(N);
        vars["Ng"] = Var(Ng);
        vars["u"] = Var(u);
        vars["v"] = Var(v);
    }

    Vector3 compute_ci(sycl::stream& out) {
        program->execute(vars, out);
        return vars["Ci"].v;
    }
};
