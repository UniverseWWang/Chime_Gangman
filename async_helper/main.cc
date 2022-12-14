#include "async_helper/graph.h"
#include "async_helper/op.h"
#include "async_helper/tensor.h"

#include "chime/core/platform/threadpool.h"

#include <iostream>

namespace ah = async_helper;

static const int TEST_COUNT = 300;

bool CheckAdd(const ah::Tensor &matrix1, const ah::Tensor &matrix2,
              const ah::Tensor &matrix3) {
  const float error = 1e-5;
  for (int64_t i = 0; i < matrix1.rows * matrix1.cols; i++) {
    if (std::fabs(matrix1.data.get()[i] + matrix2.data.get()[i] -
                  matrix3.data.get()[i]) > error) {
      std::cout << "i = " << i << ", " << matrix1.data.get()[i] << " + "
                << matrix2.data.get()[i] << " - " << matrix3.data.get()[i]
                << " = "
                << matrix1.data.get()[i] + matrix2.data.get()[i] -
                       matrix3.data.get()[i]
                << std::endl;
      return false;
    }
  }
  return true;
}

int main() {
  for (int i = 0; i < TEST_COUNT; ++i) {
    auto pool = new chime::platform::ThreadPool(chime::platform::Env::Default(),
                                                "async_helper", 12);

    ah::Tensor t1(40, 50);
    ah::Tensor t2(40, 50);
    ah::Tensor t3(40, 50);
    ah::RandomInitOp random_op1(&t1);
    ah::RandomInitOp random_op2(&t2);
    ah::AddOp add_op(random_op1.OutputsWithAsync()[0],
                     random_op2.OutputsWithAsync()[0], &t3);

    ah::StaticGraph graph(pool);
    graph.AddEdge(&random_op1, &add_op);
    graph.AddEdge(&random_op2, &add_op);

    // std::cout << "Graph forward... ";
    graph.ForwardSynced();
    // std::cout << "Graph forward done\n";

    // ah::ShowTensor(t3);

    if (!CheckAdd(t1, t2, t3)) {
      std::cout << "Add failed! iteration " << i << std::endl;
      return 1;
    }
  }
  std::cout << "Add success!\n";
}
