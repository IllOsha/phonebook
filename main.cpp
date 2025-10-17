#include <iostream>
#include <phonebook.h>
#include <validators.h>
#include <limits>  

int main(){
    phonebook pb;
    const std::string datafile = "phonebook.db";
    pb.loadfromfile(datafile);
    std::cout << "loaded " << pb.size() << " contacts.\n";

    while(true){
        std::cout << "commands: list,add,find,delete,save,stats,exit\n> ";
        std::string cmd;
        if(!(std::cin >> cmd)) break;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (cmd == "list"){
            pb.listall();
        }
        else if (cmd == "add"){
            contacts d;
            std::string tmp;

            std::cout << "FIRST NAME: ";
            std::getline(std::cin, tmp);
            d.firstname = Validators::trim(tmp);

            std::cout << "LAST NAME: ";
            std::getline(std::cin, tmp);
            d.lastname = Validators::trim(tmp);  

            std::cout << "MIDDLE NAME: ";
            std::getline(std::cin, tmp);
            d.middlename = Validators::trim(tmp);  

            std::cout << "EMAIL: ";
            std::getline(std::cin, tmp);
            d.email = Validators::trim(tmp);  

            std::cout << "BIRTHDAY DD-MM-YYYY: ";
            std::getline(std::cin, tmp);
            d.birthday = Validators::trim(tmp);  

            std::cout << "ADDRESS: ";
            std::getline(std::cin, tmp);
            d.address = Validators::trim(tmp);
            
            
            
            while(true){
                std::cout << "add number? (y/n): ";
                std::string a;
                std::getline(std::cin, a);
                a = Validators::trim(a);

                if (a == "n" || a == "N") break;
                if (a.empty()) continue;  

                PhoneNumber p;
                std::cout << "label: ";
                std::getline(std::cin, p.label);
                p.label = Validators::trim(p.label);

                std::cout << "number: ";
                std::getline(std::cin, p.number);  
                p.number = Validators::trim(p.number);

                if (!Validators::validphone(p.number)) {
                    std::cout << "warning: phone format may be invalid.\n";
                }
                d.phones.push_back(p);
            }

            if (!Validators::validname(d.firstname) || !Validators::validname(d.lastname)){
                std::cout << "invalid name - not added\n";
            }
            else{
                pb.addcontact(std::move(d));
                std::cout << "added\n";
            }
        }
        else if (cmd == "find"){
            std::string q;
            std::cout << "Enter search query: ";
            std::getline(std::cin, q);
            q = Validators::trim(q);
            auto res = pb.findbyname(q);
            for (auto idx : res){
                const auto& d = pb.at(idx);
                std::cout << "[" << idx << "] " << d.lastname << " " << d.firstname << " | " << d.email << "\n";
            }
            if (res.empty()) {
                std::cout << "No contacts found.\n";
            }
        }
        else if (cmd == "delete"){  
            std::cout << "enter index to delete: ";
            size_t idx;
            std::cin >> idx;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (pb.removebyindex(idx)) std::cout << "removed\n";
            else std::cout << "bad index\n";
        }
        else if (cmd == "save"){
            pb.savetofile(datafile);  
            std::cout << "saved\n";
        }
        else if (cmd == "stats"){
            std::cout << "Total contacts: " << pb.size() << "\n";  
        }
        else if (cmd == "exit"){
            pb.savetofile(datafile);
            std::cout << "goodbye!\n";
            break;
        }
        else{
            std::cout << "site is bussy sorry call later \n"; 
        }
    }
    return 0;
}
