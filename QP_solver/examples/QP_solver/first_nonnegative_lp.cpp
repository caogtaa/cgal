#include <CGAL/basic.h>
#include <CGAL/QP_models.h>
#include <CGAL/QP_functions.h>

// choose exact integral type
#ifndef CGAL_USE_GMP
#include <CGAL/MP_Float.h>
typedef CGAL::MP_Float ET;
#else
#include <CGAL/Gmpz.h>
typedef CGAL::Gmpz ET;
#endif

// program and solution types
typedef CGAL::Nonnegative_linear_program_from_pointers<int> Program;
typedef CGAL::Quadratic_program_solution<ET> Solution;

int main() {
  int  Ax[] = {1, -1};                        // column for x
  int  Ay[] = {1,  2};                        // column for y
  int*  A[] = {Ax, Ay};                       // A comes columnwise
  int   b[] = {7, 4};                         // right-hand side
  CGAL::Comparison_result
        r[] = {CGAL::SMALLER, CGAL::SMALLER}; // constraints are "<="
  int   c[] = {0, -32};

  // now construct the linear program; the first two parameters are
  // the number of variables and the number of constraints (rows of A)
  Program lp (2, 2, A, b, r, c); // constant term defaults to 0

  // solve the program, using ET as the exact type
  Solution s = CGAL::solve_nonnegative_linear_program(lp, ET());

  // output solution
  if (s.is_optimal()) { // we know that, don't we?
    std::cout << "Optimal feasible solution: ";
    for (Solution::Variable_value_iterator it = s.variable_values_begin();
	 it != s.variable_values_end(); ++it)
      std::cout << *it << "  ";
    std::cout << std::endl << "Optimal objective function value: "
	      << s.objective_value() << std::endl;
  }

  return 0;
}
