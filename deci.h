#ifndef _DECI_H_
#define _DECI_H_

#include "basis_pms.h"

using namespace std;

class Decimation
{
  public:
    Decimation(lit **ls_var_lit, int *ls_var_lit_count, lit **ls_clause_lit, long long *ls_org_clause_weight, long long ls_top_clause_weight);

    void make_space(int max_c, int max_v);
    void free_memory();
    void init(int *ls_local_opt, int *ls_global_opt, lit *ls_unit_clause, int ls_unit_clause_count, int *ls_clause_lit_count, int *ls_hard_binary_clause, int ls_hard_binary_clause_count, int *ls_soft_binary_clause, int ls_soft_binary_clause_count);
    void push_unit_clause_to_queue(lit tem_l);
    void assign(int v, int sense);
    void remove_unassigned_var(int v);
    void hunit_propagation();
    void hbinary_propagation();
    void sbinary_propagation();
    void sunit_propagation();
    void unit_propagation();
    void random_propagation();
    void unit_prosess();
    bool choose_sense(int v);

    vector<int> fix;

    int num_vars;
    int num_clauses;

    long long *h_true_score;
    long long *h_false_score;
    long long *hscore;
    long long *s_true_score;
    long long *s_false_score;
    long long *sscore;

    lit **clause_lit;
    lit **var_lit;
    int *var_lit_count;

    int *local_opt;
    int *global_opt;
    long long *org_clause_weight;
    long long top_clause_weight;

    lit *hunit_clause_queue;
    int *sense_in_hunit_clause_queue;
    int hunit_beg_pointer;
    int hunit_end_pointer;

    lit *sunit_clause_queue;
    int *sense_in_sunit_clause_queue;
    int sunit_beg_pointer;
    int sunit_end_pointer;

    int *unassigned_var;
    int *index_in_unassigned_var;
    int unassigned_var_count;

    int *clause_delete;
    int *clause_lit_count;
    
    int *hard_binary_clause;
    int *soft_binary_clause;
    int *index_in_hard_binary_clause;
    int *index_in_soft_binary_clause;
    int hard_binary_clause_count;
    int soft_binary_clause_count;
};

Decimation::Decimation(lit **ls_var_lit, int *ls_var_lit_count, lit **ls_clause_lit, long long *ls_org_clause_weight, long long ls_top_clause_weight)
{
    var_lit = ls_var_lit;
    var_lit_count = ls_var_lit_count;
    clause_lit = ls_clause_lit;
    org_clause_weight = ls_org_clause_weight;
    top_clause_weight = ls_top_clause_weight;
}

void Decimation::make_space(int max_c, int max_v)
{
    num_vars = max_v;
    num_clauses = max_c;

    max_c += 10;
    max_v += 10;

    h_true_score = new long long[max_v];
    h_false_score = new long long[max_v];
    hscore = new long long[max_v];
    s_true_score = new long long[max_v];
    s_false_score = new long long[max_v];
    sscore = new long long[max_v];

    fix.resize(max_v);
    fix.reserve(max_v);

    hunit_clause_queue = new lit[max_v];
    sense_in_hunit_clause_queue = new int[max_v];

    sunit_clause_queue = new lit[max_v];
    sense_in_sunit_clause_queue = new int[max_v];

    unassigned_var = new int[max_v];
    index_in_unassigned_var = new int[max_v];

    clause_delete = new int[max_c];
    clause_lit_count = new int[max_c];
    
    hard_binary_clause = new int[max_c];
    soft_binary_clause = new int[max_c];
    index_in_hard_binary_clause = new int[max_c];
    index_in_soft_binary_clause = new int[max_c];
}

void Decimation::free_memory()
{
    fix.clear();
    delete[] h_true_score;
    delete[] h_false_score;
    delete[] hscore;
    delete[] s_true_score;
    delete[] s_false_score;
    delete[] sscore;

    delete[] hunit_clause_queue;
    delete[] sense_in_hunit_clause_queue;

    delete[] sunit_clause_queue;
    delete[] sense_in_sunit_clause_queue;

    delete[] unassigned_var;
    delete[] index_in_unassigned_var;

    delete[] clause_delete;
    delete[] clause_lit_count;
    
    delete[] hard_binary_clause;
    delete[] soft_binary_clause;
    delete[] index_in_hard_binary_clause;
    delete[] index_in_soft_binary_clause;
}

