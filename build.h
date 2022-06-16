#ifndef _BUILD_H_
#define _BUILD_H_

#include "basis_pms.h"

BandMaxSAT::BandMaxSAT() {}

void BandMaxSAT::settings()
{
    cutoff_time = 60;
    backward_step = 21;
    gamma = 0.9;
    lambda = 1.0;
    ArmNum = 20;
    max_clause_score = 1000;
    
    if (problem_weighted == 1)
    {
        max_tries = 100000000;
        max_flips = 200000000;
        max_non_improve_flip = 10000000;

        large_clause_count_threshold = 0;
        soft_large_clause_count_threshold = 0;

        rdprob = 0.01;
        hd_count_threshold = 15;
        rwprob = 0.1;
        smooth_probability = 0.01;

        if ((top_clause_weight / num_sclauses) > 10000)
        {
            h_inc = 300;
            softclause_weight_threshold = 500;
        }
        else
        {
            h_inc = 3;
            softclause_weight_threshold = 0;
        }

        if (num_vars > 2000)
        {
            rdprob = 0.01;
            hd_count_threshold = 15;
            rwprob = 0.1;
            smooth_probability = 0.0000001;
        }
    }
    else
    {
        max_tries = 100000000;
        max_flips = 200000000;
        max_non_improve_flip = 10000000;

        large_clause_count_threshold = 0;
        soft_large_clause_count_threshold = 0;

        rdprob = 0.01;
        hd_count_threshold = 42;
        rwprob = 0.091;
        smooth_probability = 0.000003;

        h_inc = 1;
        softclause_weight_threshold = 400;

        if (num_vars < 1100) //for somall instances
        {
            h_inc = 1;
            softclause_weight_threshold = 0;
            rdprob = 0.01;
            hd_count_threshold = 15;
            rwprob = 0;
            smooth_probability = 0.01;
            return;
        }
    }
}

void BandMaxSAT::build_neighbor_relation()
{
    int i, j, count;
    int v, c, n;
    int temp_neighbor_count;
    
    for (v = 1; v <= num_vars; ++v)
    {
        neighbor_flag[v] = 0;
    }
    
    for (v = 1; v <= num_vars; ++v)
    {
        neighbor_flag[v] = 1;
        temp_neighbor_count = 0;

        for (i = 0; i < var_lit_count[v]; ++i)
        {
            c = var_lit[v][i].clause_num;
            for (j = 0; j < clause_lit_count[c]; ++j)
            {
                n = clause_lit[c][j].var_num;
                if (neighbor_flag[n] != 1)
                {
                    neighbor_flag[n] = 1;
                    temp_neighbor[temp_neighbor_count++] = n;
                }
            }
        }

        neighbor_flag[v] = 0;

        var_neighbor[v] = new int[temp_neighbor_count];
        var_neighbor_count[v] = temp_neighbor_count;

        count = 0;
        for (i = 0; i < temp_neighbor_count; i++)
        {
            var_neighbor[v][count++] = temp_neighbor[i];
            neighbor_flag[temp_neighbor[i]] = 0;
        }
    }
}

