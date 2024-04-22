//===----------------------------------------------------------------------===//
//
//                         
//
// 
//
// 
//
//===----------------------------------------------------------------------===//

#ifndef VEND_ROCKSDB_H
#define VEND_ROCKSDB_H

#include "dbengine/dbengine.h"
#include "thirdpart/rocksdb/db.h"
#include "thirdpart/rocksdb/slice.h"
#include "thirdpart/rocksdb/options.h"
#include "thirdpart/rocksdb/status.h"
#include "thirdpart/rocksdb/table.h"

class RocksDb : public DbEngine {

public:

    RocksDb(const std::string &db_path);

    ~RocksDb() {
        Close();
        std::cout<<"close rocksdb "<<std::endl;
        delete rocksdb_;
        iterator_ = nullptr;
        rocksdb_ = nullptr;
    };


    bool Open() override;

    bool Close() override;

    bool Get(uint32_t key, uint32_t *value) override;

    bool Get(uint32_t key, std::vector<uint32_t> &value) override;
    bool Get(uint32_t key, std::set<uint32_t> *value) override;

    bool Put(uint32_t key, uint32_t value) override;

    bool Put(uint32_t key, const std::vector<uint32_t> &value) override;

    void BatchWrite(const std::vector<uint32_t> &keys, const std::vector<std::vector<uint32_t>> &values) override;

    // calls before scan
    void InitIter() override;


    bool Next(uint32_t *key, std::vector<uint32_t> *value) override;

    /**
     *  @param len :length of data (byte )
     * */
    void ValueToString(const std::vector<uint32_t> &value, size_t *len,char **data);

    char *ValueToString(uint32_t value, size_t *len);

    void StringToValue(const std::string &data, std::vector<uint32_t> &value);
    void StringToValue(const std::string &data, std::set<uint32_t> *value);

    void StringToValue(const std::string &data, uint32_t *value);

private:
    void DisableCache();
    ROCKSDB_NAMESPACE::DB *rocksdb_;
    ROCKSDB_NAMESPACE::Iterator *iterator_;
    ROCKSDB_NAMESPACE::Options open_options_;
    ROCKSDB_NAMESPACE::ReadOptions read_options_;
    ROCKSDB_NAMESPACE::WriteOptions write_options_;

};


#endif //VEND_ROCKSDB_H
