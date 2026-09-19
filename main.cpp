#include <future>
#include <iostream>
#include <random>
#include <vector>

size_t calc(double r, size_t tests, size_t seed);
double area(double r, size_t threads, size_t tests);
bool isInside(double x, double y, double r);

int main()
{}

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
  std::vector< std::future< size_t > > res;
  res.reserve(threads);

  for (size_t i = 0; i < threads; ++i) {
    res.push_back(std::async(std::launch::async, calc, r, tests, i));
  }

  size_t count = 0;
  for (size_t i = 0; i < threads; ++i) {
    count += res[i].get();
  }

  return 4 * r * r * static_cast< double >(count) / static_cast< double >(threads * tests);
}

bool isInside(double x, double y, double r)
{
  double dx = r - x, dy = r - y;
  return dx * dx + dy * dy <= r * r;
}
