#include <ios>	
#include <iomanip>	
#include <sstream>

namespace tyro
{
std::string pad_zeros(int num, int width=4) 
{
  std::ostringstream out;
  out << std::internal << std::setfill('0') << std::setw(width) << num;
  return out.str();
}
}