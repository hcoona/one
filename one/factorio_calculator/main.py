from ortools.linear_solver import pywraplp

from absl import app
from absl import logging

#
# Recipes
#
# l: Time in seconds, m: Oil refinery, n: Chemical plant
# x1: Crude oil, x2: Water, x3: Coal
# y1: Heavy oil, y2: Light oil, y3: Petroleum gas
# z1: Sulfur, z2: Plastic bar, z3: Lubricant, z4: Solid fuel
#
# Advanced oil processing
#   100x1 + 50x2 + 5l + m => 25y1 + 45y2 + 55y3
# AKA. (Notice that the factory count should not be enlarged for time variation)
#   (R1) 1200x1 + 600x2 + 60l + m => 300y1 + 540y2 + 660y3
#
# Heavy oil cracking
#   40y1 + 30x2 + 2l + n => 30y2
# AKA.
#   (R2) 1200y1 + 900x2 + 60l + n => 900y2
#
# Light oil cracking
#   30y2 + 30x2 + 2l + n => 20y3
# AKA.
#   (R3) 900y2 + 900x2 + 60l + n => 600y3
#
# Sulfur
#   30y3 + 30x2 + l + n => 2z1
# AKA.
#   (R4) 1800y3 + 1800x2 + 60l + n => 120z1
#
# Plastic bar
#   x3 + 20y3 + l + n => 2z2
# AKA.
#   (R5) 60x3 + 1200y3 + 60l + n => 120z2
#
# Lubricant
#   10y1 + l + n => 10z3
# AKA.
#   (R6) 600y1 + 60l + n => 600z3
#
# Solid fuel
#   20y1 + 2l + n => z4
#   10y2 + 2l + n => z4
#   20y3 + 2l + n => z4
# AKA.
#   (R7) 600y1 + 60l + n => 30z4
#   (R8) 300y2 + 60l + n => 30z4
#   (R9) 600y3 + 60l + n => 30z4
#
#
# Applying (R1) r1 times would:
#   1200x1 + 600x2 + 60l + m => 300y1 + 540y2 + 660y3
#   1. Consuming 1200r1 x1, 600r1 x2, 12r1 m, AKA.
#      producing -1200r1 x1, -600r1 x2, -12r1 m.
#   2. Producing 300r1 y1, 540r1 y2, 660r1 y3.
#
# Rewriting R1~R9:
#   (R1) 1200x1 + 600x2 + 60l + m => 300y1 + 540y2 + 660y3
#   (R2) 1200y1 + 900x2 + 60l + n => 900y2
#   (R3) 900y2 + 900x2 + 60l + n => 600y3
#   (R4) 1800y3 + 1800x2 + 60l + n => 120z1
#   (R5) 60x3 + 1200y3 + 60l + n => 120z2
#   (R6) 600y1 + 60l + n => 600z3
#   (R7) 600y1 + 60l + n => 30z4
#   (R8) 300y2 + 60l + n => 30z4
#   (R9) 600y3 + 60l + n => 30z4
#
# pylint: disable=line-too-long
#
# We got:
#   Regard as a vector of
#        [     m,     n,    x1,    x2,    x3,    y1,    y2,    y3,    z1,    z2,    z3,    z4]
#   v1 = [    -1,     0, -1200,  -600,     0,   300,   540,   660,     0,     0,     0,     0]
#   v2 = [     0,    -1,     0,  -900,     0, -1200,   900,     0,     0,     0,     0,     0]
#   v3 = [     0,    -1,     0,  -900,     0,     0,  -900,   600,     0,     0,     0,     0]
#   v4 = [     0,    -1,     0, -1800,     0,     0,     0, -1800,   120,     0,     0,     0]
#   v5 = [     0,    -1,     0,     0,   -60,     0,     0, -1200,     0,   120,     0,     0]
#   v6 = [     0,    -1,     0,     0,     0,  -600,     0,     0,     0,     0,   600,     0]
#   v7 = [     0,    -1,     0,     0,     0,  -600,     0,     0,     0,     0,     0,    30]
#   v8 = [     0,    -1,     0,     0,     0,     0,  -300,     0,     0,     0,     0,    30]
#   v9 = [     0,    -1,     0,     0,     0,     0,     0,  -600,     0,     0,     0,    30]
#
# We finally want at least:
#   65z1, 720z2, 150z3, 200z4, 200y2
#        [     m,     n,    x1,    x2,    x3,    y1,    y2,    y3,    z1,    z2,    z3,    z4]
#   v0 = [  -inf,  -inf,  -inf,  -inf,  -inf,     0,   200,     0,    65,   540,   150,   200]
#
# pylint: enable=line-too-long
#
# We aim to optimize consuming x1.
#
# Constrains: Sigma(vi*ri) >= v0, ri >= 0
#
#   y1: 300r1 - 1200r2 - 600r6 - 600r7 >= 0
#   y2: 540r1 + 900r2 - 900r3 - 300r8 >= 200
#   y3: 660r1 + 600r3 - 1800r4 - 1200r5 - 600r9 >= 0
#   z1: 120r4 >= 65
#   z2: 120r5 >= 720
#   z3: 600r6 >= 150
#   z4: 30r7 + 30r8 + 30r9 >= 200
#
# Aim: min(r1)
#
# Result:
#   r[0] = 8.561253561253562
#   r[1] = 1.5153133903133904
#   r[2] = 4.207621082621083
#   r[3] = 0.5416666666666666
#   r[4] = 6
#   r[5] = 1.25
#   r[6] = 0
#   r[7] = 6.666666666666667
#   r[8] = 0
#