void BandMaxSAT::build_instance(char *filename)
{
    istringstream iss;
    string line;
    char tempstr1[10];
    char tempstr2[10];

    ifstream infile(filename);
    if (!infile)
    {
        cout << "c the input filename " << filename << " is invalid, please input the correct filename." << endl;
        exit(-1);
    }

    /*** build problem data structures of the instance ***/
    while (getline(infile, line))
    {
        if (line[0] == 'p')
        {
            int read_items;
            num_vars = num_clauses = 0;
            read_items = sscanf(line.c_str(), "%s %s %d %d %lld", tempstr1, tempstr2, &num_vars, &num_clauses, &top_clause_weight);

            if (read_items < 5)
            {
                cout << "read item < 5 " << endl;
                exit(-1);
            }
            break;
        }
    }

    allocate_memory();

    int v, c;
    for (c = 0; c < num_clauses; c++)
    {
        clause_lit_count[c] = 0;
        clause_lit[c] = NULL;
    }
    for (v = 1; v <= num_vars; ++v)
    {
        var_lit_count[v] = 0;
        var_lit[v] = NULL;
        var_neighbor[v] = NULL;
    }

    int cur_lit;
    c = 0;
    problem_weighted = 0;
    partial = 0;
    num_hclauses = num_sclauses = 0;
    max_clause_length = 0;
    min_clause_length = 100000000;
    unit_clause_count = 0;
    hard_binary_clause_count = 0;
    soft_binary_clause_count = 0;

    int *redunt_test = new int[num_vars + 1];
    memset(redunt_test, 0, sizeof(int) * (num_vars + 1));
    
    //Now, read the clauses, one at a time.
    while (getline(infile, line))
    {
        if (line[0] == 'c')
            continue;
        else
        {
            iss.clear();
            iss.str(line);
            iss.seekg(0, ios::beg);
        }
        clause_lit_count[c] = 0;

        iss >> org_clause_weight[c];
        if (org_clause_weight[c] != top_clause_weight)
        {
            if (org_clause_weight[c] != 1)
                problem_weighted = 1;
            total_soft_weight += org_clause_weight[c];
            num_sclauses++;
        }
        else
        {
            num_hclauses++;
            partial = 1;
        }

        iss >> cur_lit;
        int clause_reduent = 0;
        while (cur_lit != 0)
        {
            if (redunt_test[abs(cur_lit)] == 0)
            {
                temp_lit[clause_lit_count[c]] = cur_lit;
                clause_lit_count[c]++;
                redunt_test[abs(cur_lit)] = cur_lit;
            }
            else if (redunt_test[abs(cur_lit)] != cur_lit)
            {
                clause_reduent = 1;
                break;
            }
            iss >> cur_lit;
        }
        if (clause_reduent == 1)
        {
            for (int i = 0; i < clause_lit_count[c]; ++i)
                redunt_test[abs(temp_lit[i])] = 0;

            num_clauses--;
            clause_lit_count[c] = 0;
            continue;
        }

        clause_lit[c] = new lit[clause_lit_count[c] + 1];

        int i;
        for (i = 0; i < clause_lit_count[c]; ++i)
        {
            clause_lit[c][i].clause_num = c;
            clause_lit[c][i].var_num = abs(temp_lit[i]);
            redunt_test[abs(temp_lit[i])] = 0;
            if (temp_lit[i] > 0)
                clause_lit[c][i]
                    .sense = 1;
            else
                clause_lit[c][i].sense = 0;

            var_lit_count[clause_lit[c][i].var_num]++;
        }
        clause_lit[c][i].var_num = 0;
        clause_lit[c][i].clause_num = -1;

        if (clause_lit_count[c] == 1)
            unit_clause[unit_clause_count++] = clause_lit[c][0];
        else if (clause_lit_count[c] == 2){
            if (org_clause_weight[c] == top_clause_weight)
                hard_binary_clause[hard_binary_clause_count++] = c;
            else
                soft_binary_clause[soft_binary_clause_count++] = c;
        }
            

        if (clause_lit_count[c] > max_clause_length)
            max_clause_length = clause_lit_count[c];

        if (clause_lit_count[c] < min_clause_length)
            min_clause_length = clause_lit_count[c];

        c++;
    }

    infile.close();

    //creat var literal arrays
    for (v = 1; v <= num_vars; ++v)
    {
        //unassigned_hard_only_var[v - 1] = v;
        //index_in_unassigned_hard_only_var[v] = v - 1;
        var_lit[v] = new lit[var_lit_count[v] + 1];
        var_lit_count[v] = 0; //reset to 0, for build up the array
    }
    //unassigned_hard_only_var_num = num_vars;
    //scan all clauses to build up var literal arrays
    for (c = 0; c < num_clauses; ++c)
    {
        for (int i = 0; i < clause_lit_count[c]; ++i)
        {
            v = clause_lit[c][i].var_num;
            var_lit[v][var_lit_count[v]] = clause_lit[c][i];
            ++var_lit_count[v];
        }
    }
    
    for (v = 1; v <= num_vars; ++v)
        var_lit[v][var_lit_count[v]].clause_num = -1;

    build_neighbor_relation();

    best_soln_feasible = 0;
}

