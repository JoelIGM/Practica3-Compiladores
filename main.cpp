#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <cassert>
#include <deque>
#include <queue>
#include <algorithm>
#include <iterator>

//C previous implementations
#include "fa.h"
#include "LinkedList.h"
#include "regex.h"
#include "stack.h"
#include "nfa.h"

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

DFA remove_dead_and_unreachable(const DFA& dfa) {
    // Forward reachability
    // DFS from the start state
    std::set<int> reachable;
    std::vector<int> frontier = {dfa.start_state};
    reachable.insert(dfa.start_state);
    while (!frontier.empty()) {
        int cur = frontier.back();
        frontier.pop_back();
        for (char c : dfa.alphabet) {
            auto it = dfa.transitions.find({cur, c});
            if (it != dfa.transitions.end()) {
                int next = it->second;
                if (!reachable.count(next)) {
                    reachable.insert(next);
                    frontier.push_back(next);
                }
            }
        }
    }

    
    // Build the reversed transition graph
    std::map<int, std::vector<int>> reverse_trans;
    for (const auto& t : dfa.transitions) {
        int from = t.first.first;
        int to   = t.second;
        if (reachable.count(from) && reachable.count(to)) {
            reverse_trans[to].push_back(from);
        }
    }

    // DFS backwards from every accepting state
    std::set<int> live;
    std::vector<int> back_frontier;
    for (int a : dfa.accept_states) {
        if (reachable.count(a)) {
            live.insert(a);
            back_frontier.push_back(a);
        }
    }
    while (!back_frontier.empty()) {
        int cur = back_frontier.back();
        back_frontier.pop_back();
        for (int prev : reverse_trans[cur]) {
            if (!live.count(prev)) {
                live.insert(prev);
                back_frontier.push_back(prev);
            }
        }
    }

    // Valid states
    std::set<int> valid;
    for (int s : reachable) {
        if (live.count(s))
            valid.insert(s);
    }

    // Build clean DFA keeping only valid states 
    DFA clean;
    clean.alphabet    = dfa.alphabet;
    clean.states      = valid;
    clean.start_state = dfa.start_state;

    for (int s : dfa.accept_states)
        if (valid.count(s))
            clean.accept_states.insert(s);

    for (const auto& t : dfa.transitions) {
        int  from = t.first.first;
        char c    = t.first.second;
        int  to   = t.second;
        if (valid.count(from) && valid.count(to))
            clean.transitions[{from, c}] = to;
    }

    return clean;
}

static int delta(const DFA& M, int q, char c) {
    auto it = M.transitions.find({q, c});
    return (it != M.transitions.end()) ? it->second : -1;
}

