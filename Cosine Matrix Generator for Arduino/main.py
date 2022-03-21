import numpy as np

# We need only 1/4 of the whole matrix, so the size should be a half of its whole size
# (minus zeroes on the edges if you wish).
matrix_size = 101   # to change | Set the resolution of the matrix
indent = 7  # to change | Set zero by default, then increase by the value of zeroes on edges

step = 1/(matrix_size/2)
half_matrix_size = int(101/2) - indent
val_matrix = np.zeros((half_matrix_size, half_matrix_size), np.float32)


def func(x, y):
    val = np.cos(np.pi/2*x)*np.cos(np.pi/2*y)* \
          np.cos(np.pi/2*x)*np.cos(np.pi/2*y)* \
          np.cos(np.pi/2*x)*np.cos(np.pi/2*y)* \
          np.cos(np.pi/2*x)*np.cos(np.pi/2*y)*1.5
    if val > 1:
        val = 1
    return val


# Ready-paste to Arduino IDE format
def print_matrix(matrix):
    print("{", end="")
    for j, row in enumerate(matrix):
        print("{", end="")
        for i, val in enumerate(row):
            if i == len(row)-1:
                print("{:.2f}".format(val), end="}")
            else:
                print("{:.2f}".format(val), end=", ")
        if j != len(matrix)-1:
            print(", \n", end="")
    print(end="}\n")


if __name__ == "__main__":
    y = 0.0
    for i in range(0, half_matrix_size):
        x = 0.0
        for j in range(0, half_matrix_size):
            val_matrix[i][j] = func(x, y)
            x += step
        y += step

    print_matrix(val_matrix)

