#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <string>
#include <set>
#include <map>
#include <cstdio>

struct MapsLine {
  uint64_t start, end, size;
  char flags[4];
  std::string filename;
  bool operator<(const MapsLine &m) const {
    return m.start > this->start;
  }
  bool operator==(const MapsLine &m) const {
    return m.start == this->start;
  }

  void print() const {
    std::cout << filename << " [" <<
                 std::hex << start << "," <<
                 std::hex << end << "] = " <<
                 std::dec << size/4096 << std::endl;
  }
};

struct Mappings {
  std::set<MapsLine> lines;

  // Too lazy for optional :)
  bool init(const char *file) {
    std::ifstream f(file);
    if (f.fail())
      return false;

    std::string l;
    char filename[4096];
    while (std::getline(f, l)) {
      MapsLine ml;
      int rc = sscanf(l.c_str(), "%lx-%lx %4c %*s %*s %*[0-9 ]%[^\n]\n",
          &ml.start,
          &ml.end,
          ml.flags,
          filename);
      if (rc < 3) continue;
      if (rc < 4) filename[0] = '\0';
      ml.filename = std::string{!*filename ? "<Anonymous>" : filename};
      ml.size = ml.end - ml.start;
      lines.insert(ml);
    }
    return lines.size() > 1;
  }

  std::set<MapsLine>::iterator begin() {return lines.begin();}
  std::set<MapsLine>::iterator end() {return lines.end();}
  bool contains(const MapsLine &ml) {return lines.contains(ml);}
  auto find(const MapsLine &ml) {return lines.find(ml);}
  auto insert(const MapsLine &ml, uint64_t sz) {
    MapsLine m = ml;
    m.size = sz;
    return lines.insert(std::move(m));
  }
  auto insert(const MapsLine &ml) {return lines.insert(ml);}
  void print() const { for (auto &l : lines) l.print();}
  size_t size() { size_t r = 0; for (auto &l : lines) r += l.size; return r;} 
};

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cout << "Specify two files" << std::endl;
    return -1;
  }
  Mappings left, right, grew, shrank, both;
  if (!left.init(argv[1]) || !right.init(argv[2]))
    return -1;

  for (auto const &l : left) {
    auto const &r = right.find(l);
    if (r != right.end()) {
      if (l.end < r->end)
        grew.insert(l, r->end - l.end);
      else if (l.end > r->end)
        shrank.insert(l, l.end - r->end);
      else
        both.insert(l);
    } else {
      shrank.insert(l);
    }
  }
  for (auto const &r : right) {
    auto const &l = left.find(r);
    if (l == left.end())
      grew.insert(r);
  }

  // Print results
  size_t g = grew.size();
  size_t s = shrank.size();
  size_t f = both.size();
  auto M = 1024*1024;
  std::cout << "Bulk analysis (MB)" << std::endl;
  std::cout << std::setw(18) << "Lost: " << s/M << std::endl;
  std::cout << std::setw(18) << "New: " << g/M << std::endl;
  std::cout << std::setw(18) << "Kept: " << f/M << std::endl;
  if (g > s)
    std::cout << std::setw(18) << "Grew: " << (g-s)/M << std::endl;
  else
    std::cout << std::setw(18) << "Shrank: " << (s-g)/M << std::endl;

  std::cout << "Grown (new or bigger) pages" << std::endl;
  grew.print();


}
/*
  uint64_t left_sz = 0;
  uint64_t right_sz = 0;
  std::cout << "Left pages: " << std::endl;
  for (auto const &it : leftmaps) {
    auto search = rightmaps.find(it);
    if (search == rightmaps.end()) {
      // We found a hit, ignore if identical
      if (it.end == search.end)
        continue;

      // Otherwise add it to both
      MapsLine buf = it;
      it.size

      bothmaps.insert(s
    }

    left_sz += it.size;
  }

  std::cout << "Right pages: " << std::endl;
  for (auto const &it : rightmaps) {
    if (leftmaps.contains(it))
      continue;

    std::cout << it.filename << " [" <<
                 std::hex << it.start << "," <<
                 std::hex << it.end << "] = " <<
                 std::dec << it.size/4096 << std::endl;
    right_sz += it.size;
  }

  std::cout << "Total growth: " << (right_sz - left_sz) / 4096 << " pages" << std::endl;


  return 0;
}
*/
/*
  uint64_t total_sz = 0;
  uint64_t pad_sz = 0;
  std::map<std::string, uint64_t> szmap;
  for (auto &ml : mapset) {
    total_sz += ml.size;
    if (ml.filename.size() > pad_sz)
      pad_sz = ml.filename.size();

    auto it = szmap.find(ml.filename);
    if (it == szmap.end()) {
      szmap[ml.filename] = ml.size;
    } else {
      szmap[ml.filename] += ml.size;
    }
  }

//  for (auto &ml : mapset) {
//    std::cout << std::left << std::setw(pad_sz + 4) << ml.filename << " " << ml.size/(4096) << std::endl;
//  }
//
  for (auto const & [key, val] : szmap) {
    if (val < 64*1024*1024)
      continue;
    std::cout << std::left << std::setw(pad_sz + 4) << key << ": " << val/4096 << std::endl;
  }
  std::cout << "Total size (MB): " << total_sz/(1024*1024) << std::endl;
  

  return 0;
}
*/
