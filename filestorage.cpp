#include <filestorage.h>
#include <fstream>

filestorage::filestorage(std::string fn) : filename(std::move(fn)){
}

std::vector<contacts> filestorage::loadall(){
    std::vector<contacts> res;
    std::ifstream ifs(filename);
    if (!ifs) return res;
    std::string line;
    std::string block;
    while (std::getline(ifs,line)){
        if (line == "----"){
            if (!block.empty()) {
                res.push_back(contacts::deserialize(block));
                block.clear();
            }
        }
        else{
            block += line +"\n";
        }
    }

    if (!block.empty()) res.push_back(contacts::deserialize(block));
    return res;
}
void filestorage :: saveall(const std::vector <contacts>& data){
    std::ofstream ofs(filename, std :: ios :: trunc);
    for (const auto & c : data) ofs << c.serialize();
}
