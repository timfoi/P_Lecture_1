#include <cmath>
#include <exception>
#include <iostream>
#include <pthread.h>
#include <random>
#include <stdexcept>
#include <system_error>
#include <vector>

constexpr size_t THREADS_COUNT = 4;

size_t calc(double r, size_t tests, size_t seed);
double area(double r, size_t threads, size_t tests);
bool isInside(double x, double y, double r);

struct Data
{
  double r;
  size_t tests, seed;
  size_t res = 0;
};

void *adapter(void *data)
{
  Data *task = static_cast< Data * >(data);
  task->res = calc(task->r, task->tests, task->seed);
  return nullptr;
}

int main()
{
  double r = 0.0;
  size_t tests = 0;
  if (!(std::cin >> r >> tests) || r <= 0.0) {
    std::cerr << "Invalid data" << '\n';
    return 1;
  }
  try {
    double res = area(r, THREADS_COUNT, tests);
    std::cout << "Монте-Карло — " << res << '\n';
    std::cout << "Формула — " << std::acos(-1.0) * r * r << '\n';
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}

size_t calc(double r, size_t tests, size_t seed)
{
  std::mt19937_64 gen(seed);

  double min = 0, max = 2 * r;
  std::uniform_real_distribution< double > dist(min, max);

  size_t res = 0;
  for (size_t i = 0; i < tests; ++i) {
    if (isInside(dist(gen), dist(gen), r)) {
      ++res;
    }
  }
  return res;
}

double area(double r, size_t threads, size_t tests)
{
  if (!threads || !tests || r <= 0.0) {
    throw std::invalid_argument("All args must be greater than 0");
  }

  size_t base = tests / threads;
  size_t remainder = tests % threads;
  std::vector< pthread_t > ths(threads);
  std::vector< Data > tasks;
  tasks.reserve(threads);

  for (size_t i = 0; i < threads; ++i) {
    size_t currTests = base + ((i < remainder) ? 1 : 0);
    tasks.push_back({r, currTests, i, 0});
  }

  for (size_t i = 0; i < threads; ++i) {
    int err = pthread_create(&ths[i], nullptr, adapter, &tasks[i]);
    if (err) {
      for (size_t j = 0; j < i; ++j) {
        pthread_join(ths[j], nullptr);
      }
      throw std::system_error(err, std::generic_category(), "pthread_create failed");
    }
  }

  int firstError = 0;
  for (pthread_t th : ths) {
    int err = pthread_join(th, nullptr);
    if (err && !firstError) {
      firstError = err;
    }
  }

  if (firstError) {
    throw std::system_error(firstError, std::generic_category(), "pthread_join failed");
  }

  size_t count = 0;
  for (const Data &task : tasks) {
    count += task.res;
  }

  return 4.0 * r * r * static_cast< double >(count) / static_cast< double >(tests);
}

bool isInside(double x, double y, double r)
{
  double dx = r - x, dy = r - y;
  return dx * dx + dy * dy <= r * r;
}
