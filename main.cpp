/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <set>
#include <algorithm>
#include "lexer.h"

using namespace std;

LexicalAnalyzer lex;

struct RHS{
    string tok;
    RHS * next;
};

struct rule{
    string non;
    RHS * rhs;
    rule * next;
};

//method to parse RHS using recursion. ends by consuming a HASH
bool parseRHS(RHS * r, Token t){
    if(t.token_type == ID){
        r->tok.assign(t.lexeme);
        //r = temp;
        return parseRHS(r->next = new RHS, lex.GetToken());
    }
    else if(t.token_type == HASH){
        return true;
    }
    else
        return false; //encountered invalid char; error
}

bool parseRule(rule * r, Token t){
    if(t.token_type == ID){
        r->non = t.lexeme;
        t = lex.GetToken();
        if(t.token_type == ARROW){
            if(!parseRHS(r->rhs = new RHS, lex.GetToken())){
                cout << "Error: tokens on RHS must either be ID or a HASH\n";
                return 1;
            }
        }else{
            cout << "Error: rules must have an arrow as their second token\n";
            return 1;
        }

        return parseRule(r->next = new rule, lex.GetToken());
    }
    else if(t.token_type == DOUBLEHASH){
        return true;
    }
    else
        return false;
}

pair<vector<string>, vector<string> > sortTokens(rule * r){
    vector<string> non;
    vector<string> term;

    //get set of non terminals
    set<string> nonT;

    rule * rIt = r;
    while(rIt != NULL && rIt->next != NULL){
        //add non if not already added
        nonT.insert(rIt->non);

        rIt = rIt->next;
    }

    //get ordered set of non terminals and of everything in RHS's
    rule * rIt2 = r;
    while(rIt2 != NULL && rIt2->next != NULL){
        if(find(nonT.begin(), nonT.end(), rIt2->non) != nonT.end() && find(non.begin(), non.end(), rIt2->non) == non.end())
        //token is a nonterminal and not already added
            non.push_back(rIt2->non);

        RHS * rhsIt = rIt2->rhs;
        while(rhsIt != NULL && rhsIt->next != NULL){
            //add token to non vector
            string symbol = rhsIt->tok;
            if(find(nonT.begin(), nonT.end(),symbol) != nonT.end() && find(non.begin(), non.end(), symbol) == non.end())
                //token is a nonterminal and not already added
                non.push_back(symbol);

            else if(find(nonT.begin(), nonT.end(),symbol) == nonT.end() && find(term.begin(), term.end(), symbol) == term.end())
                //token is a non terminal and not already added
                term.push_back(symbol);

            rhsIt = rhsIt->next;
        }
        rIt2 = rIt2->next;
    }


    return make_pair(non, term);
}

