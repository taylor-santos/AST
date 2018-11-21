#include "AST.h"


template<class T>
std::vector<T> difference(std::vector<T> a, std::vector<T> b) {
    std::vector<T> ret(a.size() + b.size());
    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());
    auto it = std::set_difference(a.begin(), a.end(), b.begin(), b.end(), ret.begin());
    ret.resize(it-ret.begin());
    return ret;
}
template<class T>
std::vector<T> intersection(std::vector<T> a, std::vector<T> b) {
    std::vector<T> ret(a.size() + b.size());
    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());
    auto it = std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), ret.begin());
    ret.resize(it-ret.begin());
    return ret;
}
template<class K, class V>
std::pair<std::map<K, V>, std::map<K, std::pair<V, V>>> map_intersection(std::map<K, V> a, std::map<K, V> b) {
    std::map<K, V> same;
    std::map<K, std::pair<V, V>> diff;
    for (auto kv : a) {
        if (b.find(kv.first) != b.end()) {
            if (kv.second == b[kv.first]) {
                same[kv.first] = kv.second;
            } else {
                diff[kv.first] = std::pair<V, V>(kv.second, b[kv.first]);
            }
        }
    }
    return std::pair<std::map<K, V>, std::map<K, std::pair<V, V>>>(same, diff);
}
template<class K, class V>
std::map<K, V> map_difference(std::map<K, V> a, std::map<K, V> b) {
    std::map<K, V> diff;
    for (auto kv : a) {
        if (b.find(kv.first) == b.end()) {
            diff[kv.first] = kv.second;
        }
    }
    return diff;
}

namespace AST
{
    std::map<std::string, ClassStruct*> ASTNode::classTable;
    std::map<std::string, ClassStruct*> ASTNode::builtinTypes;
    std::map<std::string, ClassStruct*> ASTNode::builtinIdents;
    void ASTNode::json_indent(std::ostream& out, unsigned int indent){
        for (unsigned int i=0; i<indent; i++){
                out << "  ";
        }
    }
    void ASTNode::json_head(std::ostream& out, unsigned int &indent, std::string node_kind){
        out << "{" << std::endl;
        indent++;
        this->json_indent(out, indent);
        out << "\"kind\": \"" << node_kind << "\"";
    }
    void ASTNode::json_close(std::ostream& out, unsigned int &indent){
        if (indent > 0) indent--;
        out << std::endl;
        this->json_indent(out, indent);
        out << "}";
    }
    void ASTNode::json_child(std::ostream& out, unsigned int indent, std::string field, ASTNode *child){
        out << "," << std::endl;
        this->json_indent(out, indent);
        out << "\"" << field << "\": ";
        child->json(out, indent);
    }
    
