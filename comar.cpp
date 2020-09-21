
#include <assert.h>
#include <iostream>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <fstream>
#define OOO std::cout


class COmarius
{
public:

    class Node
    {
    public:
        friend class COmarius;
        enum E_TYPE{eNULL,eNODE,eLEAF};
        Node(Node::E_TYPE t, const std::string& name):_name(name),_type(t){
            _parent=nullptr;
        }
        Node(Node::E_TYPE t=eNULL):_type(t){
            _parent=nullptr;
        }
        ~Node(){
            for(const auto& a : _values)
                delete a;
        }
        const std::string& nv()const{return _name;};
        bool ok()const{return _type!=eNULL;}
        void name_it(std::string& name){
            _name = name;
            name.clear();
        }
        Node* store_it(std::string& name){
            Node* paka = new Node(Node::eLEAF, name);
            paka->_parent = this;
            _type = eNODE;
            this->_values.push_back(paka);
            name.clear();
            return paka;
        }

        const Node& operator[](const char* key)const
        {
            if(key==_name)
                return *this;
            for(const auto& a : _values)
            {
                if(a->_name==key){
                    // cannot have parent and first child
                    assert(this->_name!=key);
/*
                    if(a->_name.at(0)=='@')
                    {
                        std::string idx;
                        return *_get_ref((const Node*)&a, a->_name, idx);
                    }
*/
                    return *a;
                }
            }

            static Node Dummy;
            return Dummy;
        }

        const Node* _get_ref(const Node* pn, const std::string& v, std::string& idx)const
        {
            bool indexing=false;
            std::string ev;
            for(const auto& a : v)
            {
                if(a=='@')continue;
                if(a=='&')continue;
                if(a=='/'){
                    if(ev==".."){
                        pn=pn->_parent;
                    }else{
                        if(ev.empty()){
                            pn=pn->_root();
                        }
                        else{
                            pn=pn->_at(ev);
                        }
                    }
                    ev.clear();
                    continue;
                }else{
                    if(a=='['){
                        indexing=true;
                        continue;
                    }
                    if(a==']'){
                        indexing=false;
                        continue;
                    }
                }
                if(indexing)
                    idx+=a;
                else
                    ev += a;
            }//for
            pn = pn->_at(ev);
            return pn;
        }

        const Node* parent(){
            return _parent;
        }

        const std::vector<Node*>& values(){
            return _values;
        }

        const Node& node(int index=0)const{
            static Node empty;
            if(_values.size() && _values.size()>=index)
                return *_values[index];
            return empty;
        }

        const std::string value(int index=0)const{
            static std::string empty="";
            if(_values.size() )
            {
                std::string v;
                std::string idx = "0";
                if(_values.size()>index){
                    v = _values[index]->_name;
                }
                else{
                    v = _values[0]->_name;
                    idx = std::to_string(index);
                }

                if(v[0]!='@' && v[0]!='&') // value reference
                    return v;

                const Node* pn = this;

                // @../../rect/sss/sss[2],@/x/sdf/size
                pn = _get_ref(pn, v, idx);

                size_t dx = !idx.empty() ? std::stod(idx) : 0;
                if(pn->_values.size()>dx)
                    return pn->_values[dx]->_name;
            }
            return empty;
        }

        size_t count()const{
            return _values.size();
        }
private:
        const Node* _root()const {
            const Node* pn = this;
            while(pn->_parent)
                pn=pn->_parent;
            return pn;
        }

        const Node* _at(const std::string& s)const {
            for(const auto& a : _values){
                if(a->_name==s){
                    return a;
                }
            }
            static Node empty;
            return &empty;
        }

    private:
        std::string                 _name;
        std::vector<Node*>             _values;
        Node*                        _parent;
        Node::E_TYPE                 _type;
    };

public:
    COmarius():_pnode(nullptr)
    {

    }
    // looup todo
    const COmarius::Node& operator[](const char* key)const
    {
        return _pnode->operator[](key);
    }
    void parse(const char* fname)
    {
        _parse(fname);
    }
private:
    void _parse(const char* fname)
    {
        char p     = 0;
        int line   =1;
        Node* paka = nullptr;
        Node* parent = nullptr;
        std::ifstream fi(fname);
        if(fi.is_open())
        {
            while (std::getline(fi, _line))
            {
                ++line;
                char prev = 0;
                std::istringstream iss(_line);
                if(_line.empty() || _line[0]=='#')
                    continue;
                for(const auto f : _line)
                {
                    if(f=='#'){ break; }
                    switch(f)
                    {
                    case '\t':
                    case ' ':
                        break;
                    case '{':
                        parent = paka;
                        paka = _new();
                        paka->_name=_string;
                        paka->_parent = parent;
                        _string.clear();
                        if(paka!=parent && parent)
                            parent->_values.push_back(paka);
                        break;
                    case '}':
                        if(paka==nullptr){
                            throw line;
                        }
                        if(!_string.empty())
                        {
                            paka->store_it(_string);
                        }
                        paka = paka->_parent;
                        break;
                    case ',':
                        paka->store_it(_string);
                        break;
                    default:
                        _string+=f;
                        break;
                    }
                }
            }
            print(_pnode, 0);
        }
    }

    Node* _new()
    {
        if(_pnode==nullptr)
        {
            _pnode=new Node(Node::eNODE);
            _pnode->_parent = _pnode;
            return _pnode;
        }
        return new Node(Node::eLEAF);
    }


    void _wipe()
    {
        OOO << _string.c_str() << "\n";
        _string.clear();
    }

    void print(const Node* p, int depth)
    {
        ++depth;
        //OOO << "\n";
        for(int i=0;i<depth;i++) OOO<<" ";

        OOO  << p->_name.c_str() << "\n";
        if(p->_values.size())
        {
            //OOO << "\n";
            for(int i=0;i<depth;i++) OOO<<" ";
            OOO << "{\n";
            for(const auto& a : p->_values)
            {
                print(a,depth);
            }
            //OOO << "\n";
            for(int i=0;i<depth;i++) OOO<<" ";
            OOO << "}\n";
        }
        --depth;
    }

    //

private:

    std::string             _string;
    std::string             _line;
    Node*                    _pnode = nullptr;
    bool                    _escap=false;
    bool                    _toking=false;
    bool                    _refon=false;
};


int main(void)
{
    COmarius aj;

    try{
        aj.parse("./test.comar");
    }catch(int line)
    {
        std::cerr << "error parsing line " << line << "\n";
        return  -1;
    }

    std::string el = aj["x"]["xi"]["shape"].value(0);
    el = aj["x"]["xi"]["shape"].value(1);
    const COmarius::Node& n = aj["x"]["xi"]["r2"];
    el = n.value(0);
    el = n.value(1);


    return 1;
}