DFA minimize_dfa (const DFA& dfa) {
    DFA dfa_min;

    // partición inicial
    std::set<int> F = dfa.accept_states;
    std::set<int> Q_minus_F;
    std::set_difference(dfa.states.begin(), dfa.states.end(),
                         F.begin(), F.end(),
                         std::inserter(Q_minus_F, Q_minus_F.begin()));
    std::set<std::set<int>> P;

    if (!F.empty()) P.insert(F);
    if (!Q_minus_F.empty()) P.insert(Q_minus_F);

    // cola
    std::deque<std::set<int>> W;
    if (!F.empty() && F != dfa.states) {
        W.push_back(F);
        W.push_back(Q_minus_F);
    }

    // algoritmo 1
    while (!W.empty()) {
        std::set<int> A = W.front();
        W.pop_front();

        for (char c : dfa.alphabet) {
            // X los estados que tienen una transición c en A
            std::set<int> X;
            for (int q : dfa.states) {
                int dq = delta(dfa, q, c);
                if (dq != -1 && A.count(dq)) X.insert(q);
            }

	    // si no hay nada, vamos con el siguiente char
            if (X.empty()) {
	    	continue;
	    }

	    // guardamos P pq se modifica dentro del ciclo
            std::vector<std::set<int>> P_snapshot(P.begin(), P.end());

            for (const std::set<int>& Y : P_snapshot) {
                std::set<int> Y1;
                std::set_intersection(Y.begin(), Y.end(), X.begin(), X.end(),
                                       std::inserter(Y1, Y1.begin()));
                std::set<int> Y2;
                std::set_difference(Y.begin(), Y.end(), X.begin(), X.end(),
                                     std::inserter(Y2, Y2.begin()));

                if (Y1.empty() || Y2.empty()) continue; // Y no se parte

                // ahora P = (P \ {Y}) ∪ {Y1, Y2}
                P.erase(Y);
                P.insert(Y1);
                P.insert(Y2);

                auto itW = std::find(W.begin(), W.end(), Y);
                if (itW != W.end()) {
                    *itW = Y1;
                    W.push_back(Y2);
                } else {
                    if (Y1.size() <= Y2.size()) W.push_back(Y1);
                    else                        W.push_back(Y2);
                }
            }
        }
    }

    // construimos el dfa mínimo a partir de nuestra partición P

    // primero mapeamos cada estado original a índice de su bloque
    std::map<int, int> estado_a_bloque;
    std::vector<std::set<int>> bloques(P.begin(), P.end());
    for (size_t i = 0; i < bloques.size(); ++i) {
        for (int q : bloques[i]) {
            estado_a_bloque[q] = static_cast<int>(i);
        }
    }

    // el alfabeto es el mismo lol
    dfa_min.alphabet = dfa.alphabet;

    // los estados uno por bloque
    for (size_t i = 0; i < bloques.size(); ++i) {
        dfa_min.states.insert(static_cast<int>(i));
    }

    // el estado inicial es el bloque que contiene a dfa.start_state
    dfa_min.start_state = estado_a_bloque[dfa.start_state];

    // estados de aceptación son los bloques que contienen al menos un estado de aceptación
    for (size_t i = 0; i < bloques.size(); ++i) {
        for (int q : bloques[i]) {
            if (dfa.accept_states.count(q)) {
                dfa_min.accept_states.insert(static_cast<int>(i));
                break;
            }
        }
    }

    // para las transiciones tomamos un representante de cada bloque y vemos a dónde va
    for (size_t i = 0; i < bloques.size(); ++i) {
        int representante = *bloques[i].begin();
        for (char c : dfa.alphabet) {
            int dq = delta(dfa, representante, c);
            if (dq != -1) {
                int destino_bloque = estado_a_bloque[dq];
                dfa_min.transitions[{static_cast<int>(i), c}] = destino_bloque;
            }
        }
    }
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

fa nfa_to_fa(const nfa& input_nfa) {
    // obtenemos el alfabeto del nfa
    std::set<char> alphabet;
    for (int i = 0; i < input_nfa.length; ++i) {
        alphabet.insert(input_nfa.transitions[i].symbol);
    }

    // nuestras estructuras auxiliares para el algoritmo de subconjuntos
    std::map<std::set<int>, int> dfa_state_ids;
    std::vector<std::set<int>> dfa_states;
    std::queue<std::set<int>> unmarked_states;

    std::vector<transition> dfa_transitions;
    std::vector<int> dfa_accept_states;

    // estado inicial del dfa: conjunto que contiene el estado inicial del nfa
    std::set<int> initial_set = { input_nfa.start };
    dfa_state_ids[initial_set] = 0;
    dfa_states.push_back(initial_set);
    unmarked_states.push(initial_set);

    while (!unmarked_states.empty()) {
        std::set<int> current_set = unmarked_states.front();
        unmarked_states.pop();

        int current_dfa_id = dfa_state_ids[current_set];

        // Verificar si es un estado de aceptación
        if (current_set.count(input_nfa.accept)) {
            if (std::find(dfa_accept_states.begin(), dfa_accept_states.end(), current_dfa_id) == dfa_accept_states.end()) {
                dfa_accept_states.push_back(current_dfa_id);
            }
        }

        // por cada símbolo del alfabeto...
        for (char sym : alphabet) {
            std::set<int> next_set;

            // transición delta para cada estado s en el conjunto actual
            for (int state : current_set) {
                for (int i = 0; i < input_nfa.length; ++i) {
                    if (input_nfa.transitions[i].start == state && input_nfa.transitions[i].symbol == sym) {
                        next_set.insert(input_nfa.transitions[i].finish);
                    }
                }
            }

            if (next_set.empty()) continue; // omitir transiciones a estado pozo 

            // si descubrimos un nuevo conjunto de estados, asignamos un ID
            if (dfa_state_ids.find(next_set) == dfa_state_ids.end()) {
                int next_id = dfa_states.size();
                dfa_state_ids[next_set] = next_id;
                dfa_states.push_back(next_set);
                unmarked_states.push(next_set);
            }

            // registramos la transición del DFA
            transition t;
            t.start = current_dfa_id;
            t.finish = dfa_state_ids[next_set];
            t.symbol = sym;
            dfa_transitions.push_back(t);
        }
    }

    // copia de datos a la de destino
    fa result;
    result.start = 0;
    result.states = dfa_states.size();
    result.length = dfa_transitions.size();

    // memoria para transiciones
    result.transitions = (transition*)malloc(result.length * sizeof(transition));
    for (int i = 0; i < result.length; ++i) {
        result.transitions[i] = dfa_transitions[i];
    }

    // memoria para estados de aceptación
    result.length_accept_states = dfa_accept_states.size();
    result.accept_states = (int*)malloc(result.length_accept_states * sizeof(int));
    for (int i = 0; i < result.length_accept_states; ++i) {
        result.accept_states[i] = dfa_accept_states[i];
    }

    return result;
}

DFA convert_dfa_to_cpp(const dfa& c_dfa) {
    DFA cpp_dfa;

    for (int state = 0; state < c_dfa.length_states; ++state) {
        cpp_dfa.states.insert(state);
    }

    cpp_dfa.start_state = c_dfa.start;

    for (int i = 0; i < c_dfa.length_accept_states; ++i) {
        cpp_dfa.accept_states.insert(c_dfa.accept_states[i]);
    }

    for (int i = 0; i < c_dfa.length_transitions; ++i) {
        const transition& current = c_dfa.transitions[i];
        cpp_dfa.alphabet.insert(current.symbol);
        cpp_dfa.transitions[{current.start, current.symbol}] = current.finish;
    }

    return cpp_dfa;
}

int main () {
    DFA dfa_original;
    
    // TODO: Construir el DFA original a partir de su pipeline de regex
    //Create the regular expresions 1,2 and 5
    const char *regex1 = "(a|b)*abb";
    const char *regex2 = "(0|1)*01(0|1)*";
    const char *regex5 = "(0|10*1)*";
    //Make them in regex format
    regex r1 = parse_regex(regex1);
    regex r2 = parse_regex(regex2);
    regex r5 = parse_regex(regex5);
    //Create the thompson automatas
    nfa nfa1 = regex_to_nfa(r1);
    nfa nfa2 = regex_to_nfa(r2);
    nfa nfa5 = regex_to_nfa(r5);
    //Change the format nfa to fa (automatas with more than one acceptance state)
    fa fa1 = nfa_to_fa(nfa1);
    fa fa2 = nfa_to_fa(nfa2);
    fa fa5 = nfa_to_fa(nfa5);
    //Transform them to dfa
    dfa *dfa1 = nfa_to_dfa(&fa1,"ab", 2);
    dfa *dfa2 = nfa_to_dfa(&fa2,"01", 2);
    dfa *dfa5 = nfa_to_dfa(&fa5,"01", 2);
    //dfa from c to DFA in cpp
    DFA cpp_dfa1 = convert_dfa_to_cpp(*dfa1);
    DFA cpp_dfa2 = convert_dfa_to_cpp(*dfa2);
    DFA cpp_dfa5 = convert_dfa_to_cpp(*dfa5);
    print_dfa ( cpp_dfa1 );
    print_dfa ( cpp_dfa2 );
    print_dfa ( cpp_dfa5 );

    //Minimize the DFA
    DFA min_dfa1 = minimize_dfa(cpp_dfa1);
    DFA min_dfa2 = minimize_dfa(cpp_dfa2);
    DFA min_dfa5 = minimize_dfa(cpp_dfa5);

    print_dfa_min (min_dfa1);
    print_dfa_min (min_dfa2);
    print_dfa_min (min_dfa5);

    assert( cpp_dfa1.states.size () >= min_dfa1.states.size ());
    std :: cout << "\nComprobacion de estados correcta: "
                << cpp_dfa1 .states.size () << " >= " << min_dfa1.states.
                    size () << "\n";
    assert( cpp_dfa1.states.size () >= min_dfa1.states.size ());
    std :: cout << "\nComprobacion de estados correcta: "
                << cpp_dfa1 .states.size () << " >= " << min_dfa1.states.
                    size () << "\n";
    assert( cpp_dfa1.states.size () >= min_dfa1.states.size ());
    std :: cout << "\nComprobacion de estados correcta: "
                << cpp_dfa1 .states.size () << " >= " << min_dfa1.states.
                    size () << "\n";

    // Expresión 1: (a|b)*abb - Cadenas que terminan con el sufijo abb
    std::vector<std::string> accept_strings1 = {
        "abb", "aabb", "babb", "ababb", "bbabb", "aaaaabb", "babababb", "bbbbabb", "ababbabb", "bbaabb"
    };

    std::vector<std::string> reject_strings1 = {
        "a", "b", "ab", "ba", "abba", "bb", "abab", "bbab", "aaaaa", ""
    };

    // Expresión 2: (0|1)*01(0|1)* - Cadenas binarias que contienen la subcadena 01
    std::vector<std::string> accept_strings2 = {
        "01", "001", "010", "101", "11011", "0001000", "0111", "1101", "010101", "1001"
    };

    std::vector<std::string> reject_strings2 = {
        "0", "1", "00", "11", "110", "000", "111", "10", "111000", "0000"
    };

    // Expresión 5: (0|10*1)* - Cadenas binarias con un número par de unos
    std::vector<std::string> accept_strings5 = {
        "", "0", "11", "011", "101", "110", "000", "1001", "1111", "01010"
    };

    std::vector<std::string> reject_strings5 = {
        "1", "01", "10", "001", "100", "111", "0111", "1110", "10101", "010101"
    };

    run_test_suite (min_dfa1 , accept_strings1 , reject_strings1);
    run_test_suite (min_dfa2 , accept_strings2 , reject_strings2 );
    run_test_suite (min_dfa5 , accept_strings5 , reject_strings5 );

    free_regex(&r1);
    free_regex(&r2);
    free_regex(&r5);
    return 0;
}