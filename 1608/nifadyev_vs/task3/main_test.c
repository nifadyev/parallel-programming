#include <gtest/gtest.h>
#include <mpich/mpi.h>

int main(int argc, char *argv[])
{
    int result = 0;
    int rank;

    MPI_Init(&argc, &argv);
    ::testing::InitGoogleTest(&argc, argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    ::testing::TestEventListeners &listeners =
        ::testing::UnitTest::GetInstance()->listeners();
    if (rank != 0)
    {
        delete listeners.Release(listeners.default_result_printer());
    }
    result = RUN_ALL_TESTS();
    MPI_Finalize();


    return result;
}