void Decimation::init(int *ls_local_opt, int *ls_global_opt, lit *ls_unit_clause, int ls_unit_clause_count, int *ls_clause_lit_count, int *ls_hard_binary_clause, int ls_hard_binary_clause_count, int *ls_soft_binary_clause, int ls_soft_binary_clause_count)
{
    int v;
    int c;
    //parameters used in decimation
    hunit_beg_pointer = 0;
    hunit_end_pointer = 0;

    sunit_beg_pointer = 0;
    sunit_end_pointer = 0;

    unassigned_var_count = num_vars;

    //data structure of the instance
    local_opt = ls_local_opt;
    global_opt = ls_global_opt;

    for (int i = 0; i < num_vars; ++i)
    {
        v = i + 1;
        unassigned_var[i] = v;
        index_in_unassigned_var[v] = i;

        fix[v] = -1;
        sense_in_hunit_clause_queue[v] = -1; // not in hunit queue
        sense_in_sunit_clause_queue[v] = -1; // not in sunit queue
    }

    for (int i = 0; i < num_clauses; ++i)
    {
        clause_lit_count[i] = ls_clause_lit_count[i];
        clause_delete[i] = 0;
        index_in_hard_binary_clause[i] = -1;
        index_in_soft_binary_clause[i] = -1;
    }

    for (int i = 0; i < ls_unit_clause_count; ++i)
    {
        push_unit_clause_to_queue(ls_unit_clause[i]);
    }
    
    hard_binary_clause_count = ls_hard_binary_clause_count;
    for (int i = 0; i < hard_binary_clause_count; ++i)
    {
        int c = ls_hard_binary_clause[i];
        hard_binary_clause[i] = c;
        index_in_hard_binary_clause[c] = i;
    }
    
    soft_binary_clause_count = ls_soft_binary_clause_count;
    for (int i = 0; i < soft_binary_clause_count; ++i)
    {
        int c = ls_soft_binary_clause[i];
        soft_binary_clause[i] = c;
        index_in_soft_binary_clause[c] = i;
    }
    
    //figout score
    for (v = 1; v <= num_vars; ++v)
    {
        h_false_score[v] = 0;
        h_true_score[v] = 0;
        s_false_score[v] = 0;
        s_true_score[v] = 0;
        for (int i = 0; i < var_lit_count[v]; ++i)
        {
            c = var_lit[v][i].clause_num;
            if (org_clause_weight[c] == top_clause_weight)
            {
                if (var_lit[v][i].sense == 1)
                    ++h_true_score[v];
                else
                    ++h_false_score[v];
            }
            else
            {
                if (var_lit[v][i].sense == 1)
                    s_true_score[v] += org_clause_weight[c];
                else
                    s_false_score[v] += org_clause_weight[c];
            }
        }
        hscore[v] = max(h_false_score[v], h_true_score[v]);
        sscore[v] = max(s_false_score[v], s_true_score[v]);
    }
}

void Decimation::push_unit_clause_to_queue(lit tem_l)
{
    int v = tem_l.var_num;
    int c = tem_l.clause_num;
    if (org_clause_weight[c] == top_clause_weight)
    {
        if (sense_in_hunit_clause_queue[v] == -1)
        {
            sense_in_hunit_clause_queue[v] = tem_l.sense;
            hunit_clause_queue[hunit_end_pointer++] = tem_l;
        }
        else
        {
            if (sense_in_hunit_clause_queue[v] != tem_l.sense) //conflict var in hard unit queue
            {
                sense_in_hunit_clause_queue[v] = -2; //means this variable is conflict in hard unit queue
            }
        }
    }
    else
    {
        if (sense_in_hunit_clause_queue[v] != -1)
            return; //be defined by hard unit queue

        if (sense_in_sunit_clause_queue[v] == -1)
        {
            sense_in_sunit_clause_queue[v] = tem_l.sense;
            sunit_clause_queue[sunit_end_pointer++] = tem_l;
        }
        else
        {
            if (sense_in_sunit_clause_queue[v] != tem_l.sense) //conflict var in hard unit queue
            {
                sense_in_sunit_clause_queue[v] = -3; //means this variable is conflict in hard unit queue
            }
        }
    }
}