    void ASTNode::json_string(std::ostream& out, unsigned int indent, std::string field, std::string str){
        out << "," << std::endl;
        this->json_indent(out, indent);
        out << "\"" << field << "\": \"" << str << "\"";
    }
    void ASTNode::json_int(std::ostream& out, unsigned int indent, std::string field, unsigned int val){
        out << "," << std::endl;
        this->json_indent(out, indent);
        out << "\"" << field << "\": " << val;
    }
    template<class NodeType>
    void ASTNode::json_list(std::ostream& out, unsigned int indent, std::string field, std::vector<NodeType> const& list){
        out << "," << std::endl;
        this->json_indent(out, indent);
        out << "\"" << field << "\": [";
        std::string sep = "";
        for (NodeType node: list) {
                out << sep << std::endl;
                this->json_indent(out, indent+1);
                node->json(out, indent+1);
                sep = ",";
        }
        if (list.size() > 0) {        
            out << std::endl;
            this->json_indent(out, indent);
        }
        out << "]";
    }
    void TypedArg::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "TypedArg");
        this->json_string(out, indent, "name_", this->name_);
        this->json_string(out, indent, "type_", this->type_);
        this->json_close(out, indent);
    }
    void Method::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "Method");
        this->json_string(out, indent, "name_", this->name_);
        this->json_list<TypedArg*>(out, indent, "args_", *this->args_);
        if (this->isTyped_){
            this->json_string(out, indent, "type_", this->type_);
        }
        this->json_list<Statement*>(out, indent, "stmts_", *this->stmts_);
        this->json_close(out, indent);
    }
    void Class::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Class");
        this->json_string(out, indent, "name_", this->name_);
        this->json_list<TypedArg*>(out, indent, "args_", *this->args_);
        this->json_string(out, indent, "extends_", this->extends_);
        this->json_list<Statement*>(out, indent, "stmts_", *this->stmts_);
        this->json_list<Method*>(out, indent, "mthds_", *this->mthds_);
        this->json_close(out, indent);
    }
    void Program::json(std::ostream &out, unsigned int indent){
        this->json_head(out, indent, "Program");
        this->json_list<Class*>(out, indent, "classes_", *this->classes_);
        this->json_list<Statement*>(out, indent, "stmts_", *this->stmts_);
        this->json_close(out, indent);
    }
    void If::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "If");
        this->json_child(out, indent, "cond_", this->cond_);
        this->json_list<Statement*>(out, indent, "if_true_", *this->if_true_stmts_);
        this->json_list<Statement*>(out, indent, "if_false_", *this->if_false_stmts_);
        this->json_close(out, indent);
    }
    void While::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "While");
        this->json_child(out, indent, "cond_", this->cond_);
        this->json_list<Statement*>(out, indent, "stmts_", *this->stmts_);
        this->json_close(out, indent);
    }
    void LExpr::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "LExpr");
        if (this->isField_)
            this->json_child(out, indent, "obj_", this->obj_);
        this->json_string(out, indent, "name_", this->name_);
        this->json_close(out, indent);
    }
    void Assignment::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "Assignment");
        this->json_child(out, indent, "l_expr_", this->l_expr_);
        if (this->isTyped_)
            this->json_string(out, indent, "type_", this->type_);
        this->json_child(out, indent, "r_expr_", this->r_expr_);
        this->json_close(out, indent);
    }
    void Return::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "Return");
        if (!this->returnsNone_)
            this->json_child(out, indent, "r_expr_", this->r_expr_);
        this->json_close(out, indent);
    }
    void TypeAlt::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "TypeAlt");
        this->json_string(out, indent, "name_", this->name_);
        this->json_string(out, indent, "type_", this->type_);
        this->json_list<Statement*>(out, indent, "stmts_", *this->stmts_);
        this->json_close(out, indent);
    }
    void Typecase::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "Typecase");
        this->json_child(out, indent, "expr_", this->expr_);
        this->json_list<TypeAlt*>(out, indent, "alternatives_", *this->alternatives_);
        this->json_close(out, indent);
    }
    void IntLit::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "Int");
        this->json_int(out, indent, "val_", this->val_);
        this->json_close(out, indent);
    }
    void StrLit::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "String");
        this->json_string(out, indent, "text_", this->text_);
        this->json_close(out, indent);
    }
    void Call::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "Call");
        this->json_child(out, indent, "obj_", this->obj_);
        this->json_string(out, indent, "mthd_", this->mthd_);
        this->json_list<RExpr*>(out, indent, "args_", *this->args_);
        this->json_close(out, indent);
    }
    void Constructor::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "Constructor");
        this->json_string(out, indent, "name_", this->name_);
        this->json_list<RExpr*>(out, indent, "args_", *this->args_);
        this->json_close(out, indent);
    }
    void And::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "And");
        this->json_child(out, indent, "lhs_", this->lhs_);
        this->json_child(out, indent, "rhs_", this->rhs_);
        this->json_close(out, indent);
    }
    void Or::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "Or");
        this->json_child(out, indent, "lhs_", this->lhs_);
        this->json_child(out, indent, "rhs_", this->rhs_);
        this->json_close(out, indent);
    }
    void Not::json(std::ostream &out, unsigned int indent) {
        this->json_head(out, indent, "Not");
        this->json_child(out, indent, "expr_", this->expr_);
        this->json_close(out, indent);
    }
    
    void Program::addBuiltins() {
        ClassStruct *Obj     = this->addBuiltinType("Obj", nullptr);
        ClassStruct *Nothing = this->addBuiltinType("Nothing", Obj);
        ClassStruct *Int     = this->addBuiltinType("Int", Obj);
        ClassStruct *String  = this->addBuiltinType("String", Obj);
        ClassStruct *Boolean = this->addBuiltinType("Boolean", Obj);
        this->addBuiltinIdent("none",  Nothing);
        this->addBuiltinIdent("true",  Boolean);
        this->addBuiltinIdent("false", Boolean);
        //                     Type    Method    Returns  Arguments
        this->addBuiltinMethod(Obj,    "STR",    String,  std::vector<ClassStruct*>());
        this->addBuiltinMethod(Obj,    "PRINT",  Nothing, std::vector<ClassStruct*>());
        this->addBuiltinMethod(Obj,    "EQUALS", Boolean, std::vector<ClassStruct*>(1, Obj));
        this->addBuiltinMethod(String, "PLUS",   String,  std::vector<ClassStruct*>(1, String));
        this->addBuiltinMethod(Int,    "PLUS",   Int,     std::vector<ClassStruct*>(1, Int));
        this->addBuiltinMethod(Int,    "MINUS",  Int,     std::vector<ClassStruct*>(1, Int));
        this->addBuiltinMethod(Int,    "NEGATE", Int,     std::vector<ClassStruct*>());
        this->addBuiltinMethod(Int,    "TIMES",  Int,     std::vector<ClassStruct*>(1, Int));
        this->addBuiltinMethod(Int,    "DIVIDE", Int,     std::vector<ClassStruct*>(1, Int));
    }
    
    bool Program::typeCheck() {
        bool failed = false;
        std::string T = "├", Bar = "│", Elbow = "└", Dash = "─";
        std::stack<std::pair<ClassStruct*, std::string>> classStack;
        std::cout << "--------Checking Class Hierarchy--------" << std::endl;    
        addBuiltins();
        checkClassHierarchy(failed);
        if (failed){ 
            return true; 
        }
        classStack.push(std::pair<ClassStruct*, std::string>(this->builtinTypes["Obj"], ""));
        while (!classStack.empty()) {
            auto currPair = classStack.top();
            classStack.pop();
            std::cout << currPair.second << currPair.first->name << std::endl;
            for (size_t i=0; i<currPair.first->children.size(); i++) {
                auto child = currPair.first->children[i];
                std::string prefix = currPair.second;
                size_t findIndex = 0;
                while ((findIndex = prefix.find(T, findIndex)) != std::string::npos) {
                    prefix.replace(findIndex, T.length(), Bar);
                    findIndex += Bar.length();
                }
                findIndex = 0;
                while ((findIndex = prefix.find(Dash, findIndex)) != std::string::npos) {
                    prefix.replace(findIndex, Dash.length(), " ");
                    findIndex += 1;
                }
                findIndex = 0;
                while ((findIndex = prefix.find(Elbow, findIndex)) != std::string::npos) {
                    prefix.replace(findIndex, Elbow.length(), " ");
                    findIndex += 1;
                }
                
                if (i==0) {
                    classStack.push(std::pair<ClassStruct*, std::string>(child, prefix + Elbow + Dash));
                } else {
                    classStack.push(std::pair<ClassStruct*, std::string>(child, prefix + T + Dash));
                    
                }
            }
        }
        std::cout << "-Building Lowest Common Ancestor Table--" << std::endl;
        buildLCAs(failed);
        if (failed) { 
            return true; 
        }
        for (auto it : this->classTable) {
            std::cout << "\t" << it.first;
        }
        std::cout << std::endl;
        for (auto it1 : this->classTable) {
            std::cout << it1.first << "\t";
            for (auto it2 : this->classTable) {
                std::cout << it1.second->LCA[it2.second]->name << "\t";
            }
            std::cout << std::endl;
        }
        std::cout << "---------Building Method Tables---------" << std::endl;
        buildMethodTables(failed);
        if (failed) {
            return true;
        }
        for (auto it : this->classTable) {
            ClassStruct *cs = it.second;
            for (auto it2 : cs->methodTable) {
                MethodStruct *ms = it2.second;
                std::cout << cs->name << "." << ms->name << "(";
                std::string sep = "";
                for (ClassStruct *arg : ms->argTypes) {
                    std::cout << sep << arg->name;
                    sep = ", ";
                }
                std::cout << ") : " << ms->type->name << std::endl;
            }
        }
        std::cout << "------Checking Method Inheritance-------" << std::endl;
        checkMethodInheritance(failed);
        if (failed) {
            return true;
        }
        for (auto it : this->classTable) {
            ClassStruct *cs = it.second;
            for (auto it2 : cs->methodTable) {
                MethodStruct *ms = it2.second;
                std::cout << cs->name << "." << ms->name << "(";
                std::string sep = "";
                for (ClassStruct *arg : ms->argTypes) {
                    std::cout << sep << arg->name;
                    sep = ", ";
                }
                std::cout << ") : " << ms->type->name << std::endl;
            }
        }
        std::cout << "-----Finding Fields and Local Vars------" << std::endl;
        getVars(failed);
        if (failed) {
            return true;
        }
        for (auto c : this->classTable) {
            ClassStruct *cs = c.second;
            if (this->builtinTypes.find(cs->name) == this->builtinTypes.end()) {
                std::cout << "Class " << cs->name << std::endl;
                std::cout << "    Fields:" << std::endl;
                for (auto v : cs->fieldTable) { 
                    if (this->builtinIdents.find(v.first) == this->builtinIdents.end()) {
                        std::cout << "        this." << v.first;
                        if (v.second.second) {
                            std::cout << "\t* " << v.second.first->name;
                        }
                        std::cout << std::endl;
                    }
                }
                std::cout << "    Constructor Locals:" << std::endl;
                for (auto v : cs->constructor->symbolTable) { 
                    if (this->builtinIdents.find(v.first) == this->builtinIdents.end()) {
                        std::cout << "        " << v.first;
                        if (v.second.second) {
                            std::cout << "\t* " << v.second.first->name;
                        }
                        std::cout << std::endl;
                    }
                }
                for (auto m : cs->methodTable) {
                    MethodStruct *ms = m.second;
                    std::cout << "    " << ms->name << "() Locals:" << std::endl;
                    for (auto v : ms->symbolTable) { 
                        if (this->builtinIdents.find(v.first) == this->builtinIdents.end()) {
                            std::cout << "        " << v.first;
                            if (v.second.second) {
                                std::cout << "\t* " << v.second.first->name;
                            }
                            std::cout << std::endl;
                        }
                    }
                }
            }
        }
        std::cout << "Program Body Locals:" << std::endl;
        for (auto v : this->body_->symbolTable) { 
            if (this->builtinIdents.find(v.first) == this->builtinIdents.end()) {
                std::cout << "   " << v.first;
                if (v.second.second) {
                    std::cout << "\t* " << v.second.first->name;
                }
                std::cout << std::endl;
            }
        } 
        std::cout << "------------Inferring Types-------------" << std::endl;
        inferTypes(failed);
        if (failed) {
            return true;
        }
        for (auto c : this->classTable) {
            ClassStruct *cs = c.second;
            if (this->builtinTypes.find(cs->name) == this->builtinTypes.end()) {
                std::cout << "Class " << cs->name << std::endl;
                std::cout << "    Fields:" << std::endl;
                for (auto v : cs->fieldTable) { 
                    if (this->builtinIdents.find(v.first) == this->builtinIdents.end()) {
                        std::cout << "        this." << v.first;
                        if (v.second.first != nullptr) {
                            if (v.second.second)
                                std::cout << "\t* ";
                            else
                                std::cout << "\t  ";
                            std::cout << v.second.first->name;
                        }
                        std::cout << std::endl;
                    }
                }
                std::cout << "    Constructor Locals:" << std::endl;
                for (auto v : cs->constructor->symbolTable) { 
                    if (this->builtinIdents.find(v.first) == this->builtinIdents.end()) {
                        std::cout << "        " << v.first;
                        if (v.second.first != nullptr) {
                            if (v.second.second)
                                std::cout << "\t* ";
                            else
                                std::cout << "\t  ";
                            std::cout << v.second.first->name;
                        }
                        std::cout << std::endl;
                    }
                }
                for (auto m : cs->methodTable) {
                    MethodStruct *ms = m.second;
                    std::cout << "    " << ms->name << "() Locals:" << std::endl;
                    for (auto v : ms->symbolTable) { 
                        if (this->builtinIdents.find(v.first) == this->builtinIdents.end()) {
                            std::cout << "        " << v.first;
                            if (v.second.first != nullptr) {
                                if (v.second.second)
                                    std::cout << "\t* ";
                                else
                                    std::cout << "\t  ";
                                std::cout << v.second.first->name;
                            }
                            std::cout << std::endl;
                        }
                    }
                }
            }
        }
        std::cout << "Program Body Locals:" << std::endl;
        for (auto v : this->body_->symbolTable) { 
            if (this->builtinIdents.find(v.first) == this->builtinIdents.end()) {
                std::cout << "    " << v.first;
                if (v.second.first != nullptr) {
                    if (v.second.second)
                        std::cout << "\t* ";
                    else
                        std::cout << "\t  ";
                    std::cout << v.second.first->name;
                }
                std::cout << std::endl;
            }
        } 
        return failed;
    }
    void Program::buildClassMap(bool &failed) {
        std::string name, super;
        ClassStruct *cs;
        //Get class declarations
        for (Class *c : *this->classes_) {
            name = c->getName();
            if (classTable.find(name) == classTable.end()) {
                cs = new ClassStruct({name, nullptr});
                classTable[name] = cs;
                c->setClassStruct(cs);
            } else {
                std::cerr << c->getPosition() << " Error: Redefinition of class \"" << name << "\"" << std::endl;
                failed = true;
            }
        }
        //Find class inheritance
        for (Class *c : *this->classes_) {
            name = c->getName();
            super = c->getExtends();
            if (classTable.find(super) == classTable.end()) {
                std::cerr << c->getPosition() << " Error: Class \"" << name << "\" extends unrecognized type \"" << super << "\"" << std::endl;
                failed = true;
            } else if (this->builtinTypes.find(super) != this->builtinTypes.end() && 
                       super != "Obj") {
                std::cerr << c->getPosition() << " Error: Class \"" << name << "\" cannot derive from builtin type \"" << super << "\"" << std::endl;
                failed = true;
            } else {
                c->getClassStruct()->super = classTable[super];
                classTable[super]->children.push_back(c->getClassStruct());
            }
        }
    }
    
    void Program::checkClassHierarchy(bool &failed) {
        bool foundCycle;
        ClassStruct *curr1, *curr2;
        std::map<ClassStruct*, bool> noCycles;
        
        buildClassMap(failed);
        for (auto it : this->classTable) {
            ClassStruct *cs = it.second;
            if (!cs->super) {
                noCycles[cs] = true;
            } else {
                noCycles[cs] = false;
            }
        }
        for (auto it : this->classTable) {
            ClassStruct *cs = it.second;
            foundCycle = false;
            curr1 = cs;
            if (noCycles[curr1])
                continue;
            curr2 = curr1->super;
            while (!noCycles[curr1] && !noCycles[curr2]) {
                if (curr1 == curr2) {
                    std::cerr << "Error: Class hierarchy contains a cycle involving class \"" << curr1->name << "\"" << std::endl;
                    failed = true;
                    foundCycle = true;
                    break;
                }
                if (curr1->super) {
                    curr1 = curr1->super;
                } else {
                    break;
                }
                if (curr2->super && curr2->super->super) {
                    curr2 = curr2->super->super;
                } else {
                    break;
                }
            }
            if (!foundCycle) {
                curr1 = cs;
                while (!noCycles[curr1]) {
                    noCycles[curr1] = true;
                    if (curr1->super) {
                        curr1 = curr1->super;
                    } else {
                        break;
                    }
                }
            }
        }
    }
    void Program::buildLCAs(bool &failed) {
        std::map<ClassStruct*, std::vector<ClassStruct*>> paths;
        ClassStruct *curr;
        for (auto it : this->classTable) {
            curr = it.second;
            while (curr != nullptr) {
                paths[it.second].push_back(curr);
                curr = curr->super;
            }
        }
        for (auto it1 = paths.begin(); it1 != paths.end(); it1++) {
            for (auto it2 = it1; it2 != paths.end(); it2++) {
                auto curr1 = it1->second.rbegin();
                auto curr2 = it2->second.rbegin();
                auto prev = curr1;
                while (*curr1 == *curr2 && 
                        curr1 != it1->second.rend() && 
                        curr2 != it2->second.rend()) {
                    prev = curr1;
                    curr1++;
                    curr2++;
                }
                it1->first->LCA[it2->first] = *prev;
                it2->first->LCA[it1->first] = *prev;
            }
        }
    }
    void Program::buildMethodTables(bool &failed) {
        ClassStruct *cs, *argType;
        MethodStruct *ms;
        std::string methodName, argTypeName;
        for (Class *c : *this->classes_) {
            cs = c->getClassStruct();
            for (Method *m : *c->getMethods()) {
                methodName = m->getName();
                if (cs->methodTable.find(methodName) != cs->methodTable.end()) {
                    std::cerr << m->getPosition() << " Error: Redefinition of method \"" << cs->name << "." << methodName << "\"" << std::endl;
                    failed = true;
                } else {
                    ms = new MethodStruct({m->getName(), cs});
                    for (TypedArg *arg : *m->getArgs()) {
                        argTypeName = arg->getType();
                        if (this->classTable.find(argTypeName) == this->classTable.end()) {
                            std::cerr << arg->getPosition() << " Error: Argument \"" << arg->getName()
                                    << "\" of method \"" << cs->name << "." << ms->name << "()\""
                                    << " has unrecognized type \"" << argTypeName << "\"" << std::endl;
                            failed = true;
                        }else{
                            argType = this->classTable[arg->getType()];
                            ms->argTypes.push_back(argType);
                            ms->symbolTable[arg->getName()] = std::pair<ClassStruct*, bool>(argType, true);
                        }
                    }
                    for (auto it : this->builtinIdents) {
                        ms->symbolTable[it.first] = std::pair<ClassStruct*, bool>(it.second, true);
                    }
                    ms->type = this->classTable[m->getType()];
                    cs->methodTable[methodName] = ms;
                    m->setMethodStruct(ms);
                }
            }
            cs->constructor = new MethodStruct({cs->name, cs, cs});
            for (TypedArg *arg : *c->getArgs()) {
                argTypeName = arg->getType();
                if (this->classTable.find(argTypeName) == this->classTable.end()) {
                    std::cerr << arg->getPosition() << " Error: Argument \"" << arg->getName()
                            << "\" of class \"" << cs->name 
                            << "\" constructor has unrecognized type \"" << argTypeName << "\"" << std::endl;
                    failed = true;
                }else{
                    argType = this->classTable[arg->getType()];
                    cs->constructor->argTypes.push_back(argType);
                    cs->constructor->symbolTable[arg->getName()] = std::pair<ClassStruct*, bool>(argType, true);
                }
            }
            for (auto it : this->builtinIdents) {
                cs->constructor->symbolTable[it.first] = std::pair<ClassStruct*, bool>(it.second, true);
            }
        }
        this->body_ = new MethodStruct({"Body", nullptr, nullptr});
        for (auto it : this->builtinIdents) {
            this->body_->symbolTable[it.first] = std::pair<ClassStruct*, bool>(it.second, true);
        }
    }
    void Program::checkMethodInheritance(bool &failed) {
        std::map<ClassStruct*, bool> checked;
        std::stack<ClassStruct*> classStack;
        ClassStruct *curr;
        MethodStruct *ms, *super_ms;
        size_t i;
        for (auto it : this->classTable) {
            if (it.second->super == nullptr) {
                checked[it.second] = true;
            } else {
                checked[it.second] = false;
            }
        }
        for (auto it : this->classTable) {
            curr = it.second;
            while (!checked[curr]) {
                classStack.push(curr);
                curr = curr->super;
            }
            while (!classStack.empty()) {
                curr = classStack.top();
                classStack.pop();
                for (auto mthdKeyValue : curr->super->methodTable) {
                    if (curr->methodTable.find(mthdKeyValue.first) != curr->methodTable.end()) {
                        ms = curr->methodTable[mthdKeyValue.first];
                        super_ms = mthdKeyValue.second;
                        if (ms->type->LCA[super_ms->type] != super_ms->type) {
                            std::cerr << "Error: Inherited method \"" << curr->name << "." << ms->name << "()\" does not return a subtype of its overridden method" << std::endl;
                            failed = true;
                        }
                        if (ms->argTypes.size() != super_ms->argTypes.size()) {
                            std::cerr << "Error: Inherited method \"" << curr->name << "." << ms->name << "()\" does not share the same number of arguments as its overridden method" << std::endl;
                            failed = true;
                        } else {
                            for (i = 0; i < ms->argTypes.size(); i++) {
                                if (ms->argTypes[i]->LCA[super_ms->argTypes[i]] != ms->argTypes[i]) {
                                    std::cerr << "Error: Argument " << i+1 << " of inherited method \"" << curr->name << "." << ms->name << "()\" is not a supertype of its overridden argument" << std::endl;
                                    failed = true;
                                }
                            }
                        }
                        
                    } else {
                        curr->methodTable[mthdKeyValue.first] = mthdKeyValue.second;
                    }
                }
                checked[curr] = true;
            }
        }
    }
    void Program::getVars(bool &failed) {
        std::vector<std::string> fields, fieldCopy, vars;
        std::map<std::string, std::pair<ClassStruct*, bool>> fieldTable;
        MethodStruct *ms;
        ClassStruct *cs;
        for (Class *c : *this->classes_) {
            cs = c->getClassStruct();
            cs->constructor->symbolTable["this"] = std::pair<ClassStruct*, bool>(cs, true);
            for (auto sym_key_value : cs->constructor->symbolTable) {
                vars.push_back(sym_key_value.first);
            }
            for (Statement *s : *c->getStatements()) {
                s->getVars(vars, fields, cs->constructor->symbolTable, fieldTable, true, failed);
            }
            for (std::string field : fields) {
                cs->fieldTable[field] = fieldTable[field];
            }
            vars.clear();
            fieldTable.clear();
            for (Method *m : *c->getMethods()) {
                ms = m->getMethodStruct();
                ms->symbolTable["this"] = std::pair<ClassStruct*, bool>(cs, true);
                for (auto sym_key_value : ms->symbolTable) {
                    vars.push_back(sym_key_value.first);
                }
                for (Statement *s : *m->getStatements()) {
                    fieldCopy = fields;
                    s->getVars(vars, fieldCopy, ms->symbolTable, fieldTable, false, failed);
                }
                vars.clear();
                fieldTable.clear();
            }
            fields.clear();
        }
        for (auto sym_key_value : this->body_->symbolTable) {
            vars.push_back(sym_key_value.first);
        }
        for (Statement *s : *this->stmts_) {
            s->getVars(vars, fields, this->body_->symbolTable, fieldTable, false, failed);
        }
    }
    void Program::inferTypes(bool &failed) {
        bool changed;
        ClassStruct *cs;
        MethodStruct *ms;
        int pass = 1;
        do {
            std::cout << "Pass " << pass++ << std::endl;
            changed = false;
            for (Class *c : *this->classes_) {
                cs = c->getClassStruct();
                for (Statement *s : *c->getStatements()) {
                    s->updateTypes(cs, cs->constructor, changed, failed);
                }
                for (Method *m : *c->getMethods()) {
                    ms = m->getMethodStruct();
                    for (Statement *s : *m->getStatements()) {
                        s->updateTypes(cs, ms, changed, failed);
                    }
                }
            }
            for (Statement *s : *this->stmts_) {
                s->updateTypes(nullptr, this->body_, changed, failed);
            }
        } while (changed && !failed);
    }
}