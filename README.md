# image-segmentation

Image segmentation implemented using MPI, K-means algorithm in a network

## Set up container
1. Set up `.env` file:
```
NET_MODE=<main|worker>
NET_NAME=<name>
NET_PASS=<password>
USER_PASS=<user password>
```
2. Create a `images` folder put your images in:

2. Start container:
```sh
docker-compose up -d
```

3. **Create** or **join** network:
```sh
docker exec -it worker sh -c "./init-vpn.sh"
```

3. In **main** docker container, add **worker** node on **main** node:
```sh
# Main container
./add-node.sh -H <host> -HN <host-name> -P <port> -U <user>
ssh-copy-id <host>
```

## Image segmentation:
Run mpirun:
```sh
# Main container
mpirun --allow-run-as-root -H <host>:N --mca btl_tcp_if_include ham0 mpi-img-seg <n_cluster> <path/to/img> <n_iter>
```
Using hostfile:
```sh
# Main container
mpirun --allow-run-as-root --hostfile ./hostfile --mca btl_tcp_if_include ham0 mpi-img-seg <n_cluster> <path/to/img> <n_iter>
```