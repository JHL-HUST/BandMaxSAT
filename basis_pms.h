#ifndef _BASIS_PMS_H_
#define _BASIS_PMS_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h> //these two h files are for timing in linux
#include <unistd.h>
#include <math.h>

using namespace std;

#define mypop(stack) stack[--stack##_fill_pointer]
#define mypush(item, stack) stack[stack##_fill_pointer++] = item

const float MY_RAND_MAX_FLOAT = 10000000.0;
const int MY_RAND_MAX_INT = 10000000;
const float BASIC_SCALE = 0.0000001; //1.0f/MY_RAND_MAX_FLOAT;

// Define a data structure for a literal.
struct lit
{
  	int clause_num; //clause num, begin with 0
  	int var_num;	//variable num, begin with 1
  	bool sense;		//is 1 for true literals, 0 for false literals.
};

static struct tms start_time;
static double get_runtime()
{
  	struct tms stop;
  	times(&stop);
  	return (double)(stop.tms_utime - start_time.tms_utime + stop.tms_stime - start_time.tms_stime) / sysconf(_SC_CLK_TCK);
}
static void start_timing()
{
	  times(&start_time);
}

class BandMaxSAT
{
    private:
  	/***********non-algorithmic information ****************/
  	int problem_weighted;
  	int partial; //1 if the instance has hard clauses, and 0 otherwise.
  	int pure_sat;
  
  	int max_clause_length;
  	int min_clause_length;
  
  	//size of the instance
  	int num_vars;	//var index from 1 to num_vars
  	int num_clauses; //clause index from 0 to num_clauses-1
  	int num_hclauses;
  	int num_sclauses;
  
  	//steps and time
  	int tries;
  	int max_tries;
  	unsigned int max_flips;
  	unsigned int max_non_improve_flip;
  	unsigned int step;
  
  	int print_time;
  	int cutoff_time;
  	int prioup_time;
  	double opt_time;
    
    	/**********BandMaxSAT information*****************/
    	int backward_step; // reward delay steps
    	double gamma; // reward discount factor
    	double lambda; // exploration bias parameter
    	int ArmNum; // number of sampled arms
    	int current_index;
    	int local_times;
    	long long pre_unsat_weight;
    	double max_clause_score;
    	int if_exceed;
    	int tabu_step;
    	double *clause_score;
    	int *selected_clauses;  
   	int *selected_times; 
    	int *sampled_clauses; 
    
  	/**********end non-algorithmic information*****************/
  	/* literal arrays */
  	lit **var_lit;		   //var_lit[i][j] means the j'th literal of var i.
  	int *var_lit_count;	//amount of literals of each var
  	lit **clause_lit;	  //clause_lit[i][j] means the j'th literal of clause i.
  	int *clause_lit_count; // amount of literals in each clause
  
  	/* Information about the variables. */
  	long long *score;  long long *score_org;
  	long long *time_stamp;
  	int **var_neighbor;
  	int *var_neighbor_count;
  	int *neighbor_flag;
  	int *temp_neighbor;
    
  	/* Information about the clauses */
  	long long top_clause_weight;
  	long long *org_clause_weight;
  	long long total_soft_weight;
  	long long *clause_weight;  
  	int *sat_count;
  	int *sat_var;
  	long long *clause_selected_count;
  	int *best_soft_clause;
  
  	//original unit clause stack
  	lit *unit_clause;
  	int unit_clause_count;
  
  	//unsat clauses stack
  	int *hardunsat_stack;		   //store the unsat clause number
  	int *index_in_hardunsat_stack; //which position is a clause in the unsat_stack
  	int hardunsat_stack_fill_pointer;
  
  	int *softunsat_stack;		   //store the unsat clause number
  	int *index_in_softunsat_stack; //which position is a clause in the unsat_stack
  	int softunsat_stack_fill_pointer;
  
  	//variables in unsat clauses
  	int *unsatvar_stack;
  	int unsatvar_stack_fill_pointer;
  	int *index_in_unsatvar_stack;
  	int *unsat_app_count; //a varible appears in how many unsat clauses
  
  	//good decreasing variables (dscore>0 and confchange=1)
  	int *goodvar_stack;
  	int goodvar_stack_fill_pointer;
  	int *already_in_goodvar_stack;
  
  	/* Information about solution */
  	int *cur_soln; //the current solution, with 1's for True variables, and 0's for False variables
  	int *best_soln;
  	int *local_opt_soln;
  	int best_soln_feasible; //when find a feasible solution, this is marked as 1.
  	int local_soln_feasible;
  	int hard_unsat_nb;
  	long long soft_unsat_weight;
  	long long opt_unsat_weight;
  	long long local_opt_unsat_weight;
  
  	//clause weighting
  	int *large_weight_clauses;
  	int large_weight_clauses_count;
  	int large_clause_count_threshold;
  
  	int *soft_large_weight_clauses;
  	int *already_in_soft_large_weight_stack;
  	int soft_large_weight_clauses_count;
  	int soft_large_clause_count_threshold;
  
  	//tem data structure used in algorithm
  	int *best_array;
  	int best_count;
  	int *temp_lit;
    
    	int *unassigned_hard_only_var;
    	int *index_in_unassigned_hard_only_var;
    	int unassigned_hard_only_var_num;
    
    	int *hard_binary_clause;
    	int *soft_binary_clause;
    	int hard_binary_clause_count;
    	int soft_binary_clause_count;
    
  	//parameters used in algorithm
  	float rwprob;
  	float rdprob;
  	float smooth_probability;
  	int hd_count_threshold;
  	int h_inc;
  	int softclause_weight_threshold;
  
  	//function used in algorithm
  	void build_neighbor_relation();
  	void allocate_memory();
  	bool verify_sol();
  	void increase_weights();
  	void smooth_weights();
  	void update_clause_weights();
    	void update_clause_scores(long long s);
  	void unsat(int clause);
  	void sat(int clause);
  	void init(vector<int> &init_solution);
  	void flip(int flipvar);
  	void update_goodvarstack1(int flipvar);
  	void update_goodvarstack2(int flipvar);
  	int pick_var();
    	int pick_var2();
  
    public:
  	BandMaxSAT();
  	void settings();
  	void build_instance(char *filename);
  	void local_search(char *inputfile);
  	void local_search_with_decimation(char *inputfile);
  	void simple_print();
  	void print_best_solution();
  	void free_memory();
};

#endif
