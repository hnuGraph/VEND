//===----------------------------------------------------------------------===//
//
//
//
//
//
//
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <set>
#include "dbengine/rocksdb.h"

using namespace ROCKSDB_NAMESPACE;

RocksDb::RocksDb(const std::string &db_path) : DbEngine(db_path) {
    rocksdb_ = nullptr;
    iterator_ = nullptr;
    open_options_.IncreaseParallelism();
    // open_options_.OptimizeLevelStyleCompaction();
    // create the DB if it's not already present
    open_options_.create_if_missing = true;
    read_options_ = ROCKSDB_NAMESPACE::ReadOptions();
    read_options_.fill_cache = false;

    write_options_ = ROCKSDB_NAMESPACE::WriteOptions();

    assert(Open());
}

bool RocksDb::Close() {
    if (rocksdb_ != nullptr) {
        Status s = rocksdb_->Close();
        rocksdb_ = nullptr;
        assert(s.ok());
        return s.ok();
    }
    return true;

}


bool RocksDb::Open() {
    Status s = DB::Open(open_options_, db_path_, &rocksdb_);
    return s.ok();
}

bool RocksDb::Get(uint32_t key, uint32_t *value) {
    std::string data;
    Status s = rocksdb_->Get(read_options_, std::to_string(key), &data);
    if (s.IsNotFound())
        return false;
    StringToValue(data, value);
    return true;
}

bool RocksDb::Get(uint32_t key, std::vector<uint32_t> *value) {
    std::string data;
    Status s = rocksdb_->Get(read_options_, std::to_string(key), &data);
    if (s.IsNotFound())
        return false;
    StringToValue(data, value);
    return true;
}

bool RocksDb::Get(uint32_t key, std::set<uint32_t> *value) {
    std::string data;
    Status s = rocksdb_->Get(read_options_, std::to_string(key), &data);
    if (s.IsNotFound())
        return false;
    StringToValue(data, value);
    return true;
}

void RocksDb::StringToValue(const std::string &data, std::vector<uint32_t> *value) {
    size_t len = data.length();
    value->assign((uint32_t *) data.c_str(), (uint32_t *) data.c_str() + len / 4);
}

void RocksDb::StringToValue(const std::string &data, std::set<uint32_t> *value) {
    size_t len = data.length();
    value->insert((uint32_t *) data.c_str(), (uint32_t *) data.c_str() + len / 4);
}

void RocksDb::StringToValue(const std::string &data, uint32_t *value) {
    *value = std::stoul(data);
}

bool RocksDb::Put(uint32_t key, uint32_t value) {
    size_t len;
    char *data = ValueToString(value, &len);
    Status s = rocksdb_->Put(write_options_, std::to_string(key), Slice(data, len));
    free(data);
    return s.ok();
}

bool RocksDb::Put(uint32_t key, const std::vector<uint32_t> &value) {
    if (value.empty())
        return true;
    size_t len;
    char *data;
    ValueToString(value, &len,&data);
    Status s = rocksdb_->Put(write_options_, std::to_string(key), Slice(data, len));
    free(data);
    return s.ok();
}

void RocksDb::ValueToString(const std::vector<uint32_t> &value, size_t *len, char **data) {

    *len = sizeof(uint32_t) * value.size();
    *data = (char *) (malloc(*len));
    memset(*data, 0, *len);
    size_t offset = 0;
    for (auto &v: value) {
        memcpy(*data + offset, &v, sizeof(uint32_t));
        offset += sizeof(uint32_t);
    }

}

char *RocksDb::ValueToString(uint32_t value, size_t *len) {

    *len = sizeof(uint32_t);
    char *data = (char *) (malloc(*len));
    memset(data, 0, *len);
    memcpy(data, &value, *len);
    return data;
}


void RocksDb::BatchWrite(const std::vector<uint32_t> &keys, const std::vector<std::vector<uint32_t>> &values) {

    assert(keys.size() == values.size());
    WriteBatch write_batch;
    size_t len;
    char **data = new char* [keys.size()];
    for (size_t i = 0; i < keys.size(); ++i) {
        ValueToString(values[i], &len,&data[i]);
        write_batch.Put(std::to_string(keys[i]), Slice(data[i], len));
    }
    rocksdb_->Write(write_options_, &write_batch);
    for(int i=0;i<keys.size();++i)
        delete[] data[i];
    delete[] data;
    rocksdb_->Close();
    assert(Open());
}

void RocksDb::InitIter() {
    iterator_ = rocksdb_->NewIterator(read_options_);
    iterator_->SeekToFirst();
}

bool RocksDb::Next(uint32_t *key, std::vector<uint32_t> *value) {
    if (!iterator_->Valid())
        return false;
    StringToValue(iterator_->key().ToString(), key);
    StringToValue(iterator_->value().ToString(), value);
    iterator_->Next();
    return true;

}















