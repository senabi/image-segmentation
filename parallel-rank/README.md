[![Open in Visual Studio Code](https://classroom.github.com/assets/open-in-vscode-f059dc9a6f8d3a56e377f745f24479a46679e63a5d9fe6f495e02850cd0d8118.svg)](https://classroom.github.com/online_ide?assignment_repo_id=6262063&assignment_repo_type=AssignmentRepo)

> Ivan R. Buendia Gutierrez
> 
> Diego Portocarrero Espirilla
> 
> Jhoel Tapara Quispe

# Build

```
mkdir build;
cd build && cmake .. && make
```

# Run

Parallel rank
```
mpirun -np <N> ./build/parallel-rank --mca opal_warn_on_missing_libcuda 0
```
Image segmentation
```
mpirun -np <N> ./main <NUM_CLUSTERS> <IMAGE_PATH> <NUM_ITERATIONS>
```