void Decimation::remove_unassigned_var(int v)
{
    int index = index_in_unassigned_var[v];
    int last_var = unassigned_var[--unassigned_var_count];
    unassigned_var[index] = last_var;
    index_in_unassigned_var[last_var] = index;
}

void Decimation::assign(int v, int sense)
{
    int c, l;
    lit tem_lit;
    fix[v] = sense;
    remove_unassigned_var(v);

    for (int i = 0; i < var_lit_count[v]; ++i)
    {
        c = var_lit[v][i].clause_num;
        
        if (org_clause_weight[c] == top_clause_weight){
            if (index_in_hard_binary_clause[c] >= 0){
                int index = index_in_hard_binary_clause[c];
                index_in_hard_binary_clause[hard_binary_clause[hard_binary_clause_count - 1]] = index;
                hard_binary_clause[index] = hard_binary_clause[--hard_binary_clause_count];
                index_in_hard_binary_clause[c] = -1;
            }
        }
        else{
            if (index_in_soft_binary_clause[c] >= 0){
                int index = index_in_soft_binary_clause[c];
                index_in_soft_binary_clause[soft_binary_clause[soft_binary_clause_count - 1]] = index;
                soft_binary_clause[index] = soft_binary_clause[--soft_binary_clause_count];
                index_in_soft_binary_clause[c] = -1;
            }
        }
        
        if (clause_delete[c] == 1)
            continue;

        if (sense == var_lit[v][i].sense)
        {
            clause_delete[c] = 1;
            if (org_clause_weight[c] == top_clause_weight)
            {
                for (int j = 0; j < clause_lit_count[c]; j++)
                {
                    tem_lit = clause_lit[c][j];
                    if (tem_lit.sense == 1)
                    {
                        h_true_score[tem_lit.var_num]--;
                    }
                    else
                        h_false_score[tem_lit.var_num]--;
                    hscore[tem_lit.var_num] = max(h_true_score[tem_lit.var_num], h_false_score[tem_lit.var_num]);
                }
            }
            else
            {
                for (int j = 0; j < clause_lit_count[c]; j++)
                {
                    tem_lit = clause_lit[c][j];
                    if (tem_lit.sense == 1)
                    {
                        s_true_score[tem_lit.var_num] -= org_clause_weight[c];
                    }
                    else
                        s_false_score[tem_lit.var_num] -= org_clause_weight[c];
                    sscore[tem_lit.var_num] = max(s_true_score[tem_lit.var_num], s_false_score[tem_lit.var_num]);
                }
            }
            
            
            continue;
        }

        for (int j = 0; j < clause_lit_count[c]; j++)
        {
            if (clause_lit[c][j].var_num == v)
            {
                swap(clause_lit[c][j], clause_lit[c][--clause_lit_count[c]]);
                break;
            }
        }
        if (clause_lit_count[c] == 1)
        {
            push_unit_clause_to_queue(clause_lit[c][0]);
        }
        else if (clause_lit_count[c] == 2)
        {
            if (org_clause_weight[c] == top_clause_weight){
                index_in_hard_binary_clause[c] = hard_binary_clause_count;
                hard_binary_clause[hard_binary_clause_count++] = c;
            }
            else{
                index_in_soft_binary_clause[c] = soft_binary_clause_count;
                soft_binary_clause[soft_binary_clause_count++] = c;
            }
        }
    }
}

bool Decimation::choose_sense(int v)
{
    return rand() % 2;
}

void Decimation::hunit_propagation()
{
    int v, c, sense, rd;

    v = hunit_clause_queue[hunit_beg_pointer].var_num;
    sense = hunit_clause_queue[hunit_beg_pointer].sense;
    hunit_beg_pointer++;

    if (sense_in_hunit_clause_queue[v] == -2)
    {
        if (sscore[v] > 0)
        {
            if (sscore[v] == s_true_score[v])
                sense = 1;
            else
                sense = 0;
        }
        else
        {
            sense = choose_sense(v);
        }
        
    }
    assign(v, sense);
}

