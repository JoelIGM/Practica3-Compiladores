#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <cassert>

//C previous implementations
#include "fa.h"
#include "LinkedList.h"

struct DFA {
    std ::set <int> states;
    std ::set <char > alphabet ;
    int start_state ;
    std ::set <int> accept_states ;
    std ::map <std ::pair <int, char >, int> transitions ;
};

void print_dfa (const DFA& dfa) {
    std :: cout << "--- Tabla de Transiciones (DFA Original) ---\n";
    std :: cout << "Estado Inicial: " << dfa. start_state << "\n";
    std :: cout << "Estados de Aceptacion: ";
    for (int f : dfa. accept_states ) std :: cout << f << " ";
    std :: cout << "\nTransiciones:\n";
    for (const auto& entry : dfa. transitions ) {
        std :: cout << " d(" << entry.first.first << ", '"
                    << entry.first.second << "') -> " << entry.second << "\n";
    }
}

void print_dfa_min (const DFA& dfa_min) {
    std :: cout << "--- Tabla de Transiciones (DFA Minimizado) ---\n";
    std :: cout << "Estado Inicial: " << dfa_min. start_state << "\n";
    std :: cout << "Estados de Aceptacion: ";
    for (int f : dfa_min. accept_states ) std :: cout << f << " ";
    std :: cout << "\nTransiciones:\n";
    for (const auto& entry : dfa_min. transitions ) {
        std :: cout << " d(" << entry.first.first << ", '"
                    << entry.first.second << "') -> " << entry.second << "\n";
    }
}

DFA minimize_dfa (const DFA& dfa) {
    DFA dfa_min;
    // TODO: Implementar el algoritmo de refinamiento de particiones
    return dfa_min;
}

bool test_string (const DFA& dfa , const std :: string& input) {
    int current = dfa. start_state ;
    for (char c : input) {
    auto it = dfa. transitions .find ({ current , c});
    if (it == dfa. transitions .end ()) {
        return false;
    }
    current = it ->second;
    }
    return dfa. accept_states .count(current) > 0;
}

void run_test_suite (const DFA& dfa ,
                    const std :: vector <std :: string >& accept_tests ,
                    const std :: vector <std :: string >& reject_tests ) {
    int passed = 0;
    int total = accept_tests .size () + reject_tests .size ();
    std :: cout << "\n[Corriendo Casos de Aceptacion]\n";
    for (const auto& s : accept_tests ) {
        bool res = test_string (dfa , s);
        std :: cout << "Cadena \"" << s << "\": " << (res ? "PASS" : "FAIL")
            << "\n";
        if (res) passed ++;
    }

    std :: cout << "\n[Corriendo Casos de Rechazo]\n";
    for (const auto& s : reject_tests ) {
        bool res = ! test_string (dfa , s);
        std :: cout << "Cadena \"" << s << "\": " << (res ? "PASS" : "FAIL")
            << "\n";
        if (res) passed ++;
        }

    std :: cout << "\nResultado: " << passed << "/" << total << " pruebas superadas.\n";
}

int main () {
    std :: cout << "\nPROBAMOOOOOS C ----------------------\n";
    transition t = {0, 1, 'a'};
    std::cout << "start: " << t.start << ", finish: " << t.finish << ", symbol: " << t.symbol << std::endl;
    std :: cout << "\nOMAGOTO -----------------------------\n";

    DFA dfa_original ;
    // TODO: Construir el DFA original a partir de su pipeline de regex

    print_dfa ( dfa_original );

    DFA dfa_min = minimize_dfa ( dfa_original );
    print_dfa_min (dfa_min);

    assert( dfa_original .states.size () >= dfa_min.states.size ());
    std :: cout << "\nComprobacion de estados correcta: "
                << dfa_original .states.size () << " >= " << dfa_min.states.
                    size () << "\n";

    std :: vector <std :: string > accept_strings = {
    // Colocar 10 cadenas que deben ser aceptadas
    };

    std :: vector <std :: string > reject_strings = {
    // Colocar 10 cadenas que deben ser rechazadas
    };

    run_test_suite (dfa_min , accept_strings , reject_strings );
    return 0;
}