void printAll(rule * r){ //prints nonterminals and terminals in order
    //get nonterminals and terminals
    pair<vector<string>, vector<string> > tokens = sortTokens(r);
    vector<string> non = tokens.first;
    vector<string> term = tokens.second;

    //print non terms in order
    vector<string>::iterator nonIt;
    for(nonIt = non.begin(); nonIt != non.end(); nonIt++)
        cout << *nonIt << " ";

    //print terms in order
    vector<string>::iterator termIt;
    for(termIt = term.begin(); termIt != term.end(); termIt++)
        cout << *termIt << " ";

}
 vector<rule *> prune(rule * r){
    //get nonterminals and terminals
    pair<vector<string>, vector<string> > tokens = sortTokens(r);
    vector<string> non = tokens.first;
    vector<string> term = tokens.second;

    //find generative symbols
    set<string> gen;
    gen.insert(term.begin(), term.end());

    vector<rule*> newRules; //will be new list of rules

    bool changed = true;

    while(changed) {
        changed = false;

        rule * it = r;
        while (it != NULL && it->next != NULL) {
            bool allGen = true; //keep track of if RHS is all generative symbols
            RHS *rhsIt = it->rhs;
            while (rhsIt != NULL && rhsIt->next != NULL) {
                string symbol = rhsIt->tok;
                if(find(gen.begin(), gen.end(), symbol) == gen.end()){ //symbol is not generative
                    allGen = false;
                }
                rhsIt = rhsIt->next;
            }
            if (allGen){
                if(find(gen.begin(), gen.end(), it->non) == gen.end()){ //symbol has not been added already
                    gen.insert(it->non);
                    changed = true;
                }
            }

            it = it->next;
        }
    }

    //get rid of all rules with non-generative symbols
    rule * it1 = r;
    while(it1 != NULL && it1->next != NULL){
        bool allGen = true; //assume all symbols in rule are generative
        if(find(gen.begin(), gen.end(), it1->non) == gen.end())//LHS not generative
            allGen = false;
        else{
            RHS * rhsIt = it1->rhs;
            while(rhsIt != NULL && rhsIt->next != NULL){
                if(find(gen.begin(), gen.end(), rhsIt->tok) == gen.end())//RHS symbol not generative
                    allGen = false;
                rhsIt = rhsIt->next;
            }
        }

        if(allGen)
            newRules.push_back(it1);

        it1 = it1->next;
    }

    //find all reachable symbols
    set<string> reach;
    vector<rule *> newRules2;

    reach.insert(newRules.at(0)->non);
    newRules2.push_back(newRules.at(0)); //start symbol is reachable

    bool changed2 = true;
    while(changed2) {
        changed2 = false;
        //increment through all reachable rules
        for(int pos = 0; pos < newRules.size(); pos++){
            if(find(reach.begin(), reach.end(),newRules.at(pos)->non) != reach.end()){
                //rule is reachable
                RHS *rhsIt = newRules.at(pos)->rhs; //all nonterms in RHS are reachable
                while (rhsIt != NULL && rhsIt->next != NULL) {
                    if (find(non.begin(), non.end(), rhsIt->tok) != non.end() &&
                        find(reach.begin(), reach.end(), rhsIt->tok) == reach.end()) {
                        //token is a nonterminal and not already added
                        reach.insert(rhsIt->tok);
                        changed2 = true;
                    }
                    rhsIt = rhsIt->next;
                }
            }
        }
    }

    //get rid of all rules with non-reachable symbols
    for(int it3 = 1; it3 < newRules.size(); it3++){
        if(find(reach.begin(), reach.end(), newRules.at(it3)->non) != reach.end())
            //LHS is reachable
            newRules2.push_back(newRules.at(it3));
    }

    return newRules2;

}

void printVec(vector<rule *> v){
    for(int i = 0; i < v.size(); i++){
        cout << v.at(i)->non << " ->";
        RHS * rhsIt = v.at(i)->rhs;
        if(rhsIt->tok.empty())
            cout << " #";
        else
            while(rhsIt != NULL && rhsIt->next != NULL){
                cout << " " << rhsIt->tok;
                rhsIt = rhsIt->next;
            }
        cout << endl;
    }
}

int main (int argc, char* argv[])
{
    int task;

    if (argc < 2)
    {
        cout << "Error: missing argument\n";
        return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the first argument to your program is stored in argv[1]
     */

    task = atoi(argv[1]);

    //parse first rule
    Token t = lex.GetToken();
    rule *head = new rule;

    if(t.token_type == ID) {
        head->non = t.lexeme;
        //head->rhs = (RHS*) malloc(sizeof(RHS));
        //head->next = (rule*) malloc(sizeof(rule));
        t = lex.GetToken();
        if(t.token_type == ARROW){
            if(!parseRHS(head->rhs = new RHS, lex.GetToken())){
                cout << "Error: tokens on RHS must either be ID or a HASH\n";
                return 1;
            }

            //parse any additional rules
            if(!parseRule(head->next = new rule, lex.GetToken())){
                cout << "Error: invalid rules\n";
                return 1;
            }
        }else{
            cout << "Error: rules must have an arrow as their second token\n";
            return 1;
        }
    }else{
        cout << "Error: rules must start with an ID\n";
        return 1;
    }

    int a = 0;
    /*
       Hint: You can modify and use the lexer from previous project
       to read the input. Note that there are only 4 token types needed
       for reading the input in this project.

       WARNING: You will need to modify lexer.cc and lexer.h to only
       support the tokens needed for this project if you are going to
       use the lexer.
     */

    switch (task) {
        case 1:
            printAll(head);
            break;

        case 2:
            printVec(prune(head));
            break;

        case 3:
            // TODO: perform task 3.
            break;

        case 4:
            // TODO: perform task 4.
            break;

        case 5:
            // TODO: perform task 5.
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
}