void BandMaxSAT::allocate_memory()
{
    int malloc_var_length = num_vars + 10;
    int malloc_clause_length = num_clauses + 10;

    unit_clause = new lit[malloc_clause_length];

    var_lit = new lit *[malloc_var_length];
    var_lit_count = new int[malloc_var_length];
    clause_lit = new lit *[malloc_clause_length];
    clause_lit_count = new int[malloc_clause_length];

    score = new long long[malloc_var_length];
    var_neighbor = new int *[malloc_var_length];
    var_neighbor_count = new int[malloc_var_length];
    time_stamp = new long long[malloc_var_length];
    neighbor_flag = new int[malloc_var_length];
    temp_neighbor = new int[malloc_var_length];

    org_clause_weight = new long long[malloc_clause_length];
    clause_weight = new long long[malloc_clause_length];  
    sat_count = new int[malloc_clause_length];
    sat_var = new int[malloc_clause_length];
    clause_selected_count = new long long[malloc_clause_length];
    best_soft_clause = new int[malloc_clause_length];

    hardunsat_stack = new int[malloc_clause_length];
    index_in_hardunsat_stack = new int[malloc_clause_length];
    softunsat_stack = new int[malloc_clause_length];
    index_in_softunsat_stack = new int[malloc_clause_length];

    unsatvar_stack = new int[malloc_var_length];
    index_in_unsatvar_stack = new int[malloc_var_length];
    unsat_app_count = new int[malloc_var_length];

    goodvar_stack = new int[malloc_var_length];
    already_in_goodvar_stack = new int[malloc_var_length];

    cur_soln = new int[malloc_var_length];
    best_soln = new int[malloc_var_length];
    local_opt_soln = new int[malloc_var_length];

    large_weight_clauses = new int[malloc_clause_length];
    soft_large_weight_clauses = new int[malloc_clause_length];
    already_in_soft_large_weight_stack = new int[malloc_clause_length];

    best_array = new int[malloc_var_length];
    temp_lit = new int[malloc_var_length];
    
    unassigned_hard_only_var = new int[malloc_var_length];
    index_in_unassigned_hard_only_var = new int[malloc_var_length];
    
    hard_binary_clause = new int[malloc_clause_length];
    soft_binary_clause = new int[malloc_clause_length];
    
    clause_score = new double[malloc_clause_length];
    selected_clauses = new int[malloc_clause_length];
    selected_times = new int[malloc_clause_length];
    sampled_clauses = new int[malloc_clause_length];
}

void BandMaxSAT::free_memory()
{
    int i;
    for (i = 0; i < num_clauses; i++)
        delete[] clause_lit[i];

    for (i = 1; i <= num_vars; ++i)
    {
        delete[] var_lit[i];
        delete[] var_neighbor[i];
    }

    delete[] var_lit;
    delete[] var_lit_count;
    delete[] clause_lit;
    delete[] clause_lit_count;

    delete[] score;
    delete[] var_neighbor;
    delete[] var_neighbor_count;
    delete[] time_stamp;
    delete[] neighbor_flag;
    delete[] temp_neighbor;

    delete[] org_clause_weight;
    delete[] clause_weight;
    delete[] sat_count;
    delete[] sat_var;
    delete[] clause_selected_count;
    delete[] best_soft_clause;

    delete[] hardunsat_stack;
    delete[] index_in_hardunsat_stack;
    delete[] softunsat_stack;
    delete[] index_in_softunsat_stack;

    delete[] unsatvar_stack;
    delete[] index_in_unsatvar_stack;
    delete[] unsat_app_count;

    delete[] goodvar_stack;
    delete[] already_in_goodvar_stack;

    //delete [] fix;
    delete[] cur_soln;
    delete[] best_soln;
    delete[] local_opt_soln;

    delete[] large_weight_clauses;
    delete[] soft_large_weight_clauses;
    delete[] already_in_soft_large_weight_stack;

    delete[] best_array;
    delete[] temp_lit;
    
    delete[] unassigned_hard_only_var;
    delete[] index_in_unassigned_hard_only_var;
    
    delete[] hard_binary_clause;
    delete[] soft_binary_clause;
    
    delete[] clause_score;
    delete[] selected_clauses;
    delete[] selected_times;
    delete[] sampled_clauses;
}

#endif
