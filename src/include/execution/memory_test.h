

#include "graph/graph.h"
#include "vend/vend_factory.h"


class VendMemory{

public:
    VendMemory(VendType vend_type, const std::string &vend_prefix):vend_type_(vend_type){
        encode_path_=VendFactory::GetVendPath(vend_prefix, vend_type);
    }
    void Execute(){
        std::shared_ptr<DbEngine> db = std::make_shared<DbEngine>();
        VendFactory::GetEncode(vend_type_,encode_path_,db,vend_);
        std::cout<<VEND_STRING[vend_type_]<<" memory cost: "<<memory::getMemory()<<" KB"<<std::endl;
    }

private:
    std::shared_ptr<Vend> vend_;
    VendType vend_type_;
    std::string encode_path_;

};