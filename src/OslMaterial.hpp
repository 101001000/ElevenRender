#pragma once
#include <iostream>
#include "Vector.h"

struct Var {

    enum Type {
        NUL, FLO, VEC
    };

    Type type;
    float f;
    Vector3 v;

    Var(Vector3 _v) {
        v = _v;
        type = VEC;
    }

    Var(float _f) {
        f = _f;
        type = VEC;
    }

    Var() {
        type = Var::NUL;
    }

};

struct Map {
#define HASH_TABLE_SIZE 1024

public:

    Var vals[HASH_TABLE_SIZE];

    Var& operator [] (const char* str) {
        return vals[hash_string(str)];
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

    void print() {
        for (int i = 0; i < HASH_TABLE_SIZE; i++) {
            if (vals[i].type == Var::FLO)
                std::cout << "var " << i << "(float): " << vals[i].f << std::endl;
            if (vals[i].type == Var::VEC)
                std::cout << "var " << i << "(vector): (" << vals[i].v[0] << ", " << vals[i].v[1] << ", " << vals[i].v[2] << ")" << std::endl;
            
        }
    }

};


struct Exp {

    enum Type {
        NUL, NUM, VEC, VAR, SUM
    };

    Type type;
    float n;
    char* x;
    Exp* e1;
    Exp* e2;
    Exp* e3;

    Exp(Type _type, float _n) {
        n = _n;
        type = _type;
    }

    Exp(Type _type, char* _x) {
        x = _x;
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

    Vector3 V(Map& map) {
        switch (type) {
        case NUM:
            return Vector3(n, n, n);
        case VEC:
            return Vector3(e1->A(map), e2->A(map), e3->A(map));
        case VAR:
            return map[x].v;
        case SUM:
            return e1->V(map) + e2->V(map);
        }
    }

    float A(Map& map) {
        switch (type) {
        case NUM:
            return n;
        case VEC:
            return NAN;
        case VAR:
            return map[x].f;
        case SUM:
            return e1->A(map) + e2->A(map);
        }
    }

};


struct Statement {

    enum Type {
        NUL, SEQ, ASS, SKIP, IF
    };

    Type type;

    Statement* s1;
    Statement* s2;

    char* x;
    Exp* e;

    Statement(Type _type) {
        type = _type;
    }

    Statement(Type _type, char* _x, Exp* _e) {
        type = _type;
        x = _x;
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

    void execute(Map& map) {
        switch (type) {
        case NUL:
            break;
        case SKIP:
            break;
        case SEQ:
            s1->execute(map);
            s2->execute(map);
            break;
        case ASS:
            if (!isnan(e->A(map))) {
                map[x].type = Var::FLO;
                map[x].f = e->A(map);
            }
            else {
                map[x].type = Var::VEC;
                map[x].v = e->V(map);
            }
            break;
        case IF:
            if (e->A(map) != 0) {
                s1->execute(map);
            }
            else {
                s2->execute(map);
            }
            break;
        }
    }

    void print() {
        switch (type) {
        case NUL:
            std::cout << "STATEMENT: NUL";
            break;
        case SKIP:
            std::cout << "STATEMENT: SKIP";
            break;
        case SEQ:
            std::cout << "STATEMENT: SEQ (";
            s1->print();
            std::cout << ", ";
            s2->print();
            std::cout << ")";
            break;
        case ASS:
            std::cout << "STATEMENT: ASS(" << x << ")";
            break;
        case IF:
            std::cout << "STATEMENT: IF (";
            s1->print();
            std::cout << ", ";
            s2->print();
            std::cout << ")";
            break;
        }
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

        vars["P"].v = P;
        vars["I"].v = I;
        vars["N"].v = N;
        vars["Ng"].v = Ng;
        vars["u"].f = u;
        vars["v"].f = v;
    }

    Vector3 compute_ci() {
        program->execute(vars);
        return vars["Ci"].v;
    }
};