void Decimation::sunit_propagation()
{
    int v, c, sense, rd;

    int ht;
    ht = 15;

    int best_v = sunit_clause_queue[sunit_beg_pointer].var_num;
    int best_score = sscore[best_v];
    int index = sunit_beg_pointer;
    int count = sunit_end_pointer - sunit_beg_pointer;
    if (count > 15)
    {
        for (int i = 0; i < 15; ++i)
        {
            rd = rand() % count;

            v = sunit_clause_queue[sunit_beg_pointer + rd].var_num;
            if (sscore[v] > best_score)
            {
                best_v = v;
                index = sunit_beg_pointer + rd;
            }
        }
    }
    else
    {
        for (int i = sunit_beg_pointer; i < sunit_end_pointer; ++i)
        {
            v = sunit_clause_queue[i].var_num;
            if (sscore[v] > best_score)
            {
                best_v = v;
                index = i;
            }
        }
    }
    swap(sunit_clause_queue[sunit_beg_pointer], sunit_clause_queue[index]);
    v = sunit_clause_queue[sunit_beg_pointer].var_num;
    sense = sunit_clause_queue[sunit_beg_pointer].sense;
    sunit_beg_pointer++;

    if (fix[v] != -1)
        return;

    if (sense_in_sunit_clause_queue[v] == -3)
    {
        sense = choose_sense(v);
    }
    assign(v, sense);
    
}

void Decimation::random_propagation()
{
    int v, sense;
    v = unassigned_var[rand() % unassigned_var_count];
    sense = rand() % 2;
    assign(v, sense);
}

void Decimation::hbinary_propagation()
{
    int bestv, bestsense, bestindex, c, v, sense, tempscore, index;
    long long bestscore = -1;
    if (hard_binary_clause_count <= 15){
        for (int i = 0; i < hard_binary_clause_count; i++){
            c = hard_binary_clause[i];
            index = i;
            int sense0 = clause_lit[c][0].sense, sense1 = clause_lit[c][1].sense;
            int v0 = clause_lit[c][0].var_num, v1 = clause_lit[c][1].var_num;
            if (sense0 == 1){
                if (sense1 == 1){
                    if (s_true_score[v0] > s_true_score[v1]){
                        v = v0; sense = 1;
                        tempscore = s_true_score[v0];
                    }
                    else if (s_true_score[v0] < s_true_score[v1]){
                        v = v1; sense = 1;
                        tempscore = s_true_score[v1];
                    }
                    else{
                        tempscore = s_true_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 1;
                        }
                        else{
                            v = v1; sense = 1;
                        }
                    }
                }
                else{
                    if (s_true_score[v0] > s_false_score[v1]){
                        tempscore = s_true_score[v0];
                        v = v0; sense = 1;
                    }
                    else if (s_true_score[v0] < s_false_score[v1]){
                        tempscore = s_false_score[v1];
                        v = v1; sense = 0;
                    }
                    else{
                        tempscore = s_false_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 1;
                        }
                        else{
                            v = v1; sense = 0;
                        }
                    }
                }
            }
            else{
                if (sense1 == 1){
                    if (s_false_score[v0] > s_true_score[v1]){
                        tempscore = s_false_score[v0];
                        v = v0; sense = 0;
                    }
                    else if (s_false_score[v0] < s_true_score[v1]){
                        tempscore = s_true_score[v1];
                        v = v1; sense = 1;
                    }
                    else{
                        tempscore = s_true_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 0;
                        }
                        else{
                            v = v1; sense = 1;
                        }
                    }
                }
                else{
                    if (s_false_score[v0] > s_false_score[v1]){
                        tempscore = s_false_score[v0];
                        v = v0; sense = 0;
                    }
                    else if (s_false_score[v0] < s_false_score[v1]){
                        tempscore = s_false_score[v1];
                        v = v1; sense = 0;
                    }
                    else{
                        tempscore = s_false_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 0;
                        }
                        else{
                            v = v1; sense = 0;
                        }
                    }
                }
            }
            if (tempscore > bestscore){
                bestscore = tempscore;
                bestv = v;
                bestsense = sense;
                bestindex = index;
            }
            else if (tempscore == bestscore){
                if (rand() % 2 == 1){
                    bestscore = tempscore;
                    bestv = v;
                    bestsense = sense;
                    bestindex = index;
                }
            }
        }
    }
    else{
        for (int i = 0; i < 15; i++){
            c = hard_binary_clause[rand() % hard_binary_clause_count];
            int sense0 = clause_lit[c][0].sense, sense1 = clause_lit[c][1].sense;
            int v0 = clause_lit[c][0].var_num, v1 = clause_lit[c][1].var_num;
            if (sense0 == 1){
                if (sense1 == 1){
                    if (s_true_score[v0] > s_true_score[v1]){
                        v = v0; sense = 1;
                        tempscore = s_true_score[v0];
                    }
                    else if (s_true_score[v0] < s_true_score[v1]){
                        v = v1; sense = 1;
                        tempscore = s_true_score[v1];
                    }
                    else{
                        tempscore = s_true_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 1;
                        }
                        else{
                            v = v1; sense = 1;
                        }
                    }
                }
                else{
                    if (s_true_score[v0] > s_false_score[v1]){
                        tempscore = s_true_score[v0];
                        v = v0; sense = 1;
                    }
                    else if (s_true_score[v0] < s_false_score[v1]){
                        tempscore = s_false_score[v1];
                        v = v1; sense = 0;
                    }
                    else{
                        tempscore = s_false_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 1;
                        }
                        else{
                            v = v1; sense = 0;
                        }
                    }
                }
            }
            else{
                if (sense1 == 1){
                    if (s_false_score[v0] > s_true_score[v1]){
                        tempscore = s_false_score[v0];
                        v = v0; sense = 0;
                    }
                    else if (s_false_score[v0] < s_true_score[v1]){
                        tempscore = s_true_score[v1];
                        v = v1; sense = 1;
                    }
                    else{
                        tempscore = s_true_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 0;
                        }
                        else{
                            v = v1; sense = 1;
                        }
                    }
                }
                else{
                    if (s_false_score[v0] > s_false_score[v1]){
                        tempscore = s_false_score[v0];
                        v = v0; sense = 0;
                    }
                    else if (s_false_score[v0] < s_false_score[v1]){
                        tempscore = s_false_score[v1];
                        v = v1; sense = 0;
                    }
                    else{
                        tempscore = s_false_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 0;
                        }
                        else{
                            v = v1; sense = 0;
                        }
                    }
                }
            }
            if (tempscore > bestscore){
                bestscore = tempscore;
                bestv = v;
                bestsense = sense;
                bestindex = index;
            }
            else if (tempscore == bestscore){
                if (rand() % 2 == 1){
                    bestscore = tempscore;
                    bestv = v;
                    bestsense = sense;
                    bestindex = index;
                }
            }
        }
    }
    assign(bestv, bestsense);
}

