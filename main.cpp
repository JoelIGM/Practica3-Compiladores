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

DFA minimize_dfa(const DFA& dfa) {
    // Remove unreachable and dead states
    DFA clean = remove_dead_and_unreachable(dfa);
    const std::set<int>& reachable = clean.states;

    // Split valid states into accepting and non-accepting
    std::set<int> F, nonF;
    for (int s : reachable) {
        if (clean.accept_states.count(s))
            F.insert(s);
        else
            nonF.insert(s);
    }

    // Hopcroft's partition refinement algorithm
    std::vector<std::set<int>> P;
    if (!F.empty())    P.push_back(F);
    if (!nonF.empty()) P.push_back(nonF);

    std::vector<std::set<int>> W;
    if (!F.empty())    W.push_back(F);
    if (!nonF.empty()) W.push_back(nonF);

    while (!W.empty()) {
        // Pop block A from W
        std::set<int> A = W.back();
        W.pop_back();

        for (char c : clean.alphabet) {
            
            std::set<int> X;
            for (int q : reachable) {
                auto it = clean.transitions.find({q, c});
                if (it != clean.transitions.end() && A.count(it->second)) {
                    X.insert(q);
                }
            }
            if (X.empty()) continue;

            std::vector<std::set<int>> newP;
            for (auto& Y : P) {
                std::set<int> Y1, Y2;
                for (int s : Y) {
                    if (X.count(s)) Y1.insert(s);
                    else            Y2.insert(s);
                }
                if (Y1.empty() || Y2.empty()) {
                    newP.push_back(Y);
                    continue;
                }
            
                newP.push_back(Y1);
                newP.push_back(Y2);

                bool Y_in_W = false;
                for (auto& w : W) {
                    if (w == Y) { Y_in_W = true; break; }
                }
                if (Y_in_W) {
                    std::vector<std::set<int>> newW;
                    for (auto& w : W) {
                        if (w == Y) {
                            newW.push_back(Y1);
                            newW.push_back(Y2);
                        } else {
                            newW.push_back(w);
                        }
                    }
                    W = newW;
                } else {
                    if (Y1.size() <= Y2.size())
                        W.push_back(Y1);
                    else
                        W.push_back(Y2);
                }
            }
            P = newP;
        }
    }

    // Build the minimized DFA
    std::map<int, int> state_to_block;
    for (int i = 0; i < (int)P.size(); i++) {
        for (int s : P[i]) {
            state_to_block[s] = i;
        }
    }

    DFA dfa_min;
    dfa_min.alphabet = clean.alphabet;

    // States of the minimized DFA = block indices
    for (int i = 0; i < (int)P.size(); i++) {
        dfa_min.states.insert(i);
    }

    // Start state
    dfa_min.start_state = state_to_block[clean.start_state];

    // Accept states
    for (int i = 0; i < (int)P.size(); i++) {
        for (int s : P[i]) {
            if (clean.accept_states.count(s)) {
                dfa_min.accept_states.insert(i);
                break;
            }
        }
    }

    // Transitions
    for (int i = 0; i < (int)P.size(); i++) {
        int rep = *P[i].begin();
        for (char c : clean.alphabet) {
            auto it = clean.transitions.find({rep, c});
            if (it != clean.transitions.end()) {
                int dest_block = state_to_block[it->second];
                dfa_min.transitions[{i, c}] = dest_block;
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

int main () {
    std :: cout << "\nPROBAMOOOOOS C ----------------------\n";
    transition t = {0, 1, 'a'};
    std::cout << "start: " << t.start << ", finish: " << t.finish << ", symbol: " << t.symbol << std::endl;
    std :: cout << "\nOMAGOTO -----------------------------\n";

    DFA dfa_original ;
    // TODO: Build the original DFA from your regex pipeline

    print_dfa ( dfa_original );

    DFA dfa_min = minimize_dfa ( dfa_original );
    print_dfa_min (dfa_min);

    assert( dfa_original .states.size () >= dfa_min.states.size ());
    std :: cout << "\nComprobacion de estados correcta: "
                << dfa_original .states.size () << " >= " << dfa_min.states.
                    size () << "\n";

    std :: vector <std :: string > accept_strings = {
    // Add 10 strings that must be accepted
    };

    std :: vector <std :: string > reject_strings = {
    // Add 10 strings that must be rejected
    };

    run_test_suite (dfa_min , accept_strings , reject_strings );
    return 0;
}