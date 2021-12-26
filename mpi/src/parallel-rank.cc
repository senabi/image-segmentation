#include <fmt/core.h>
#include <fmt/ranges.h>
#include <mpi.h>
#include <algorithm>
#include <iostream>
#include <random>

using namespace std;

static minstd_rand eng{random_device{}()};

// Holds the communicator rank of a process along with the
// corresponding number. This struct is used for sorting
// the values and keeping the owning process information
// intact.
struct CommRankNumber {
  int comm_rank;
  union {
    float f;
    int i;
  } number;
};

// Gathers numbers for TMPI_Rank to process zero. Allocates space for
// the MPI datatype and returns a void * buffer to process 0.
// It returns NULL to all other processes.
void* gather_number_to_root(void* number,
                            MPI_Datatype datatype,
                            MPI_Comm comm) {
  int comm_rank, comm_size;
  MPI_Comm_rank(comm, &comm_rank);
  MPI_Comm_size(comm, &comm_size);

  // Allocate an array on the root process of a size depending
  // on the MPI datatype being used.
  int datatype_size;
  MPI_Type_size(datatype, &datatype_size);
  void* gathered_numbers;
  // shared_ptr<void*> gathered_numbers;
  if (comm_rank == 0) {
    gathered_numbers = malloc(datatype_size * comm_size);
    // gathered_numbers = new void*[datatype_size * comm_size];
    // gathered_numbers = make_shared<void*>(new void*[datatype_size *
    // comm_size]);
  }

  // Gather all of the numbers on the root process
  // MPI_Gather(number, 1, datatype, gathered_numbers.get(), 1, datatype, 0,
  // comm);
  MPI_Gather(number, 1, datatype, gathered_numbers, 1, datatype, 0, comm);

  // return gathered_numbers.get();
  return gathered_numbers;
}

// This function sorts the gathered numbers on the root process and
// returns an array of ordered by the process's rank in its
// communicator. Note - this function is only executed on the root
// process.
int* get_ranks(void* gathered_numbers,
               int gathered_number_count,
               MPI_Datatype datatype) {
  int datatype_size;
  MPI_Type_size(datatype, &datatype_size);

  // Convert the gathered number array to an array of CommRankNumbers.
  // This allows us to sort the numbers and also keep the information
  // of the processes that own the numbers intact.
  CommRankNumber* comm_rank_numbers = static_cast<CommRankNumber*>(
      malloc(gathered_number_count * sizeof(CommRankNumber)));
  int i;
  for (i = 0; i < gathered_number_count; i++) {
    comm_rank_numbers[i].comm_rank = i;
    memcpy(&(comm_rank_numbers[i].number),
           static_cast<char*>(gathered_numbers) + (i * datatype_size),
           datatype_size);
  }

  // Sort the comm rank numbers based on the datatype
  if (datatype == MPI_FLOAT) {
    sort(comm_rank_numbers, comm_rank_numbers + gathered_number_count,
         [](CommRankNumber& l, CommRankNumber& r) {
           return l.number.f < r.number.f;
         });
  } else {
    sort(comm_rank_numbers, comm_rank_numbers + gathered_number_count,
         [](CommRankNumber& l, CommRankNumber& r) {
           return l.number.i < r.number.i;
         });
  }

  // Now that the comm_rank_numbers are sorted, make an array of rank
  // values for each process. The ith element of this array contains
  // the rank value for the number sent by process i.
  int* ranks = (int*)malloc(sizeof(int) * gathered_number_count);
  for (i = 0; i < gathered_number_count; i++) {
    ranks[comm_rank_numbers[i].comm_rank] = i;
  }

  // Clean up and return the rank array
  free(comm_rank_numbers);
  return ranks;
}

// Gets the rank of the recv_data, which is of type datatype. The rank
// is returned in send_data and is of type datatype.
int TMPI_Rank(void* send_data,
              void* recv_data,
              MPI_Datatype datatype,
              MPI_Comm comm) {
  // Check base cases first - Only support MPI_INT and MPI_FLOAT for
  // this function.
  if (datatype != MPI_INT && datatype != MPI_FLOAT) {
    return MPI_ERR_TYPE;
  }

  int comm_size, comm_rank;
  MPI_Comm_size(comm, &comm_size);
  MPI_Comm_rank(comm, &comm_rank);

  // To calculate the rank, we must gather the numbers to one
  // process, sort the numbers, and then scatter the resulting rank
  // values. Start by gathering the numbers on process 0 of comm.
  void* gathered_numbers = gather_number_to_root(send_data, datatype, comm);

  // Get the ranks of each process
  int* ranks = NULL;
  if (comm_rank == 0) {
    ranks = get_ranks(gathered_numbers, comm_size, datatype);
  }

  // Scatter the rank results
  MPI_Scatter(ranks, 1, MPI_INT, recv_data, 1, MPI_INT, 0, comm);

  // Do clean up
  if (comm_rank == 0) {
    free(gathered_numbers);
    free(ranks);
  }
  return MPI_SUCCESS;
}

auto random_int = [] {
  static uniform_int_distribution<int> dist{1, 20};
  return dist(eng);
};

auto random_float = [] {
  static uniform_real_distribution<float> dis(0, 1);
  return dis(eng);
};

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);
  int world_rank, world_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  auto rand_num = random_float();
  int rank;
  TMPI_Rank(&rand_num, &rank, MPI_FLOAT, MPI_COMM_WORLD);
  fmt::print("Rank for: {} Process: {} - {}\n", rand_num, world_rank, rank);
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
  return 0;
}

/*

  int rank, size, length;
  char name[80];
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);   // GET RANK/ID
  MPI_Comm_size(MPI_COMM_WORLD, &size);   // SIZE
  MPI_Get_processor_name(name, &length);  // GET name
  auto buffer =
      fmt::format("MPI Rank: {}, Total: {}, Machine: {}", rank, size, name);

  if (rank == 0) {
    fmt::print("{}\n", buffer);
    for (auto i = 1u; i < size; i++) {
      MPI_Recv(static_cast<void*>(buffer.begin().base()), buffer.size(),
               MPI_CHAR, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      fmt::print("{}\n", buffer);
    }
  } else {
    MPI_Send(static_cast<void*>(buffer.begin().base()), buffer.size(), MPI_CHAR,
             0, rank, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  return 0;


 */