void Decimation::sbinary_propagation()
{
    int bestv, bestsense, bestindex, c, v, sense, tempscore, index;
    long long bestscore = -1;
    if (soft_binary_clause_count <= 15){
        for (int i = 0; i < soft_binary_clause_count; i++){
            c = soft_binary_clause[i];
            index = i;
            int sense0 = clause_lit[c][0].sense, sense1 = clause_lit[c][1].sense;
            int v0 = clause_lit[c][0].var_num, v1 = clause_lit[c][1].var_num;
            if (sense0 == 1){
                if (sense1 == 1){
                    if (s_true_score[v0] > s_true_score[v1]){
                        v = v0; sense = 1;
                        tempscore = s_true_score[v0];
                    }
                    else if (s_true_score[v0] < s_true_score[v1]){
                        v = v1; sense = 1;
                        tempscore = s_true_score[v1];
                    }
                    else{
                        tempscore = s_true_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 1;
                        }
                        else{
                            v = v1; sense = 1;
                        }
                    }
                }
                else{
                    if (s_true_score[v0] > s_false_score[v1]){
                        tempscore = s_true_score[v0];
                        v = v0; sense = 1;
                    }
                    else if (s_true_score[v0] < s_false_score[v1]){
                        tempscore = s_false_score[v1];
                        v = v1; sense = 0;
                    }
                    else{
                        tempscore = s_false_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 1;
                        }
                        else{
                            v = v1; sense = 0;
                        }
                    }
                }
            }
            else{
                if (sense1 == 1){
                    if (s_false_score[v0] > s_true_score[v1]){
                        tempscore = s_false_score[v0];
                        v = v0; sense = 0;
                    }
                    else if (s_false_score[v0] < s_true_score[v1]){
                        tempscore = s_true_score[v1];
                        v = v1; sense = 1;
                    }
                    else{
                        tempscore = s_true_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 0;
                        }
                        else{
                            v = v1; sense = 1;
                        }
                    }
                }
                else{
                    if (s_false_score[v0] > s_false_score[v1]){
                        tempscore = s_false_score[v0];
                        v = v0; sense = 0;
                    }
                    else if (s_false_score[v0] < s_false_score[v1]){
                        tempscore = s_false_score[v1];
                        v = v1; sense = 0;
                    }
                    else{
                        tempscore = s_false_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 0;
                        }
                        else{
                            v = v1; sense = 0;
                        }
                    }
                }
            }
            if (tempscore > bestscore){
                bestscore = tempscore;
                bestv = v;
                bestsense = sense;
                bestindex = index;
            }
            else if (tempscore == bestscore){
                if (rand() % 2 == 1){
                    bestscore = tempscore;
                    bestv = v;
                    bestsense = sense;
                    bestindex = index;
                }
            }
        }
    }
    else{
        for (int i = 0; i < 15; i++){
            c = soft_binary_clause[rand() % soft_binary_clause_count];
            int sense0 = clause_lit[c][0].sense, sense1 = clause_lit[c][1].sense;
            int v0 = clause_lit[c][0].var_num, v1 = clause_lit[c][1].var_num;
            if (sense0 == 1){
                if (sense1 == 1){
                    if (s_true_score[v0] > s_true_score[v1]){
                        v = v0; sense = 1;
                        tempscore = s_true_score[v0];
                    }
                    else if (s_true_score[v0] < s_true_score[v1]){
                        v = v1; sense = 1;
                        tempscore = s_true_score[v1];
                    }
                    else{
                        tempscore = s_true_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 1;
                        }
                        else{
                            v = v1; sense = 1;
                        }
                    }
                }
                else{
                    if (s_true_score[v0] > s_false_score[v1]){
                        tempscore = s_true_score[v0];
                        v = v0; sense = 1;
                    }
                    else if (s_true_score[v0] < s_false_score[v1]){
                        tempscore = s_false_score[v1];
                        v = v1; sense = 0;
                    }
                    else{
                        tempscore = s_false_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 1;
                        }
                        else{
                            v = v1; sense = 0;
                        }
                    }
                }
            }
            else{
                if (sense1 == 1){
                    if (s_false_score[v0] > s_true_score[v1]){
                        tempscore = s_false_score[v0];
                        v = v0; sense = 0;
                    }
                    else if (s_false_score[v0] < s_true_score[v1]){
                        tempscore = s_true_score[v1];
                        v = v1; sense = 1;
                    }
                    else{
                        tempscore = s_true_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 0;
                        }
                        else{
                            v = v1; sense = 1;
                        }
                    }
                }
                else{
                    if (s_false_score[v0] > s_false_score[v1]){
                        tempscore = s_false_score[v0];
                        v = v0; sense = 0;
                    }
                    else if (s_false_score[v0] < s_false_score[v1]){
                        tempscore = s_false_score[v1];
                        v = v1; sense = 0;
                    }
                    else{
                        tempscore = s_false_score[v1];
                        if (rand() % 2 == 1){
                            v = v0; sense = 0;
                        }
                        else{
                            v = v1; sense = 0;
                        }
                    }
                }
            }
            if (tempscore > bestscore){
                bestscore = tempscore;
                bestv = v;
                bestsense = sense;
                bestindex = index;
            }
            else if (tempscore == bestscore){
                if (rand() % 2 == 1){
                    bestscore = tempscore;
                    bestv = v;
                    bestsense = sense;
                    bestindex = index;
                }
            }
        }
    }
    assign(bestv, bestsense);
}

void Decimation::unit_prosess()
{
    while (unassigned_var_count > 0)
    {
        if (hunit_beg_pointer != hunit_end_pointer)
        {
            hunit_propagation();
        }
        else if (sunit_beg_pointer != sunit_end_pointer)
        {
            sunit_propagation();
        }
        else if (hard_binary_clause_count > 0)
        {
            hbinary_propagation();
        }
        else if (soft_binary_clause_count > 0)
        {
            sbinary_propagation();
        }
        else
        {
            random_propagation();
        }
    }
}

#endif
