//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_UTILS_H
#define VEND_UTILS_H

#include <vector>
#include <cstdint>

class Utils{

public:

    static bool binary_erase(std::vector<uint32_t> &neighbors, uint32_t vertex) {
        int begin = 0, end = neighbors.size() - 1;
        while (begin <= end) {
            int mid = (begin + end) / 2;
            if (neighbors[mid] == vertex) {
                neighbors.erase(neighbors.begin() + mid);
                return true;
            } else if (neighbors[mid] < vertex)
                begin = mid + 1;
            else
                end = mid - 1;
        }
        return false;

    }



};

namespace memory{
    inline int parseLine(char* line){
        // This assumes that a digit will be found and the line ends in " Kb".
        int i = strlen(line);
        const char* p = line;
        while (*p <'0' || *p > '9') p++;
        line[i-3] = '\0';
        i = atoi(p);
        return i;
    }

    inline int getMemory(){ //Note: this value is in KB!
        FILE* file = fopen("/proc/self/status", "r");
        int result = -1;
        char line[128];

        while (fgets(line, 128, file) != NULL){
            if (strncmp(line, "VmRSS:", 7) == 0){
                result = parseLine(line);
                break;
            }
        }
        fclose(file);
        return result;
    }
}

#endif //VEND_UTILS_H