def main(args):
    del args

    solver = pywraplp.Solver.CreateSolver("SCIP")

    NEGATIVE_LARGE_ENOUGH = -360000
    v_object = [
        NEGATIVE_LARGE_ENOUGH, NEGATIVE_LARGE_ENOUGH, NEGATIVE_LARGE_ENOUGH,
        NEGATIVE_LARGE_ENOUGH, NEGATIVE_LARGE_ENOUGH, 0, 200, 0, 65, 640, 150,
        200
    ]
    value_vector = [
        [-1, 0, -1200, -600, 0, 300, 540, 660, 0, 0, 0, 0],
        [0, -1, 0, -900, 0, -1200, 900, 0, 0, 0, 0, 0],
        [0, -1, 0, -900, 0, 0, -900, 600, 0, 0, 0, 0],
        [0, -1, 0, -1800, 0, 0, 0, -1800, 120, 0, 0, 0],
        [0, -1, 0, 0, -60, 0, 0, -1200, 0, 120, 0, 0],
        [0, -1, 0, 0, 0, -600, 0, 0, 0, 0, 600, 0],
        [0, -1, 0, 0, 0, -600, 0, 0, 0, 0, 0, 30],
        [0, -1, 0, 0, 0, 0, -300, 0, 0, 0, 0, 30],
        [0, -1, 0, 0, 0, 0, 0, -600, 0, 0, 0, 30],
    ]

    solver_num_vars = []
    for i in range(len(value_vector)):
        solver_num_vars.append(solver.NumVar(0, solver.infinity(), "r%d" % i))

    for column_index in range(len(v_object)):
        sum_var = 0
        for row_index in range(len(value_vector)):
            sum_var += value_vector[row_index][column_index] * solver_num_vars[
                row_index]
        solver.Add(sum_var >= v_object[column_index])

    solver.Minimize(solver_num_vars[0])

    status = solver.Solve()
    if status == pywraplp.Solver.OPTIMAL:
        logging.info("Solution:")
        for i in range(len(solver_num_vars)):
            logging.info(
                "r[%d] = %.2f, m[%d] = %.2f, n[%d] = %.2f",
                i,
                solver_num_vars[i].solution_value(),
                i,
                solver_num_vars[i].solution_value() * value_vector[i][0],
                i,
                solver_num_vars[i].solution_value() * value_vector[i][1],
            )

        for column_index in range(len(v_object)):
            sum_var = 0
            for row_index in range(len(value_vector)):
                sum_var += value_vector[row_index][
                    column_index] * solver_num_vars[row_index].solution_value()
            logging.info("column=%d, sum=%.2f", column_index, sum_var)
    else:
        logging.info("The problem does not have an optimal solution. status=%d",
                     status)


if __name__ == "__main__":
    app.run(main)
